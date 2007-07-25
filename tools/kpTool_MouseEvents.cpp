
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

//
// Tool reaction to view mouse input.
//


#define DEBUG_KP_TOOL 0


// TODO: reduce number of includes
#include <kpTool.h>
#include <kpToolPrivate.h>

#include <limits.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpBug.h>
#include <kpColor.h>
#include <kpColorToolBar.h>
#include <kpDefs.h>
#include <kpPixmapFX.h>
#include <kpToolAction.h>
#include <kpToolEnvironment.h>
#include <kpToolToolBar.h>
#include <kpView.h>
#include <kpViewManager.h>


void kpTool::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KP_TOOL && 1
    kDebug () << "kpTool::mousePressEvent pos=" << e->pos ()
               << " button=" << (int) e->button ()
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << " modifiers=" << (int *) (int) e->modifiers ()
               << " beganDraw=" << d->beganDraw << endl;
#endif

    if (e->button () == Qt::MidButton)
    {
        const QString text = QApplication::clipboard ()->text (QClipboard::Selection);
    #if DEBUG_KP_TOOL && 1
        kDebug () << "\tMMB pasteText='" << text << "'" << endl;
    #endif
        if (!text.isEmpty ())
        {
            if (hasBegunShape ())
            {
            #if DEBUG_KP_TOOL && 1
                kDebug () << "\t\thasBegunShape - end" << endl;
            #endif
                endShapeInternal (d->currentPoint,
                                  kpBug::QRect_Normalized (QRect (d->startPoint, d->currentPoint)));
            }

            if (viewUnderCursor ())
            {
                d->environ->pasteTextAt (text,
                    viewUnderCursor ()->transformViewToDoc (e->pos ()),
                    true/*adjust topLeft so that cursor isn't
                          on top of resize handle*/);
            }

            return;
        }
    }

    int mb = mouseButton (e->buttons ());
#if DEBUG_KP_TOOL && 1
    kDebug () << "\tmb=" << mb << " d->beganDraw=" << d->beganDraw << endl;
#endif

    if (mb == -1 && !d->beganDraw)
    {
        // Ignore mouse press.
        return;
    }

    if (d->beganDraw)
    {
        if (mb == -1 || mb != d->mouseButton)
        {
        #if DEBUG_KP_TOOL && 1
            kDebug () << "\tCancelling operation as " << mb << " == -1 or != " << d->mouseButton << endl;
        #endif

            kpView *view = viewUnderStartPoint ();
            Q_ASSERT (view);

            // if we get a mousePressEvent when we're drawing, then the other
            // mouse button must have been pressed
            d->currentPoint = view->transformViewToDoc (e->pos ());
            d->currentViewPoint = e->pos ();
            cancelShapeInternal ();
        }

        return;
    }

    kpView *view = viewUnderCursor ();
    Q_ASSERT (view);

#if DEBUG_KP_TOOL && 1
    if (view)
        kDebug () << "\tview=" << view->objectName () << endl;
#endif


    // let user know what mouse button is being used for entire draw
    d->mouseButton = mouseButton (e->buttons ());
    d->shiftPressed = (e->modifiers () & Qt::ShiftModifier);
    d->controlPressed = (e->modifiers () & Qt::ControlModifier);
    d->altPressed = (e->modifiers () & Qt::AltModifier);
    d->startPoint = d->currentPoint = view->transformViewToDoc (e->pos ());
    d->currentViewPoint = e->pos ();
    d->viewUnderStartPoint = view;
    d->lastPoint = QPoint (-1, -1);

#if DEBUG_KP_TOOL && 1
    kDebug () << "\tBeginning draw @ " << d->currentPoint << endl;
#endif

    beginDrawInternal ();

    draw (d->currentPoint, d->lastPoint, QRect (d->currentPoint, d->currentPoint));
    d->lastPoint = d->currentPoint;
}

// OPT: If the mouse is moving in terms of view pixels, it still might
//      not be moving in terms of document pixels (when zoomed in).
//
//      So we should detect this and not call draw() or hover().
//
//      However, kpToolSelection needs hover() to be called on all view
//      point changes, not just document points, since the selection resize
//      handles may be smaller than document points.  Also, I wonder if
//      selections' accidental drag detection feature cares?
void kpTool::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool::mouseMoveEvent pos=" << e->pos ()
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << " modifiers=" << (int *) (int) e->modifiers ()
    kpView *v0 = viewUnderCursor (),
           *v1 = viewManager ()->viewUnderCursor (true/*use Qt*/),
           *v2 = viewUnderStartPoint ();
    kDebug () << "\tviewUnderCursor=" << (v0 ? v0->objectName () : "(none)")
               << " viewUnderCursorQt=" << (v1 ? v1->objectName () : "(none)")
               << " viewUnderStartPoint=" << (v2 ? v2->objectName () : "(none)")
               << endl;
    kDebug () << "\tfocusWidget=" << kapp->focusWidget () << endl;
#endif

    d->shiftPressed = (e->modifiers () & Qt::ShiftModifier);
    d->controlPressed = (e->modifiers () & Qt::ControlModifier);
    d->altPressed = (e->modifiers () & Qt::AltModifier);

    if (d->beganDraw)
    {
        kpView *view = viewUnderStartPoint ();
        // TODO: RMB drag then left drag away menu = assert failure.
        //       Does this affect branches/KDE/3.x/kdegraphics/kolourpaint?
        Q_ASSERT (view);

        d->currentPoint = view->transformViewToDoc (e->pos ());
        d->currentViewPoint = e->pos ();

    #if DEBUG_KP_TOOL && 0
        kDebug () << "\tDraw!" << endl;
    #endif

        bool dragScrolled = false;
        movedAndAboutToDraw (d->currentPoint, d->lastPoint, view->zoomLevelX (), &dragScrolled);

        if (dragScrolled)
        {
            d->currentPoint = calculateCurrentPoint ();
            d->currentViewPoint = calculateCurrentPoint (false/*view point*/);

            // Scrollview has scrolled contents and has scheduled an update
            // for the newly exposed region.  If draw() schedules an update
            // as well (instead of immediately updating), the scrollview's
            // update will be executed first and it'll only update part of
            // the screen resulting in ugly tearing of the viewManager's
            // tempImage.
            viewManager ()->setFastUpdates ();
        }

        drawInternal ();

        if (dragScrolled)
            viewManager ()->restoreFastUpdates ();

        d->lastPoint = d->currentPoint;
    }
    else
    {
        kpView *view = viewUnderCursor ();
        if (!view)  // possible if cancelShape()'ed but still holding down initial mousebtn
        {
            d->currentPoint = KP_INVALID_POINT;
            d->currentViewPoint = KP_INVALID_POINT;
            return;
        }

        d->currentPoint = view->transformViewToDoc (e->pos ());
        d->currentViewPoint = e->pos ();
        hover (d->currentPoint);
    }
}

void kpTool::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_TOOL && 1
    kDebug () << "kpTool::mouseReleaseEvent pos=" << e->pos ()
               << " button=" << (int) e->button ()
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << " modifiers=" << (int *) (int) e->modifiers ()
               << " beganDraw=" << d->beganDraw << endl;
#endif

    // Have _not_ already cancelShape()'ed by pressing other mouse button?
    // (e.g. you can cancel a line dragged out with the LMB, by pressing
    //       the RMB)
    if (d->beganDraw)
    {
        kpView *view = viewUnderStartPoint ();
        // TODO: RMB drag then left click away menu = assert failure.
        //       Does this affect branches/KDE/3.x/kdegraphics/kolourpaint?
        Q_ASSERT (view);

        d->currentPoint = view->transformViewToDoc (e->pos ());
        d->currentViewPoint = e->pos ();

        drawInternal ();

        endDrawInternal (d->currentPoint, kpBug::QRect_Normalized (QRect (d->startPoint, d->currentPoint)));
    }

    if ((e->buttons () & Qt::MouseButtonMask) == 0)
    {
        releasedAllButtons ();
    }
}


void kpTool::wheelEvent (QWheelEvent *e)
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::wheelEvent() modifiers=" << (int *) (int) e->modifiers ()
               << " hasBegunDraw=" << hasBegunDraw ()
               << " delta=" << e->delta ()
               << endl;
#endif

    e->ignore ();

    // If CTRL not pressed, bye.
    if ((e->modifiers () & Qt::ControlButton) == 0)
    {
    #if DEBUG_KP_TOOL
        kDebug () << "\tno CTRL -> bye" << endl;
    #endif
        return;
    }

    // If drawing, bye; don't care if a shape in progress though.
    if (hasBegunDraw ())
    {
    #if DEBUG_KP_TOOL
        kDebug () << "\thasBegunDraw() -> bye" << endl;
    #endif
        return;
    }


    // Zoom in/out depending on wheel direction.

    // Moved wheel away from user?
    if (e->delta () > 0)
    {
    #if DEBUG_KP_TOOL
        kDebug () << "\tzoom in" << endl;
    #endif
        d->environ->zoomIn (true/*center under cursor*/);
        e->accept ();
    }
    // Moved wheel towards user?
    else if (e->delta () < 0)
    {
    #if DEBUG_KP_TOOL
        kDebug () << "\tzoom out" << endl;
    #endif
    #if 1
        d->environ->zoomOut (true/*center under cursor - make zoom in/out
                                   stay under same doc pos*/);
    #else
        d->environ->zoomOut (false/*don't center under cursor - as is
                                    confusing behaviour when zooming
                                    out*/);
    #endif
        e->accept ();
    }
}
