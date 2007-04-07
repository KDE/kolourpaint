
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


#define DEBUG_KP_VIEW 0
#define DEBUG_KP_VIEW_RENDERER ((DEBUG_KP_VIEW && 1) || 0)


#include <kpView.h>

#include <math.h>
#include <stdlib.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <q3dragobject.h>
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

#if DEBUG_KP_VIEW || DEBUG_KP_VIEW_RENDERER
    #include <qdatetime.h>
#endif

#include <kdebug.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpPixmapFX.h>
#include <kpSelection.h>
#include <kpTempPixmap.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>


struct kpViewPrivate
{
    QPointer <kpDocument> m_document;
    QPointer <kpToolToolBar> m_toolToolBar;
    QPointer <kpViewManager> m_viewManager;
    QPointer <kpView> m_buddyView;
    QPointer <kpViewScrollableContainer> m_scrollableContainer;

    int m_hzoom, m_vzoom;
    QPoint m_origin;
    bool m_showGrid;
    bool m_isBuddyViewScrollableContainerRectangleShown;
    QRect m_buddyViewScrollableContainerRectangle;

    QRegion m_queuedUpdateArea;
};


kpView::kpView (kpDocument *document,
        kpToolToolBar *toolToolBar,
        kpViewManager *viewManager,
        kpView *buddyView,
        kpViewScrollableContainer *scrollableContainer,
        QWidget *parent)
    : QWidget (parent),
      d (new kpViewPrivate ())
{
    d->m_document = document;
    d->m_toolToolBar = toolToolBar;
    d->m_viewManager = viewManager;
    d->m_buddyView = buddyView;
    d->m_scrollableContainer = scrollableContainer;

    d->m_hzoom = 100, d->m_vzoom = 100;
    d->m_origin = QPoint (0, 0);
    d->m_showGrid = false;
    d->m_isBuddyViewScrollableContainerRectangleShown = false;

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
    setAttribute (Qt::WA_InputMethodEnabled, true);  // ensure using InputMethod
}

kpView::~kpView ()
{
    setHasMouse (false);

    delete d;
}


// public
kpDocument *kpView::document () const
{
    return d->m_document;
}

// protected
kpSelection *kpView::selection () const
{
    return document () ? document ()->selection () : 0;
}

// public
kpToolToolBar *kpView::toolToolBar () const
{
    return d->m_toolToolBar;
}

// protected
kpTool *kpView::tool () const
{
    return toolToolBar () ? toolToolBar ()->tool () : 0;
}

// public
kpViewManager *kpView::viewManager () const
{
    return d->m_viewManager;
}

// public
kpView *kpView::buddyView () const
{
    return d->m_buddyView;
}

// public
kpViewScrollableContainer *kpView::buddyViewScrollableContainer () const
{
    return (buddyView () ? buddyView ()->scrollableContainer () : 0);
}

// public
kpViewScrollableContainer *kpView::scrollableContainer () const
{
    return d->m_scrollableContainer;
}


// public
int kpView::zoomLevelX (void) const
{
    return d->m_hzoom;
}

// public
int kpView::zoomLevelY (void) const
{
    return d->m_vzoom;
}

// public virtual
void kpView::setZoomLevel (int hzoom, int vzoom)
{
    if (hzoom == d->m_hzoom && vzoom == d->m_vzoom)
        return;

    if (hzoom <= 0 || vzoom <= 0)
        return;

    d->m_hzoom = hzoom;
    d->m_vzoom = vzoom;

    if (viewManager ())
        viewManager ()->updateView (this);

    emit zoomLevelChanged (hzoom, vzoom);
}


// public
QPoint kpView::origin () const
{
    return d->m_origin;
}

// public virtual
void kpView::setOrigin (const QPoint &origin)
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView(" << objectName () << ")::setOrigin" << origin << endl;
#endif

    if (origin == d->m_origin)
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tNOP" << endl;
    #endif
        return;
    }

    d->m_origin = origin;

    if (viewManager ())
        viewManager ()->updateView (this);

    emit originChanged (origin);
}


// public
bool kpView::canShowGrid () const
{
    // (minimum zoom level < 600% would probably be reported as a bug by
    //  users who thought that the grid was a part of the image!)
    return ((zoomLevelX () >= 600 && zoomLevelX () % 100 == 0) &&
            (zoomLevelY () >= 600 && zoomLevelY () % 100 == 0));
}

// public
bool kpView::isGridShown () const
{
    return d->m_showGrid;
}

// public
void kpView::showGrid (bool yes)
{
    if (d->m_showGrid == yes)
        return;

    if (yes && !canShowGrid ())
        return;

    d->m_showGrid = yes;

    if (viewManager ())
        viewManager ()->updateView (this);
}


// public
bool kpView::isBuddyViewScrollableContainerRectangleShown () const
{
    return d->m_isBuddyViewScrollableContainerRectangleShown;
}

// public
void kpView::showBuddyViewScrollableContainerRectangle (bool yes)
{
    if (yes == d->m_isBuddyViewScrollableContainerRectangleShown)
        return;

    d->m_isBuddyViewScrollableContainerRectangleShown = yes;

    if (d->m_isBuddyViewScrollableContainerRectangleShown)
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
    return d->m_buddyViewScrollableContainerRectangle;
}

// protected slot
void kpView::updateBuddyViewScrollableContainerRectangle ()
{
    if (viewManager ())
        viewManager ()->setQueueUpdates ();

    {
        if (d->m_buddyViewScrollableContainerRectangle.isValid ())
        {
            if (viewManager ())
            {
                // Erase last
                viewManager ()->updateViewRectangleEdges (this,
                    d->m_buddyViewScrollableContainerRectangle);
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

        if (newRect != d->m_buddyViewScrollableContainerRectangle)
        {
            // (must set before updateView() for paintEvent() to see new
            //  rect)
            d->m_buddyViewScrollableContainerRectangle = newRect;

            if (newRect.isValid ())
            {
                if (viewManager ())
                {
                    viewManager ()->updateViewRectangleEdges (this,
                        d->m_buddyViewScrollableContainerRectangle);
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
               << ")::addToQueuedArea() already=" << d->m_queuedUpdateArea
               << " - plus - " << region
               << endl;
#endif
    d->m_queuedUpdateArea += region;
}

// public
void kpView::addToQueuedArea (const QRect &rect)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName ()
               << ")::addToQueuedArea() already=" << d->m_queuedUpdateArea
               << " - plus - " << rect
               << endl;
#endif
    d->m_queuedUpdateArea += rect;
}

// public
void kpView::invalidateQueuedArea ()
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView::invalidateQueuedArea()" << endl;
#endif

    d->m_queuedUpdateArea = QRegion ();
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
               << " area=" << d->m_queuedUpdateArea
               << endl;
#endif

    if (!vm)
        return;

    if (vm->queueUpdates ())
        return;

    if (!d->m_queuedUpdateArea.isEmpty ())
        vm->updateView (this, d->m_queuedUpdateArea);

    invalidateQueuedArea ();
}

// public
void kpView::updateMicroFocusHint (const QRect &microFocusHint)
{
    int x = microFocusHint.topLeft().x();
    int y = microFocusHint.topLeft().y();
    int width = microFocusHint.width();
    int height = microFocusHint.height();

    // COMPAT
    (void) x; (void) y; (void) width; (void) height;
    //setMicroFocusHint (x, y, width, height);
}

// public
QRect kpView::selectionViewRect () const
{
    return selection () ?
               transformDocToView (selection ()->boundingRect ()) :
               QRect ();

}

// public
QPoint kpView::mouseViewPoint (const QPoint &returnViewPoint) const
{
    if (returnViewPoint != KP_INVALID_POINT)
        return returnViewPoint;
    else
        return mapFromGlobal (QCursor::pos ());
}

// public
QPoint kpView::mouseViewPointRelativeToSelection (const QPoint &viewPoint) const
{
    if (!selection ())
        return KP_INVALID_POINT;

    return mouseViewPoint (viewPoint) - transformDocToView (selection ()->topLeft ());
}

// public
bool kpView::mouseOnSelection (const QPoint &viewPoint) const
{
    const QRect selViewRect = selectionViewRect ();
    if (!selViewRect.isValid ())
        return false;

    return selViewRect.contains (mouseViewPoint (viewPoint));
}


// public
int kpView::textSelectionMoveBorderAtomicSize () const
{
    if (!selection () || !selection ()->isText ())
        return 0;

    return qMax (4, zoomLevelX () / 100);
}

// public
bool kpView::mouseOnSelectionToMove (const QPoint &viewPoint) const
{
    if (!mouseOnSelection (viewPoint))
        return false;

    if (!selection ()->isText ())
        return true;

    if (mouseOnSelectionResizeHandle (viewPoint))
        return false;


    const QPoint viewPointRelSel = mouseViewPointRelativeToSelection (viewPoint);

    // Middle point should always be selectable
    const QPoint selCenterDocPoint = selection ()->boundingRect ().center ();
    if (tool () &&
        tool ()->calculateCurrentPoint () == selCenterDocPoint)
    {
        return false;
    }


    const int atomicSize = textSelectionMoveBorderAtomicSize ();
    const QRect selViewRect = selectionViewRect ();

    return (viewPointRelSel.x () < atomicSize ||
            viewPointRelSel.x () >= selViewRect.width () - atomicSize ||
            viewPointRelSel.y () < atomicSize ||
            viewPointRelSel.y () >= selViewRect.height () - atomicSize);
}


// protected
bool kpView::selectionLargeEnoughToHaveResizeHandlesIfAtomicSize (int atomicSize) const
{
    if (!selection ())
        return false;

    const QRect selViewRect = selectionViewRect ();

    return (selViewRect.width () >= atomicSize * 5 ||
            selViewRect.height () >= atomicSize * 5);
}

// public
int kpView::selectionResizeHandleAtomicSize () const
{
    int atomicSize = qMin (7, qMax (4, zoomLevelX () / 100));
    while (atomicSize > 0 &&
           !selectionLargeEnoughToHaveResizeHandlesIfAtomicSize (atomicSize))
    {
        atomicSize--;
    }

    return atomicSize;
}

// public
bool kpView::selectionLargeEnoughToHaveResizeHandles () const
{
    return (selectionResizeHandleAtomicSize () > 0);
}

// public
QRegion kpView::selectionResizeHandlesViewRegion (bool forRenderer) const
{
    QRegion ret;

    const int atomicLength = selectionResizeHandleAtomicSize ();
    if (atomicLength <= 0)
        return QRegion ();


    // HACK: At low zoom (e.g. 100%), resize handles will probably be too
    //       big and overlap text / cursor / too much of selection.
    //
    //       So limit the _visual_ size of handles at low zoom.  The
    //       handles' grab area remains the same for usability; so yes,
    //       there are a few pixels that don't look grabable but they are.
    //
    //       The real solution is to be able to partially render the
    //       handles outside of the selection view rect.  If not possible,
    //       at least for text boxes, render text on top of handles.
    int normalAtomicLength = atomicLength;
    int vertEdgeAtomicLength = atomicLength;
    if (forRenderer && selection ())
    {
        if (zoomLevelX () <= 150)
        {
            if (normalAtomicLength > 1)
                normalAtomicLength--;

            if (vertEdgeAtomicLength > 1)
                vertEdgeAtomicLength--;
        }

        // 1 line of text?
        if (selection ()->isText () && selection ()->textLines ().size () == 1)
        {
            if (zoomLevelX () <= 150)
                vertEdgeAtomicLength = qMin (vertEdgeAtomicLength, qMax (2, zoomLevelX () / 100));
            else if (zoomLevelX () <= 250)
                vertEdgeAtomicLength = qMin (vertEdgeAtomicLength, qMax (3, zoomLevelX () / 100));
        }
    }


    const QRect selViewRect = selectionViewRect ();

#define ADD_BOX_RELATIVE_TO_SELECTION(type,x,y)    \
    ret += QRect ((x), (y), type##AtomicLength, type##AtomicLength)

    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   selViewRect.width () - normalAtomicLength,
                                   selViewRect.height () - normalAtomicLength);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   selViewRect.width () - normalAtomicLength,
                                   0);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   0,
                                   selViewRect.height () - normalAtomicLength);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   0,
                                   0);

    ADD_BOX_RELATIVE_TO_SELECTION (vertEdge,
                                   selViewRect.width () - vertEdgeAtomicLength,
                                   (selViewRect.height () - vertEdgeAtomicLength) / 2);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   (selViewRect.width () - normalAtomicLength) / 2,
                                   selViewRect.height () - normalAtomicLength);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   (selViewRect.width () - normalAtomicLength) / 2,
                                   0);
    ADD_BOX_RELATIVE_TO_SELECTION (vertEdge,
                                   0,
                                   (selViewRect.height () - vertEdgeAtomicLength) / 2);

#undef ADD_BOX_RELATIVE_TO_SELECTION

    ret.translate (selViewRect.x (), selViewRect.y ());
    ret = ret.intersect (selViewRect);

    return ret;
}

// public
// COMPAT: use Qt Flags to be safer
int kpView::mouseOnSelectionResizeHandle (const QPoint &viewPoint) const
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView::mouseOnSelectionResizeHandle(viewPoint="
               << viewPoint << ")" << endl;
#endif

    if (!mouseOnSelection (viewPoint))
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tmouse not on sel" << endl;
    #endif
        return 0;
    }


    const QRect selViewRect = selectionViewRect ();
#if DEBUG_KP_VIEW
    kDebug () << "\tselViewRect=" << selViewRect << endl;
#endif


    const int atomicLength = selectionResizeHandleAtomicSize ();
#if DEBUG_KP_VIEW
    kDebug () << "\tatomicLength=" << atomicLength << endl;
#endif

    if (atomicLength <= 0)
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tsel not large enough to have resize handles" << endl;
    #endif
        // Want to make it possible to move a small selection
        return 0;
    }


    const QPoint viewPointRelSel = mouseViewPointRelativeToSelection (viewPoint);
#if DEBUG_KP_VIEW
    kDebug () << "\tviewPointRelSel=" << viewPointRelSel << endl;
#endif


#define LOCAL_POINT_IN_BOX_AT(x,y)  \
    QRect ((x), (y), atomicLength, atomicLength).contains (viewPointRelSel)

    // Favour the bottom & right and the corners.
    if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength,
                               selViewRect.height () - atomicLength))
    {
        return kpView::Bottom | kpView::Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength, 0))
    {
        return kpView::Top | kpView::Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, selViewRect.height () - atomicLength))
    {
        return kpView::Bottom | kpView::Left;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, 0))
    {
        return kpView::Top | kpView::Left;
    }
    else if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength,
                                    (selViewRect.height () - atomicLength) / 2))
    {
        return kpView::Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT ((selViewRect.width () - atomicLength) / 2,
                                    selViewRect.height () - atomicLength))
    {
        return kpView::Bottom;
    }
    else if (LOCAL_POINT_IN_BOX_AT ((selViewRect.width () - atomicLength) / 2, 0))
    {
        return kpView::Top;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, (selViewRect.height () - atomicLength) / 2))
    {
        return kpView::Left;
    }
    else
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tnot on sel resize handle" << endl;
    #endif
        return 0;
    }
#undef LOCAL_POINT_IN_BOX_AT
}

// public
bool kpView::mouseOnSelectionToSelectText (const QPoint &viewPoint) const
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView::mouseOnSelectionToSelectText(viewPoint="
               << viewPoint << ")" << endl;
#endif

    if (!mouseOnSelection (viewPoint))
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tmouse non on sel" << endl;
    #endif
        return false;
    }

    if (!selection ()->isText ())
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tsel not text" << endl;
    #endif
        return false;
    }

#if DEBUG_KP_VIEW
    kDebug () << "\tmouse on sel: to move=" << mouseOnSelectionToMove ()
               << " to resize=" << mouseOnSelectionResizeHandle ()
               << endl;
#endif

    return (!mouseOnSelectionToMove (viewPoint) &&
            !mouseOnSelectionResizeHandle (viewPoint));
}


// protected virtual [base QWidget]
void kpView::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::mouseMoveEvent ("
               << e->x () << "," << e->y () << ")"
               << endl;
#endif

    // TODO: This is wrong if you leaveEvent the mainView by mouseMoving on the
    //       mainView, landing on top of the thumbnailView cleverly put on top
    //       of the mainView.
    setHasMouse (rect ().contains (e->pos ()));

    if (tool ())
        tool ()->mouseMoveEvent (e);

    e->accept ();
}

// protected virtual [base QWidget]
void kpView::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::mousePressEvent ("
               << e->x () << "," << e->y () << ")"
               << endl;
#endif

    setHasMouse (true);

    if (tool ())
        tool ()->mousePressEvent (e);

    e->accept ();
}

// protected virtual [base QWidget]
void kpView::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::mouseReleaseEvent ("
               << e->x () << "," << e->y () << ")"
               << endl;
#endif

    setHasMouse (rect ().contains (e->pos ()));

    if (tool ())
        tool ()->mouseReleaseEvent (e);

    e->accept ();
}

// public virtual [base QWidget]
void kpView::wheelEvent (QWheelEvent *e)
{
    if (tool ())
        tool ()->wheelEvent (e);
}


// protected virtual [base QWidget]
bool kpView::event (QEvent *e)
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView::event() invoking kpTool::event()" << endl;
#endif
    if (tool () && tool ()->viewEvent (e))
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tkpView::event() - tool said eat event, ret true" << endl;
    #endif
        return true;
    }

#if DEBUG_KP_VIEW
    kDebug () << "\tkpView::event() - no tool or said false, call QWidget::event()" << endl;
#endif
    return QWidget::event (e);
}

// protected virtual [base QWidget]
void kpView::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::keyPressEvent()" << endl;
#endif

    if (tool ())
        tool ()->keyPressEvent (e);

    e->accept ();
}

// protected virtual [base QWidget]
void kpView::keyReleaseEvent (QKeyEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::keyReleaseEvent()" << endl;
#endif

    if (tool ())
        tool ()->keyReleaseEvent (e);

    e->accept ();
}


// protected virtual [base QWidget]
void kpView::focusInEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::focusInEvent()" << endl;
#endif
    if (tool ())
        tool ()->focusInEvent (e);
}

// protected virtual [base QWidget]
void kpView::focusOutEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::focusOutEvent()" << endl;
#endif
    if (tool ())
        tool ()->focusOutEvent (e);
}


// protected virtual [base QWidget]
void kpView::enterEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::enterEvent()" << endl;
#endif

    // Don't call setHasMouse(true) as it displays the brush cursor (if
    // active) when dragging open a menu and then dragging
    // past the extents of the menu due to Qt sending us an EnterEvent.
    // We're already covered by MouseMoveEvent anyway.
    //
    // But disabling this causes a more serious problem: RMB on a text
    // box and Esc.  We have no other reliable way to determine if the
    // mouse is still above the view (user could have moved mouse out
    // while RMB menu was up) and hence the cursor is not updated.
    setHasMouse (true);

    if (tool ())
        tool ()->enterEvent (e);
}

// protected virtual [base QWidget]
void kpView::leaveEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kDebug () << "kpView(" << objectName () << ")::leaveEvent()" << endl;
#endif

    setHasMouse (false);
    if (tool ())
        tool ()->leaveEvent (e);
}


// protected virtual [base QWidget]
void kpView::dragEnterEvent (QDragEnterEvent *)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName () << ")::dragEnterEvent()" << endl;
#endif

    setHasMouse (true);
}

// protected virtual [base QWidget]
void kpView::dragLeaveEvent (QDragLeaveEvent *)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName () << ")::dragLeaveEvent" << endl;
#endif

    setHasMouse (false);
}


// public virtual [base QWidget]
void kpView::resize (int w, int h)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName ()
               << ")::resize(" << w << "," << h << ")"
               << endl;
#endif

    QWidget::resize (w, h);
}

// protected virtual [base QWidget]
void kpView::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName () << ")::resizeEvent("
               << e->size ()
               << " vs actual=" << size ()
               << ") old=" << e->oldSize () << endl;
#endif

    QWidget::resizeEvent (e);

    emit sizeChanged (width (), height ());
    emit sizeChanged (size ());
}


// COMPAT
#if 0
// private virtual
void kpView::imStartEvent (QIMEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName () << ")::imStartEvent" << endl;
#endif

    if (tool ())
        tool ()->imStartEvent (e);
    e->accept();
}

// private virtual
void kpView::imComposeEvent (QIMEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName () << ")::imComposeEvent" << endl;
#endif

    if (tool ())
        tool ()->imComposeEvent (e);
    e->accept();
}

// private virtual
void kpView::imEndEvent (QIMEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kDebug () << "kpView(" << objectName () << ")::imEndEvent" << endl;
#endif

    if (tool ())
        tool ()->imEndEvent (e);
    e->accept();
}
#endif


//
// Renderer
//

// protected
QRect kpView::paintEventGetDocRect (const QRect &viewRect) const
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::paintEventGetDocRect(" << viewRect << ")" << endl;
#endif

    QRect docRect;

    // From the "we aren't sure whether to round up or round down" department:

    if (zoomLevelX () < 100 || zoomLevelY () < 100)
        docRect = transformViewToDoc (viewRect);
    else
    {
        // think of a grid - you need to fully cover the zoomed-in pixels
        // when docRect is zoomed back to the view later
        docRect = QRect (transformViewToDoc (viewRect.topLeft ()),  // round down
                         transformViewToDoc (viewRect.bottomRight ()));  // round down
    }

    if (zoomLevelX () % 100 || zoomLevelY () % 100)
    {
        // at least round up the bottom-right point and deal with matrix weirdness:
        // - helpful because it ensures we at least cover the required area
        //   at e.g. 67% or 573%
        // - harmless since Qt clips for us anyway
        docRect.setBottomRight (docRect.bottomRight () + QPoint (2, 2));
    }

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tdocRect=" << docRect << endl;
#endif
    kpDocument *doc = document ();
    if (doc)
    {
        docRect = docRect.intersect (doc->rect ());
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tintersect with doc=" << docRect << endl;
    #endif
    }

    return docRect;
}

// public static
void kpView::drawTransparentBackground (QPainter *painter,
                                        const QPoint &patternOrigin,
                                        const QRect &viewRect,
                                        bool isPreview)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::drawTransparentBackground() patternOrigin="
              << patternOrigin
              << " viewRect=" << viewRect
              << " isPreview=" << isPreview
               << endl;
#endif

    const int cellSize = !isPreview ? 16 : 10;

    // TODO: % is unpredictable with negatives.

    int starty = viewRect.y ();
    if ((starty - patternOrigin.y ()) % cellSize)
        starty -= ((starty - patternOrigin.y ()) % cellSize);

    int startx = viewRect.x ();
    if ((startx - patternOrigin.x ()) % cellSize)
        startx -= ((startx - patternOrigin.x ()) % cellSize);

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tstartXY=" << QPoint (startx, starty) << endl;
#endif

    painter->save ();
    
    // Clip to <viewRect> as we may draw outside it on all sides.
    painter->setClipRect (viewRect, Qt::IntersectClip/*honor existing clip*/);
    
    for (int y = starty; y <= viewRect.bottom (); y += cellSize)
    {
        for (int x = startx; x <= viewRect.right (); x += cellSize)
        {
            bool parity = ((x - patternOrigin.x ()) / cellSize +
                (y - patternOrigin.y ()) / cellSize) % 2;
            QColor col;

            if (parity)
            {
                if (!isPreview)
                    col = QColor (213, 213, 213);
                else
                    col = QColor (224, 224, 224);
            }
            else
                col = Qt::white;

            painter->fillRect (x, y, cellSize, cellSize, col);
        }
    }

    painter->restore ();
}

// protected
void kpView::paintEventDrawCheckerBoard (QPainter *painter, const QRect &viewRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView(" << objectName ()
               << ")::paintEventDrawCheckerBoard(viewRect=" << viewRect
               << ") origin=" << origin () << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    QPoint patternOrigin = origin ();

    if (scrollableContainer ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tscrollableContainer: contents[XY]="
                   << QPoint (scrollableContainer ()->contentsX (),
                              scrollableContainer ()->contentsY ())
                   << " contents[XY]Soon="
                   << QPoint (scrollableContainer ()->contentsXSoon (),
                              scrollableContainer ()->contentsYSoon ())
                   << endl;
    #endif
        // Make checkerboard appear static relative to the scroll view.
        // This makes it more obvious that any visible bits of the
        // checkboard represent transparent pixels and not gray and white
        // squares.
        patternOrigin = QPoint (scrollableContainer ()->contentsXSoon (),
                                scrollableContainer ()->contentsYSoon ());
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\t\tpatternOrigin=" << patternOrigin << endl;
    #endif
    }

    // TODO: this static business doesn't work yet
    patternOrigin = QPoint (0, 0);

    drawTransparentBackground (painter, patternOrigin, viewRect);
}

// protected
void kpView::paintEventDrawSelection (QPixmap *destPixmap, const QRect &docRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1 || 0
    kDebug () << "kpView::paintEventDrawSelection() docRect=" << docRect << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
    #if DEBUG_KP_VIEW_RENDERER && 1 || 0
        kDebug () << "\tno doc - abort" << endl;
    #endif
        return;
    }

    kpSelection *sel = doc->selection ();
    if (!sel)
    {
    #if DEBUG_KP_VIEW_RENDERER && 1 || 0
        kDebug () << "\tno sel - abort" << endl;
    #endif
        return;
    }


    //
    // Draw selection pixmap (if there is one)
    //
#if DEBUG_KP_VIEW_RENDERER && 1 || 0
    kDebug () << "\tdraw sel pixmap @ " << sel->topLeft () << endl;
#endif
    sel->paint (destPixmap, docRect);


    //
    // Draw selection border
    //

    kpViewManager *vm = viewManager ();
#if DEBUG_KP_VIEW_RENDERER && 1 || 0
    kDebug () << "\tsel border visible="
               << vm->selectionBorderVisible ()
               << endl;
#endif
    if (vm->selectionBorderVisible ())
    {
        sel->paintBorder (destPixmap, docRect, vm->selectionBorderFinished ());
    }


    //
    // Draw text cursor
    //

    if (sel->isText () &&
        vm->textCursorEnabled () &&
        (vm->textCursorBlinkState () ||
        // For the current main window:
        // As long as _any_ view has focus, blink _all_ views not just the
        // one with focus   // !this->isActiveWindow ()
        !vm->activeView ()))  // sync: call will break when vm is not held by 1 mainWindow
    {
        // TODO: fix code duplication with kpViewManager::updateTextCursor()
        QPoint topLeft = sel->pointForTextRowCol (vm->textCursorRow (), vm->textCursorCol ());
        if (topLeft != KP_INVALID_POINT)
        {
            QRect rect = QRect (topLeft.x (), topLeft.y (),
                                1, sel->textStyle ().fontMetrics ().height ());
            rect = rect.intersect (sel->textAreaRect ());
            if (!rect.isEmpty ())
            {
                kpPixmapFX::fillXORRect (destPixmap,
                    rect.x () - docRect.x (), rect.y () - docRect.y (),
                    rect.width (), rect.height (),
                    kpColor::White/*XOR color*/,
                    kpColor::DarkGray/*hint color if XOR not supported*/);
            }
        }
    }
}

// protected
bool kpView::selectionResizeHandleAtomicSizeCloseToZoomLevel () const
{
    return (abs (selectionResizeHandleAtomicSize () - zoomLevelX () / 100) < 3);
}

// protected
void kpView::paintEventDrawSelectionResizeHandles (const QRect &clipRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::paintEventDrawSelectionResizeHandles("
               << clipRect << ")" << endl;
#endif

    if (!selectionLargeEnoughToHaveResizeHandles ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tsel not large enough to have resize handles" << endl;
    #endif
        return;
    }

    kpViewManager *vm = viewManager ();
    if (!vm || !vm->selectionBorderVisible () || !vm->selectionBorderFinished ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tsel border not visible or not finished" << endl;
    #endif

        return;
    }

    const QRect selViewRect = selectionViewRect ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tselViewRect=" << selViewRect << endl;
#endif
    if (!selViewRect.intersects (clipRect))
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tdoesn't intersect viewRect" << endl;
    #endif
        return;
    }

    QRegion selResizeHandlesRegion = selectionResizeHandlesViewRegion (true/*for renderer*/);
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tsel resize handles view region="
               << selResizeHandlesRegion << endl;
#endif

    const bool doXor = !selectionResizeHandleAtomicSizeCloseToZoomLevel ();
    foreach (QRect r, selResizeHandlesRegion.rects ())
    {
        QRect s = r.intersect (clipRect);
        if (s.isEmpty ())
            continue;

        if (doXor)
        {
            kpPixmapFX::widgetFillXORRect (this,
                s.x (), s.y (), s.width (), s.height (),
                kpColor::White/*XOR color*/,
                kpColor::LightGray/*hint color if XOR not supported*/);
        }
        else
        {
            QPainter p (this);
            p.fillRect (s, Qt::blue);
        }
    }
}

// protected
void kpView::paintEventDrawTempPixmap (QPixmap *destPixmap, const QRect &docRect)
{
    kpViewManager *vm = viewManager ();
    if (!vm)
        return;

    const kpTempPixmap *tpm = vm->tempPixmap ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::paintEventDrawTempPixmap() tempPixmap="
               << tpm
               << " isVisible="
               << (tpm ? tpm->isVisible (vm) : false)
               << endl;
#endif

    if (!tpm || !tpm->isVisible (vm))
        return;

    tpm->paint (destPixmap, docRect);
}

// protected
void kpView::paintEventDrawGridLines (QPainter *painter, const QRect &viewRect)
{
    int hzoomMultiple = zoomLevelX () / 100;
    int vzoomMultiple = zoomLevelY () / 100;

    QPen ordinaryPen (Qt::gray);
    QPen tileBoundaryPen (Qt::lightGray);

    painter->setPen (ordinaryPen);

    // horizontal lines
    int starty = viewRect.top ();
    if (starty % vzoomMultiple)
        starty = (starty + vzoomMultiple) / vzoomMultiple * vzoomMultiple;
#if 0
    int tileHeight = 16 * vzoomMultiple;  // CONFIG
#endif
    for (int y = starty; y <= viewRect.bottom (); y += vzoomMultiple)
    {
    #if 0
        if (tileHeight > 0 && (y - viewRect.y ()) % tileHeight == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }
    #endif

        painter->drawLine (viewRect.left (), y, viewRect.right (), y);

    #if 0
        if (tileHeight > 0 && (y - viewRect.y ()) % tileHeight == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    #endif
    }

    // vertical lines
    int startx = viewRect.left ();
    if (startx % hzoomMultiple)
        startx = (startx + hzoomMultiple) / hzoomMultiple * hzoomMultiple;
#if 0
    int tileWidth = 16 * hzoomMultiple;  // CONFIG
#endif
    for (int x = startx; x <= viewRect.right (); x += hzoomMultiple)
    {
    #if 0
        if (tileWidth > 0 && (x - viewRect.x ()) % tileWidth == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }
    #endif

        painter->drawLine (x, viewRect.top (), x, viewRect.bottom ());

    #if 0
        if (tileWidth > 0 && (x - viewRect.x ()) % tileWidth == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    #endif
    }
}


void kpView::paintEventDrawRect (const QRect &viewRect)
{
#if DEBUG_KP_VIEW_RENDERER
    kDebug () << "\tkpView::paintEventDrawRect(viewRect=" << viewRect
               << ")" << endl;
#endif

    kpViewManager *vm = viewManager ();
    const kpDocument *doc = document ();

    if (!vm || !doc)
        return;


    if (viewRect.isEmpty ())
        return;


    QRect docRect = paintEventGetDocRect (viewRect);

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tdocRect=" << docRect << endl;
#endif


    QPainter painter (this);

    // sync: painter clips to viewRect.
    painter.setClipRect (viewRect);
    

    //
    // Draw checkboard for transparent images and/or views with borders
    //

    QPixmap docPixmap;

    bool tempPixmapWillBeRendered = false;

    if (!docRect.isEmpty ())
    {
        docPixmap = doc->getPixmapAt (docRect);
        KP_PFX_CHECK_NO_ALPHA_CHANNEL (docPixmap);
        
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tdocPixmap.hasAlpha()="
                  << docPixmap.hasAlpha () << endl;
    #endif

        tempPixmapWillBeRendered =
            (!doc->selection () &&
             vm->tempPixmap () &&
             vm->tempPixmap ()->isVisible (vm) &&
             docRect.intersects (vm->tempPixmap ()->rect ()));

    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\ttempPixmapWillBeRendered=" << tempPixmapWillBeRendered
                   << " (sel=" << doc->selection ()
                   << " tempPixmap=" << vm->tempPixmap ()
                   << " tempPixmap.isVisible=" << (vm->tempPixmap () ? vm->tempPixmap ()->isVisible (vm) : false)
                   << " docRect.intersects(tempPixmap.rect)=" << (vm->tempPixmap () ? docRect.intersects (vm->tempPixmap ()->rect ()) : false)
                   << ")"
                   << endl;
    #endif
    }

    if (!docPixmap.mask ().isNull () ||
        (tempPixmapWillBeRendered && vm->tempPixmap ()->mayChangeDocumentMask ()))
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tmask=" << !docPixmap.mask ().isNull ()
                   << endl;
    #endif
        paintEventDrawCheckerBoard (&painter, viewRect);
    }
    else
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tno mask" << endl;
    #endif
    }


    if (!docRect.isEmpty ())
    {
        //
        // Draw docPixmap + tempPixmap
        //

        if (doc->selection ())
        {
            paintEventDrawSelection (&docPixmap, docRect);
        }
        else if (tempPixmapWillBeRendered)
        {
            paintEventDrawTempPixmap (&docPixmap, docRect);
        }

    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\torigin=" << origin () << endl;
    #endif
        // Blit scaled version of docPixmap + tempPixmap.
    #if DEBUG_KP_VIEW_RENDERER && 1
        QTime scaleTimer; scaleTimer.start ();
    #endif
        // sync: ASSUMPTION: painter clips to viewRect.
        painter.translate (origin ().x (), origin ().y ());
        painter.scale (double (zoomLevelX ()) / 100.0,
                       double (zoomLevelY ()) / 100.0);
        painter.drawPixmap (docRect, docPixmap);
        painter.resetMatrix ();  // back to 1-1 scaling
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tscale time=" << scaleTimer.elapsed () << endl;
    #endif

    }  // if (!docRect.isEmpty ()) {


    //
    // Draw Grid Lines
    //

    if (isGridShown ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        QTime gridTimer; gridTimer.start ();
    #endif
        paintEventDrawGridLines (&painter, viewRect);
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tgrid time=" << gridTimer.elapsed () << endl;
    #endif
    }

    painter.end ();


    const QRect bvsvRect = buddyViewScrollableContainerRectangle ();
    if (!bvsvRect.isEmpty ())
    {
        kpPixmapFX::widgetDrawStippledXORRect (this,
            bvsvRect.x (), bvsvRect.y (), bvsvRect.width (), bvsvRect.height (),
            kpColor::White, kpColor::White,  // Stippled XOR colors
            kpColor::LightGray, kpColor::LightGray,
            viewRect);  // Hint colors if XOR not supported
    }


    if (!docRect.isEmpty ())
    {
        if (doc->selection ())
        {
            // Draw resize handles on top of possible grid lines
            paintEventDrawSelectionResizeHandles (viewRect);
        }
    }
}


// protected virtual [base QWidget]
void kpView::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    QTime timer;
    timer.start ();
#endif

    kpViewManager *vm = viewManager ();

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView(" << objectName () << ")::paintEvent() vm=" << (bool) vm
               << " queueUpdates=" << (vm && vm->queueUpdates ())
               << " fastUpdates=" << (vm && vm->fastUpdates ())
               << " viewRect=" << e->rect ()
               << " topLeft=" << QPoint (x (), y ())
               << endl;
#endif

    if (!vm)
        return;

    if (vm->queueUpdates ())
    {
        // OPT: if this update was due to the document,
        //      use document coordinates (in case of a zoom change in
        //      which view coordinates become out of date)
        addToQueuedArea (e->region ());
        return;
    }


    // It seems that e->region() is already clipped by Qt to the visible
    // part of the view (which could be quite small inside a scrollview).
    QRegion viewRegion = e->region ();
    QVector <QRect> rects = viewRegion.rects ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\t#rects = " << rects.count () << endl;
#endif

    for (QVector <QRect>::ConstIterator it = rects.begin ();
         it != rects.end ();
         it++)
    {
        paintEventDrawRect (*it);
    }


#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tall done in: " << timer.restart () << "ms" << endl;
#endif
}


#include <kpView.moc>

