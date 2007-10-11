
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
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


#define DEBUG_KP_VIEW 0
#define DEBUG_KP_VIEW_RENDERER ((DEBUG_KP_VIEW && 1) || 0)


#include <kpView.h>
#include <kpViewPrivate.h>

#include <math.h>
#include <stdlib.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpointer.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>
#include <qregion.h>
#include <qvector.h>

#include <KDebug>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpPixmapFX.h>
#include <kpTempImage.h>
#include <kpTextSelection.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>


// public static
const int kpView::MinZoomLevel = 1;
const int kpView::MaxZoomLevel = 3200;


kpView::kpView (kpDocument *document,
        kpToolToolBar *toolToolBar,
        kpViewManager *viewManager,
        kpView *buddyView,
        kpViewScrollableContainer *scrollableContainer,
        QWidget *parent)
    : QWidget (parent),
      d (new kpViewPrivate ())
{
    d->document = document;
    d->toolToolBar = toolToolBar;
    d->viewManager = viewManager;
    d->buddyView = buddyView;
    d->scrollableContainer = scrollableContainer;

    d->hzoom = 100, d->vzoom = 100;
    d->origin = QPoint (0, 0);
    d->showGrid = false;
    d->isBuddyViewScrollableContainerRectangleShown = false;

    d->paintBlankCounter = 0;

    // Don't waste CPU drawing default background since its overridden by
    // our fully opaque drawing.  In reality, this seems to make no
    // difference in performance.
    //
    // TODO: qt.html says "Setting this flag implicitly disables double
    //       buffering for the widget" but we don't get flicker...
    //       Only WA_PaintOnScreen really seems to disable double buffering.
    //       Am I confusing that with disabling the backing store?
    //
    //       http://lists.trolltech.com/qt-interest/2005-08/msg00865.html
    //       suggests that the widget becomes see-through temporarily
    //       when switching from another desktop (bad) but I can't reproduce
    //       (maybe because it's not a toplevel widget?).  Making the pixmap
    //       a brush (and having an empty paintEvent) is suggested for
    //       speed if you don't mind resize glitches.
    setAttribute (Qt::WA_NoSystemBackground, true);

    setFocusPolicy (Qt::WheelFocus);
    setMouseTracking (true);  // mouseMoveEvent's even when no mousebtn down
    setAttribute (Qt::WA_KeyCompression, true);

    // COMPAT: Need to update InputMethod support.
    //setAttribute (Qt::WA_InputMethodEnabled, true);  // ensure using InputMethod
}

kpView::~kpView ()
{
    setHasMouse (false);

    delete d;
}


// public
kpDocument *kpView::document () const
{
    return d->document;
}

// protected
kpAbstractSelection *kpView::selection () const
{
    return document () ? document ()->selection () : 0;
}

// protected
kpTextSelection *kpView::textSelection () const
{
    return document () ? document ()->textSelection () : 0;
}


// public
kpToolToolBar *kpView::toolToolBar () const
{
    return d->toolToolBar;
}

// protected
kpTool *kpView::tool () const
{
    return toolToolBar () ? toolToolBar ()->tool () : 0;
}

// public
kpViewManager *kpView::viewManager () const
{
    return d->viewManager;
}

// public
kpView *kpView::buddyView () const
{
    return d->buddyView;
}

// public
kpViewScrollableContainer *kpView::buddyViewScrollableContainer () const
{
    return (buddyView () ? buddyView ()->scrollableContainer () : 0);
}

// public
kpViewScrollableContainer *kpView::scrollableContainer () const
{
    return d->scrollableContainer;
}


// public
int kpView::zoomLevelX (void) const
{
    return d->hzoom;
}

// public
int kpView::zoomLevelY (void) const
{
    return d->vzoom;
}

// public virtual
void kpView::setZoomLevel (int hzoom, int vzoom)
{
    hzoom = qBound (MinZoomLevel, hzoom, MaxZoomLevel);
    vzoom = qBound (MinZoomLevel, vzoom, MaxZoomLevel);

    if (hzoom == d->hzoom && vzoom == d->vzoom)
        return;

    d->hzoom = hzoom;
    d->vzoom = vzoom;

    if (viewManager ())
        viewManager ()->updateView (this);

    emit zoomLevelChanged (hzoom, vzoom);
}


// public
QPoint kpView::origin () const
{
    return d->origin;
}

// public virtual
void kpView::setOrigin (const QPoint &origin)
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView(" << objectName () << ")::setOrigin" << origin;
#endif

    if (origin == d->origin)
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tNOP";
    #endif
        return;
    }

    d->origin = origin;

    if (viewManager ())
        viewManager ()->updateView (this);

    emit originChanged (origin);
}


// public
bool kpView::canShowGrid () const
{
    // (minimum zoom level < 400% would probably be reported as a bug by
    //  users who thought that the grid was a part of the image!)
    return ((zoomLevelX () >= 400 && zoomLevelX () % 100 == 0) &&
            (zoomLevelY () >= 400 && zoomLevelY () % 100 == 0));
}

// public
bool kpView::isGridShown () const
{
    return d->showGrid;
}

// public
void kpView::showGrid (bool yes)
{
    if (d->showGrid == yes)
        return;

    if (yes && !canShowGrid ())
        return;

    d->showGrid = yes;

    if (viewManager ())
        viewManager ()->updateView (this);
}


// public
bool kpView::isBuddyViewScrollableContainerRectangleShown () const
{
    return d->isBuddyViewScrollableContainerRectangleShown;
}

// public
void kpView::showBuddyViewScrollableContainerRectangle (bool yes)
{
    if (yes == d->isBuddyViewScrollableContainerRectangleShown)
        return;

    d->isBuddyViewScrollableContainerRectangleShown = yes;

    if (d->isBuddyViewScrollableContainerRectangleShown)
    {
        // Got these connect statements by analysing deps of
        // updateBuddyViewScrollableContainerRectangle() rect update code.

        connect (this, SIGNAL (zoomLevelChanged (int, int)),
                 this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
        connect (this, SIGNAL (originChanged (const QPoint &)),
                 this, SLOT (updateBuddyViewScrollableContainerRectangle ()));

        if (buddyViewScrollableContainer ())
        {
            connect (buddyViewScrollableContainer (), SIGNAL (contentsMovingSoon (int, int)),
                     this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
            connect (buddyViewScrollableContainer (), SIGNAL (resized ()),
                     this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
        }

        if (buddyView ())
        {
            connect (buddyView (), SIGNAL (zoomLevelChanged (int, int)),
                     this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
            connect (buddyView (), SIGNAL (originChanged (const QPoint &)),
                     this, SLOT (updateBuddyViewScrollableContainerRectangle ()));

            connect (buddyView (), SIGNAL (sizeChanged (int, int)),
                     this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
        }

    }
    else
    {
        disconnect (this, SIGNAL (zoomLevelChanged (int, int)),
                    this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
        disconnect (this, SIGNAL (originChanged (const QPoint &)),
                    this, SLOT (updateBuddyViewScrollableContainerRectangle ()));

        if (buddyViewScrollableContainer ())
        {
            disconnect (buddyViewScrollableContainer (), SIGNAL (contentsMovingSoon (int, int)),
                        this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
            disconnect (buddyViewScrollableContainer (), SIGNAL (resized ()),
                        this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
        }

        if (buddyView ())
        {
            disconnect (buddyView (), SIGNAL (zoomLevelChanged (int, int)),
                        this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
            disconnect (buddyView (), SIGNAL (originChanged (const QPoint &)),
                        this, SLOT (updateBuddyViewScrollableContainerRectangle ()));

            disconnect (buddyView (), SIGNAL (sizeChanged (int, int)),
                        this, SLOT (updateBuddyViewScrollableContainerRectangle ()));
        }

    }

    updateBuddyViewScrollableContainerRectangle ();
}


// protected
QRect kpView::buddyViewScrollableContainerRectangle () const
{
    return d->buddyViewScrollableContainerRectangle;
}

// protected slot
void kpView::updateBuddyViewScrollableContainerRectangle ()
{
    if (viewManager ())
        viewManager ()->setQueueUpdates ();

    {
        if (d->buddyViewScrollableContainerRectangle.isValid ())
        {
            if (viewManager ())
            {
                // Erase last
                viewManager ()->updateViewRectangleEdges (this,
                    d->buddyViewScrollableContainerRectangle);
            }
        }


        QRect newRect;
        if (isBuddyViewScrollableContainerRectangleShown () &&
            buddyViewScrollableContainer () && buddyView ())
        {
            QRect docRect = buddyView ()->transformViewToDoc (
                QRect (buddyViewScrollableContainer ()->contentsXSoon (),
                       buddyViewScrollableContainer ()->contentsYSoon (),
                       qMin (buddyView ()->width (),
                             buddyViewScrollableContainer ()->visibleWidth ()),
                       qMin (buddyView ()->height (),
                             buddyViewScrollableContainer ()->visibleHeight ())));


            QRect viewRect = this->transformDocToView (docRect);


            // (Surround the area of interest by moving outwards by 1 pixel in each
            //  direction - don't overlap area)
            newRect = QRect (viewRect.x () - 1,
                             viewRect.y () - 1,
                             viewRect.width () + 2,
                             viewRect.height () + 2);
        }
        else
        {
            newRect = QRect ();
        }

        if (newRect != d->buddyViewScrollableContainerRectangle)
        {
            // (must set before updateView() for paintEvent() to see new
            //  rect)
            d->buddyViewScrollableContainerRectangle = newRect;

            if (newRect.isValid ())
            {
                if (viewManager ())
                {
                    viewManager ()->updateViewRectangleEdges (this,
                        d->buddyViewScrollableContainerRectangle);
                }
            }
        }
    }

    if (viewManager ())
        viewManager ()->restoreQueueUpdates ();
}


// public
double kpView::transformViewToDocX (double viewX) const
{
    return (viewX - origin ().x ()) * 100.0 / zoomLevelX ();
}

// public
double kpView::transformViewToDocY (double viewY) const
{
    return (viewY - origin ().y ()) * 100.0 / zoomLevelY ();
}

// public
QPoint kpView::transformViewToDoc (const QPoint &viewPoint) const
{
    return QPoint ((int) transformViewToDocX (viewPoint.x ()),
                   (int) transformViewToDocY (viewPoint.y ()));
}

// public
QRect kpView::transformViewToDoc (const QRect &viewRect) const
{
    if (zoomLevelX () == 100 && zoomLevelY () == 100)
    {
        return QRect (viewRect.x () - origin ().x (),
                      viewRect.y () - origin ().y (),
                      viewRect.width (),
                      viewRect.height ());
    }
    else
    {
        const QPoint docTopLeft = transformViewToDoc (viewRect.topLeft ());

        // (don't call transformViewToDoc[XY]() - need to round up dimensions)
        const int docWidth = qRound (double (viewRect.width ()) * 100.0 / double (zoomLevelX ()));
        const int docHeight = qRound (double (viewRect.height ()) * 100.0 / double (zoomLevelY ()));

        // (like QWMatrix::Areas)
        return QRect (docTopLeft.x (), docTopLeft.y (), docWidth, docHeight);
    }
}


// public
double kpView::transformDocToViewX (double docX) const
{
    return (docX * zoomLevelX () / 100.0) + origin ().x ();
}

// public
double kpView::transformDocToViewY (double docY) const
{
    return (docY * zoomLevelY () / 100.0) + origin ().y ();
}

// public
QPoint kpView::transformDocToView (const QPoint &docPoint) const
{
    return QPoint ((int) transformDocToViewX (docPoint.x ()),
                   (int) transformDocToViewY (docPoint.y ()));
}

// public
QRect kpView::transformDocToView (const QRect &docRect) const
{
    if (zoomLevelX () == 100 && zoomLevelY () == 100)
    {
        return QRect (docRect.x () + origin ().x (),
                      docRect.y () + origin ().y (),
                      docRect.width (),
                      docRect.height ());
    }
    else
    {
        const QPoint viewTopLeft = transformDocToView (docRect.topLeft ());

        // (don't call transformDocToView[XY]() - need to round up dimensions)
        const int viewWidth = qRound (double (docRect.width ()) * double (zoomLevelX ()) / 100.0);
        const int viewHeight = qRound (double (docRect.height ()) * double (zoomLevelY ()) / 100.0);

        // (like QWMatrix::Areas)
        return QRect (viewTopLeft.x (), viewTopLeft.y (), viewWidth, viewHeight);
    }
}


// public
QPoint kpView::transformViewToOtherView (const QPoint &viewPoint,
                                         const kpView *otherView)
{
    if (this == otherView)
        return viewPoint;

    const double docX = transformViewToDocX (viewPoint.x ());
    const double docY = transformViewToDocY (viewPoint.y ());

    const double otherViewX = otherView->transformDocToViewX (docX);
    const double otherViewY = otherView->transformDocToViewY (docY);

    return QPoint ((int) otherViewX, (int) otherViewY);
}


// public
int kpView::zoomedDocWidth () const
{
    return document () ? document ()->width () * zoomLevelX () / 100 : 0;
}

// public
int kpView::zoomedDocHeight () const
{
    return document () ? document ()->height () * zoomLevelY () / 100 : 0;
}


// public
void kpView::setHasMouse (bool yes)
{
    kpViewManager *vm = viewManager ();
    if (!vm)
        return;

#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName ()
               << ")::setHasMouse(" << yes
               << ") existing viewUnderCursor="
               << (vm->viewUnderCursor () ? vm->viewUnderCursor ()->objectName () : "(none)")
               << endl;
#endif
    if (yes && vm->viewUnderCursor () != this)
        vm->setViewUnderCursor (this);
    else if (!yes && vm->viewUnderCursor () == this)
        vm->setViewUnderCursor (0);
}


// public
void kpView::addToQueuedArea (const QRegion &region)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName ()
               << ")::addToQueuedArea() already=" << d->queuedUpdateArea
               << " - plus - " << region
               << endl;
#endif
    d->queuedUpdateArea += region;
}

// public
void kpView::addToQueuedArea (const QRect &rect)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName ()
               << ")::addToQueuedArea() already=" << d->queuedUpdateArea
               << " - plus - " << rect
               << endl;
#endif
    d->queuedUpdateArea += rect;
}

// public
void kpView::invalidateQueuedArea ()
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView::invalidateQueuedArea()";
#endif

    d->queuedUpdateArea = QRegion ();
}

// public
void kpView::updateQueuedArea ()
{
    kpViewManager *vm = viewManager ();
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName ()
               << ")::updateQueuedArea() vm=" << (bool) vm
               << " queueUpdates=" << (vm && vm->queueUpdates ())
               << " fastUpdates=" << (vm && vm->fastUpdates ())
               << " area=" << d->queuedUpdateArea
               << endl;
#endif

    if (!vm)
        return;

    if (vm->queueUpdates ())
        return;

    if (!d->queuedUpdateArea.isEmpty ())
        vm->updateView (this, d->queuedUpdateArea);

    invalidateQueuedArea ();
}

// COMPAT: Need to update InputMethod support.
#if 0
// public
void kpView::updateMicroFocusHint (const QRect &microFocusHint)
{
    int x = microFocusHint.topLeft().x();
    int y = microFocusHint.topLeft().y();
    int width = microFocusHint.width();
    int height = microFocusHint.height();

    (void) x; (void) y; (void) width; (void) height;
    setMicroFocusHint (x, y, width, height);
}
#endif


// public
QPoint kpView::mouseViewPoint (const QPoint &returnViewPoint) const
{
    if (returnViewPoint != KP_INVALID_POINT)
        return returnViewPoint;
    else
    {
        // TODO: I don't think this is right for the main view since that's
        //       inside the scrollview (which can scroll).
        return mapFromGlobal (QCursor::pos ());
    }
}


#include <kpView.moc>

