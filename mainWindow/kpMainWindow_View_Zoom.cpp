// REFACTOR: Clean up bits of this file

/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <qdatetime.h>
#include <qpainter.h>
#include <qtimer.h>
#include <QScrollBar>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kselectaction.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpThumbnail.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpUnzoomedThumbnailView.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>
#include <kpWidgetMapper.h>
#include <kpZoomedView.h>
#include <kpZoomedThumbnailView.h>

static int ZoomLevelFromString (const QString &stringIn)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow_View.cpp:ZoomLevelFromString(" << stringIn << ")";
#endif

    // Remove any non-digits kdelibs sometimes adds behind our back :( e.g.:
    //
    // 1. kdelibs adds accelerators to actions' text directly
    // 2. ',' is automatically added to change "1000%" to "1,000%"
    QString string = stringIn;
    string.remove (QRegExp ("[^0-9]"));
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\twithout non-digits='" << string << "'";
#endif

    // Convert zoom level to number.
    bool ok = false;
    int zoomLevel = string.toInt (&ok);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tzoomLevel=" << zoomLevel;
#endif

    if (!ok || zoomLevel < kpView::MinZoomLevel || zoomLevel > kpView::MaxZoomLevel)
        return 0;  // error
    else
        return zoomLevel;
}

//---------------------------------------------------------------------

static QString ZoomLevelToString (int zoomLevel)
{
    return i18n ("%1%", zoomLevel);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setupViewMenuZoomActions ()
{
    KActionCollection *ac = actionCollection ();


    d->actionActualSize = KStandardAction::actualSize (this, SLOT (slotActualSize ()), ac);
    d->actionFitToPage = KStandardAction::fitToPage (this, SLOT (slotFitToPage ()), ac);
    d->actionFitToWidth = KStandardAction::fitToWidth (this, SLOT (slotFitToWidth ()), ac);
    d->actionFitToHeight = KStandardAction::fitToHeight (this, SLOT (slotFitToHeight ()), ac);


    d->actionZoomIn = KStandardAction::zoomIn (this, SLOT (slotZoomIn ()), ac);
    d->actionZoomOut = KStandardAction::zoomOut (this, SLOT (slotZoomOut ()), ac);


    d->actionZoom = ac->add <KSelectAction> ("view_zoom_to");
    d->actionZoom->setText (i18n ("&Zoom"));
    connect (d->actionZoom, SIGNAL (triggered (QAction *)), SLOT (slotZoom ()));
    d->actionZoom->setEditable (true);

    // create the zoom list for the 1st call to zoomTo() below
    d->zoomList.append (10); d->zoomList.append (25); d->zoomList.append (33);
    d->zoomList.append (50); d->zoomList.append (67); d->zoomList.append (75);
    d->zoomList.append (100);
    d->zoomList.append (200); d->zoomList.append (300);
    d->zoomList.append (400); d->zoomList.append (600); d->zoomList.append (800);
    d->zoomList.append (1000); d->zoomList.append (1200); d->zoomList.append (1600);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableViewMenuZoomDocumentActions (bool enable)
{
    d->actionActualSize->setEnabled (enable);
    d->actionFitToPage->setEnabled (enable);
    d->actionFitToWidth->setEnabled (enable);
    d->actionFitToHeight->setEnabled (enable);

    d->actionZoomIn->setEnabled (enable);
    d->actionZoomOut->setEnabled (enable);
    d->actionZoom->setEnabled (enable);


    // TODO: for the time being, assume that we start at zoom 100%
    //       with no grid

    // This function is only called when a new document is created
    // or an existing document is closed.  So the following will
    // always be correct:

    zoomTo (100);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::sendZoomListToActionZoom ()
{
    QStringList items;

    const QList <int>::ConstIterator zoomListEnd (d->zoomList.end ());
    for (QList <int>::ConstIterator it = d->zoomList.constBegin ();
         it != zoomListEnd;
         ++it)
    {
        items << ::ZoomLevelToString (*it);
    }

    // Work around a KDE bug - KSelectAction::setItems() enables the action.
    // David Faure said it won't be fixed because it's a feature used by
    // KRecentFilesAction.
    bool e = d->actionZoom->isEnabled ();
    d->actionZoom->setItems (items);
    if (e != d->actionZoom->isEnabled ())
        d->actionZoom->setEnabled (e);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::zoomToPre (int zoomLevel)
{
    // We're called quite early in the init process and/or when there might
    // not be a document or a view so we have a lot of "if (ptr)" guards.

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::zoomToPre(" << zoomLevel << ")";
#endif

    zoomLevel = qBound (kpView::MinZoomLevel, zoomLevel, kpView::MaxZoomLevel);

// mute point since the thumbnail suffers from this too
#if 0
    else if (d->mainView && d->mainView->zoomLevelX () % 100 == 0 && zoomLevel % 100)
    {
        if (KMessageBox::warningContinueCancel (this,
            i18n ("Setting the zoom level to a value that is not a multiple of 100% "
                  "results in imprecise editing and redraw glitches.\n"
                  "Do you really want to set to zoom level to %1%?",
                  zoomLevel),
            QString()/*caption*/,
            i18n ("Set Zoom Level to %1%", zoomLevel),
            "DoNotAskAgain_ZoomLevelNotMultipleOf100") != KMessageBox::Continue)
        {
            zoomLevel = d->mainView->zoomLevelX ();
        }
    }
#endif

    int index = 0;
    QList <int>::Iterator it = d->zoomList.begin ();

    while (index < (int) d->zoomList.count () && zoomLevel > *it)
        it++, index++;

    if (zoomLevel != *it)
        d->zoomList.insert (it, zoomLevel);

    // OPT: We get called twice on startup.  sendZoomListToActionZoom() is very slow.
    sendZoomListToActionZoom ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tsetCurrentItem(" << index << ")";
#endif
    d->actionZoom->setCurrentItem (index);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tcurrentItem="
              << d->actionZoom->currentItem ()
              << " action="
              << d->actionZoom->action (d->actionZoom->currentItem ())
              << " checkedAction"
              << d->actionZoom->selectableActionGroup ()->checkedAction ()
              << endl;;
#endif


    if (viewMenuDocumentActionsEnabled ())
    {
        d->actionActualSize->setEnabled (zoomLevel != 100);

        d->actionZoomIn->setEnabled (d->actionZoom->currentItem () < (int) d->zoomList.count () - 1);
        d->actionZoomOut->setEnabled (d->actionZoom->currentItem () > 0);
    }


    // TODO: Is this actually needed?
    if (d->viewManager)
        d->viewManager->setQueueUpdates ();

    if (d->scrollView)
    {
        d->scrollView->setUpdatesEnabled (false);
    }
}

//---------------------------------------------------------------------

// private
void kpMainWindow::zoomToPost ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::zoomToPost()";
#endif

    if (d->mainView)
    {
        actionShowGridUpdate ();
        updateMainViewGrid ();

        // Since Zoom Level KSelectAction on ToolBar grabs focus after changing
        // Zoom, switch back to the Main View.
        // TODO: back to the last view
        d->mainView->setFocus ();

    }


    // The view magnified and moved beneath the cursor
    if (tool ())
        tool ()->somethingBelowTheCursorChanged ();


    if (d->scrollView)
    {
        // TODO: setUpdatesEnabled() should really return to old value
        //       - not necessarily "true"
        d->scrollView->setUpdatesEnabled (true);
    }

    if (d->viewManager && d->viewManager->queueUpdates ()/*just in case*/)
        d->viewManager->restoreQueueUpdates ();

    setStatusBarZoom (d->mainView ? d->mainView->zoomLevelX () : 0);

#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::zoomToPost() done";
#endif
}

//---------------------------------------------------------------------

// private
void kpMainWindow::zoomTo (int zoomLevel, bool centerUnderCursor)
{
    zoomToPre (zoomLevel);


    if (d->scrollView && d->mainView)
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\tscrollView   contentsX=" << d->scrollView->horizontalScrollBar()->value ()
                   << " contentsY=" << d->scrollView->verticalScrollBar()->value ()
                   << " contentsWidth=" << d->scrollView->widget()->width ()
                   << " contentsHeight=" << d->scrollView->widget()->height ()
                   << " visibleWidth=" << d->scrollView->viewport()->width ()
                   << " visibleHeight=" << d->scrollView->viewport()->height ()
                   << " oldZoomX=" << d->mainView->zoomLevelX ()
                   << " oldZoomY=" << d->mainView->zoomLevelY ()
                   << " newZoom=" << zoomLevel
                  << endl;
    #endif

        // TODO: when changing from no scrollbars to scrollbars, Qt lies about
        //       visibleWidth() & visibleHeight() (doesn't take into account the
        //       space taken by the would-be scrollbars) until it updates the
        //       scrollview; hence the centering is off by about 5-10 pixels.

        // TODO: Use visibleRect() for greater accuracy?
        //       Or use kpAbstractScrollAreaUtils::EstimateUsableArea()
        //       instead of ScrollView::visible{Width,Height}(), as
        //       per zoomToRect()?

        int viewX, viewY;

        bool targetDocAvail = false;
        double targetDocX = -1, targetDocY = -1;

        if (centerUnderCursor &&
            d->viewManager && d->viewManager->viewUnderCursor ())
        {
            kpView *const vuc = d->viewManager->viewUnderCursor ();
            QPoint viewPoint = vuc->mouseViewPoint ();

            // vuc->transformViewToDoc() returns QPoint which only has int
            // accuracy so we do X and Y manually.
            targetDocX = vuc->transformViewToDocX (viewPoint.x ());
            targetDocY = vuc->transformViewToDocY (viewPoint.y ());
            targetDocAvail = true;

            if (vuc != d->mainView)
                viewPoint = vuc->transformViewToOtherView (viewPoint, d->mainView);

            viewX = viewPoint.x ();
            viewY = viewPoint.y ();
        }
        else
        {
            viewX = d->scrollView->horizontalScrollBar()->value () +
                        qMin (d->mainView->width (),
                              d->scrollView->viewport()->width ()) / 2;
            viewY = d->scrollView->verticalScrollBar()->value () +
                        qMin (d->mainView->height (),
                              d->scrollView->viewport()->height ()) / 2;
        }


        int newCenterX = viewX * zoomLevel / d->mainView->zoomLevelX ();
        int newCenterY = viewY * zoomLevel / d->mainView->zoomLevelY ();

        // Do the zoom.
        d->mainView->setZoomLevel (zoomLevel, zoomLevel);

    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\tvisibleWidth=" << d->scrollView->viewport()->width ()
                    << " visibleHeight=" << d->scrollView->viewport()->height ()
                    << endl;
        kDebug () << "\tnewCenterX=" << newCenterX
                    << " newCenterY=" << newCenterY << endl;
    #endif

        d->scrollView->horizontalScrollBar()->setValue(newCenterX - (d->scrollView->viewport()->width() / 2));
        d->scrollView->verticalScrollBar()->setValue(newCenterY - (d->scrollView->viewport()->height() / 2));

        if (centerUnderCursor &&
            targetDocAvail &&
            d->viewManager && d->viewManager->viewUnderCursor ())
        {
            // Move the mouse cursor so that it is still above the same
            // document pixel as before the zoom.

            kpView *const vuc = d->viewManager->viewUnderCursor ();

        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tcenterUnderCursor: reposition cursor; viewUnderCursor="
                       << vuc->objectName () << endl;
        #endif

            const double viewX = vuc->transformDocToViewX (targetDocX);
            const double viewY = vuc->transformDocToViewY (targetDocY);
            // Rounding error from zooming in and out :(
            // TODO: do everything in terms of tool doc points in type "double".
            const QPoint viewPoint ((int) viewX, (int) viewY);
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\t\tdoc: (" << targetDocX << "," << targetDocY << ")"
                       << " viewUnderCursor: (" << viewX << "," << viewY << ")"
                       << endl;
        #endif

            if (vuc->visibleRegion ().contains (viewPoint))
            {
                const QPoint globalPoint =
                    kpWidgetMapper::toGlobal (vuc, viewPoint);
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\t\tglobalPoint=" << globalPoint;
            #endif

                // TODO: Determine some sane cursor flashing indication -
                //       cursor movement is convenient but not conventional.
                //
                //       Major problem: if using QApplication::setOverrideCursor()
                //           and in some stage of flash and window quits.
                //
                //           Or if using kpView::setCursor() and change tool.
                QCursor::setPos (globalPoint);
            }
            // e.g. Zoom to 200%, scroll mainView to bottom-right.
            // Unzoomed Thumbnail shows top-left portion of bottom-right of
            // mainView.
            //
            // Aim cursor at bottom-right of thumbnail and zoom out with
            // CTRL+Wheel.
            //
            // If mainView is now small enough to largely not need scrollbars,
            // Unzoomed Thumbnail scrolls to show _top-left_ portion
            // _of top-left_ of mainView.
            //
            // Unzoomed Thumbnail no longer contains the point we zoomed out
            // on top of.
            else
            {
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\t\twon't move cursor - would get outside view"
                           << endl;
            #endif

                // TODO: Sane cursor flashing indication that indicates
                //       that the normal cursor movement didn't happen.
            }
        }

    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\t\tcheck (contentsX=" << d->scrollView->horizontalScrollBar()->value ()
                    << ",contentsY=" << d->scrollView->verticalScrollBar()->value ()
                    << ")" << endl;
    #endif
    }


    zoomToPost ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::zoomToRect (const QRect &normalizedDocRect,
        bool accountForGrips,
        bool careAboutWidth, bool careAboutHeight)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::zoomToRect(normalizedDocRect="
              << normalizedDocRect
              << ",accountForGrips=" << accountForGrips
              << ",careAboutWidth=" << careAboutWidth
              << ",careAboutHeight=" << careAboutHeight
              << ")";
#endif
    // You can't care about nothing.
    Q_ASSERT (careAboutWidth || careAboutHeight);

    // The size of the scroll view minus the current or future scrollbars.
    const QSize usableScrollArea
      (d->scrollView->maximumViewportSize().width() - d->scrollView->verticalScrollBar()->sizeHint().width(),
       d->scrollView->maximumViewportSize().height() - d->scrollView->horizontalScrollBar()->sizeHint().height());

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "size=" << d->scrollView->maximumViewportSize()
              << "scrollbar w=" << d->scrollView->verticalScrollBar()->sizeHint().width()
              << "usableSize=" << usableScrollArea;
#endif
    // Handle rounding error, mis-estimating the scroll view size and
    // cosmic rays.  We do this because we really don't want unnecessary
    // scrollbars.  This seems to need to be at least 2 for slotFitToWidth()
    // and friends.
    // least 2.
    // TODO: I might have fixed this but check later.
    const int slack = 0;

    // The grip and slack are in view coordinates but are never zoomed.
    const int viewWidth =
        usableScrollArea.width () -
        (accountForGrips ? kpGrip::Size : 0) -
        slack;
    const int viewHeight =
        usableScrollArea.height () -
        (accountForGrips ? kpGrip::Size : 0) -
        slack;

    // We want the selected document rectangle to fill the scroll view.
    //
    // The integer arithmetic rounds down, rather than to the nearest zoom
    // level, as rounding down guarantees that the view, at the zoom level,
    // will fit inside <viewWidth> x <viewHeight>.
    const int zoomX =
        careAboutWidth ?
            qMax (1, viewWidth * 100 / normalizedDocRect.width ()) :
            INT_MAX;
    const int zoomY =
        careAboutHeight ?
            qMax (1, viewHeight * 100 / normalizedDocRect.height ()) :
            INT_MAX;

    // Since kpView only supports identical horizontal and vertical zooms,
    // choose the one that will show the greatest amount of document
    // content.
    const int zoomLevel = qMin (zoomX, zoomY);

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tzoomX=" << zoomX
        << " zoomY=" << zoomY
        << " -> zoomLevel=" << zoomLevel << endl;
#endif

    zoomToPre (zoomLevel);
    {
        d->mainView->setZoomLevel (zoomLevel, zoomLevel);

        const QPoint viewPoint =
            d->mainView->transformDocToView (normalizedDocRect.topLeft ());

        d->scrollView->horizontalScrollBar()->setValue(viewPoint.x());
        d->scrollView->verticalScrollBar()->setValue(viewPoint.y());
    }
    zoomToPost ();
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotActualSize ()
{
    zoomTo (100);
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotFitToPage ()
{
  if ( d->document )
  {
    zoomToRect (
        d->document->rect (),
        true/*account for grips*/,
        true/*care about width*/, true/*care about height*/);
  }
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotFitToWidth ()
{
  if ( d->document )
  {
    const QRect docRect (
        0/*x*/,
        (int) d->mainView->transformViewToDocY (d->scrollView->verticalScrollBar()->value ())/*maintain y*/,
        d->document->width (),
        1/*don't care about height*/);
    zoomToRect (
        docRect,
        true/*account for grips*/,
        true/*care about width*/, false/*don't care about height*/);
  }
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotFitToHeight ()
{
  if ( d->document )
  {
    const QRect docRect (
        (int) d->mainView->transformViewToDocX (d->scrollView->horizontalScrollBar()->value ())/*maintain x*/,
        0/*y*/,
        1/*don't care about width*/,
        d->document->height ());
    zoomToRect (
        docRect,
        true/*account for grips*/,
        false/*don't care about width*/, true/*care about height*/);
  }
}

//---------------------------------------------------------------------

// public
void kpMainWindow::zoomIn (bool centerUnderCursor)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::zoomIn(centerUnderCursor="
              << centerUnderCursor << ") currentItem="
              << d->actionZoom->currentItem ()
              << endl;
#endif
    const int targetItem = d->actionZoom->currentItem () + 1;

    if (targetItem >= (int) d->zoomList.count ())
        return;

    d->actionZoom->setCurrentItem (targetItem);

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tnew currentItem=" << d->actionZoom->currentItem ();
#endif

    zoomAccordingToZoomAction (centerUnderCursor);
}

//---------------------------------------------------------------------

// public
void kpMainWindow::zoomOut (bool centerUnderCursor)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::zoomOut(centerUnderCursor="
              << centerUnderCursor << ") currentItem="
              << d->actionZoom->currentItem ()
              << endl;
#endif
    const int targetItem = d->actionZoom->currentItem () - 1;

    if (targetItem < 0)
        return;

    d->actionZoom->setCurrentItem (targetItem);

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tnew currentItem=" << d->actionZoom->currentItem ();
#endif

    zoomAccordingToZoomAction (centerUnderCursor);
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotZoomIn ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotZoomIn ()";
#endif

    zoomIn (false/*don't center under cursor*/);
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotZoomOut ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotZoomOut ()";
#endif

    zoomOut (false/*don't center under cursor*/);
}

//---------------------------------------------------------------------

// public
void kpMainWindow::zoomAccordingToZoomAction (bool centerUnderCursor)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::zoomAccordingToZoomAction(centerUnderCursor="
              << centerUnderCursor
              << ") currentItem=" << d->actionZoom->currentItem ()
              << " currentText=" << d->actionZoom->currentText ()
              << endl;
#endif

    // This might be a new zoom level the user has typed in.
    zoomTo (::ZoomLevelFromString (d->actionZoom->currentText ()),
                                   centerUnderCursor);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotZoom ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotZoom () index=" << d->actionZoom->currentItem ()
               << " text='" << d->actionZoom->currentText () << "'" << endl;
#endif

    zoomAccordingToZoomAction (false/*don't center under cursor*/);
}

//---------------------------------------------------------------------
