
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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
#define DEBUG_KP_VIEW_RENDERER ((DEBUG_KP_VIEW && 0) || 0)


#include <kpview.h>

#include <math.h>
#include <stdlib.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <qguardedptr.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregion.h>
#include <qmemarray.h>

#if DEBUG_KP_VIEW || DEBUG_KP_VIEW_RENDERER
    #include <qdatetime.h>
#endif

#include <kdebug.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptemppixmap.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kpviewmanager.h>
#include <kpviewscrollablecontainer.h>


struct kpViewPrivate
{
    // sync: kpView::paintEvent()
    //
    // Normally, these pointers must be valid while the kpView is alive.
    // Generally, the objects they point to are deleted only after kpView
    // is deleted.
    //
    // However, sometimes we use deleteLater() for the kpView.
    // Before the delayed deletion is executed, those objects are deleted
    // and then our paintEvent() is called.  paintEvent() must therefore
    // have some way of realising that those objects have been deleted so
    // we use guardded pointers.
    //
    // For more details, see SVN commit:
    //     "r385274 | dang | 2005-02-02 22:08:27 +1100 (Wed, 02 Feb 2005) | 21 lines".
    QGuardedPtr <kpDocument> m_document;
    QGuardedPtr <kpToolToolBar> m_toolToolBar;
    QGuardedPtr <kpViewManager> m_viewManager;
    QGuardedPtr <kpView> m_buddyView;
    QGuardedPtr <kpViewScrollableContainer> m_scrollableContainer;

    int m_hzoom, m_vzoom;
    QPoint m_origin;
    bool m_showGrid;
    bool m_isBuddyViewScrollableContainerRectangleShown;
    QRect m_buddyViewScrollableContainerRectangle;

    QRegion m_queuedUpdateArea;
    QPixmap *m_backBuffer;
};


kpView::kpView (kpDocument *document,
        kpToolToolBar *toolToolBar,
        kpViewManager *viewManager,
        kpView *buddyView,
        kpViewScrollableContainer *scrollableContainer,
        QWidget *parent, const char *name)

    : QWidget (parent, name, Qt::WNoAutoErase/*no flicker*/),
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

    d->m_backBuffer = 0;


    setBackgroundMode (Qt::NoBackground);  // no flicker
    setFocusPolicy (QWidget::WheelFocus);
    setMouseTracking (true);  // mouseMoveEvent's even when no mousebtn down
    setKeyCompression (true);
    setInputMethodEnabled (true);  // ensure using InputMethod
}

kpView::~kpView ()
{
    setHasMouse (false);

    delete d->m_backBuffer;
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
    kdDebug () << "kpView(" << name () << ")::setOrigin" << origin << endl;
#endif

    if (origin == d->m_origin)
    {
    #if DEBUG_KP_VIEW
        kdDebug () << "\tNOP" << endl;
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
    // (minimum zoom level < 400% would probably be reported as a bug by
    //  users who thought that the grid was a part of the image!)
    return ((zoomLevelX () >= 400 && zoomLevelX () % 100 == 0) &&
            (zoomLevelY () >= 400 && zoomLevelY () % 100 == 0));
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
                       QMIN (buddyView ()->width (),
                             buddyViewScrollableContainer ()->visibleWidth ()),
                       QMIN (buddyView ()->height (),
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
    kdDebug () << "kpView(" << name ()
               << ")::setHasMouse(" << yes
               << ") existing viewUnderCursor="
               << (vm->viewUnderCursor () ? vm->viewUnderCursor ()->name () : "(none)")
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
    kdDebug () << "kpView(" << name ()
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
    kdDebug () << "kpView(" << name ()
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
    kdDebug () << "kpView::invalidateQueuedArea()" << endl;
#endif

    d->m_queuedUpdateArea = QRegion ();
}

// public
void kpView::updateQueuedArea ()
{
    kpViewManager *vm = viewManager ();
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView(" << name ()
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

    if (!d->m_queuedUpdateArea.isNull ())
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
    setMicroFocusHint (x, y, width, height);
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

    return QMAX (4, zoomLevelX () / 100);
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
        tool ()->currentPoint () == selCenterDocPoint)
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
    int atomicSize = QMIN (7, QMAX (4, zoomLevelX () / 100));
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
                vertEdgeAtomicLength = QMIN (vertEdgeAtomicLength, QMAX (2, zoomLevelX () / 100));
            else if (zoomLevelX () <= 250)
                vertEdgeAtomicLength = QMIN (vertEdgeAtomicLength, QMAX (3, zoomLevelX () / 100));
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
int kpView::mouseOnSelectionResizeHandle (const QPoint &viewPoint) const
{
#if DEBUG_KP_VIEW
    kdDebug () << "kpView::mouseOnSelectionResizeHandle(viewPoint="
               << viewPoint << ")" << endl;
#endif

    if (!mouseOnSelection (viewPoint))
    {
    #if DEBUG_KP_VIEW
        kdDebug () << "\tmouse not on sel" << endl;
    #endif
        return 0;
    }


    const QRect selViewRect = selectionViewRect ();
#if DEBUG_KP_VIEW
    kdDebug () << "\tselViewRect=" << selViewRect << endl;
#endif


    const int atomicLength = selectionResizeHandleAtomicSize ();
#if DEBUG_KP_VIEW
    kdDebug () << "\tatomicLength=" << atomicLength << endl;
#endif

    if (atomicLength <= 0)
    {
    #if DEBUG_KP_VIEW
        kdDebug () << "\tsel not large enough to have resize handles" << endl;
    #endif
        // Want to make it possible to move a small selection
        return 0;
    }


    const QPoint viewPointRelSel = mouseViewPointRelativeToSelection (viewPoint);
#if DEBUG_KP_VIEW
    kdDebug () << "\tviewPointRelSel=" << viewPointRelSel << endl;
#endif


#define LOCAL_POINT_IN_BOX_AT(x,y)  \
    QRect ((x), (y), atomicLength, atomicLength).contains (viewPointRelSel)

    // Favour the bottom & right and the corners.
    if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength,
                               selViewRect.height () - atomicLength))
    {
        return Bottom | Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength, 0))
    {
        return Top | Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, selViewRect.height () - atomicLength))
    {
        return Bottom | Left;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, 0))
    {
        return Top | Left;
    }
    else if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength,
                                    (selViewRect.height () - atomicLength) / 2))
    {
        return Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT ((selViewRect.width () - atomicLength) / 2,
                                    selViewRect.height () - atomicLength))
    {
        return Bottom;
    }
    else if (LOCAL_POINT_IN_BOX_AT ((selViewRect.width () - atomicLength) / 2, 0))
    {
        return Top;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, (selViewRect.height () - atomicLength) / 2))
    {
        return Left;
    }
    else
    {
    #if DEBUG_KP_VIEW
        kdDebug () << "\tnot on sel resize handle" << endl;
    #endif
        return 0;
    }
#undef LOCAL_POINT_IN_BOX_AT
}

// public
bool kpView::mouseOnSelectionToSelectText (const QPoint &viewPoint) const
{
#if DEBUG_KP_VIEW
    kdDebug () << "kpView::mouseOnSelectionToSelectText(viewPoint="
               << viewPoint << ")" << endl;
#endif

    if (!mouseOnSelection (viewPoint))
    {
    #if DEBUG_KP_VIEW
        kdDebug () << "\tmouse non on sel" << endl;
    #endif
        return false;
    }

    if (!selection ()->isText ())
    {
    #if DEBUG_KP_VIEW
        kdDebug () << "\tsel not text" << endl;
    #endif
        return false;
    }

#if DEBUG_KP_VIEW
    kdDebug () << "\tmouse on sel: to move=" << mouseOnSelectionToMove ()
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
    kdDebug () << "kpView(" << name () << ")::mouseMoveEvent ("
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
    kdDebug () << "kpView(" << name () << ")::mousePressEvent ("
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
    kdDebug () << "kpView(" << name () << ")::mouseReleaseEvent ("
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
void kpView::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView(" << name () << ")::keyPressEvent()" << endl;
#endif

    if (tool ())
        tool ()->keyPressEvent (e);

    e->accept ();
}

// protected virtual [base QWidget]
void kpView::keyReleaseEvent (QKeyEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView(" << name () << ")::keyReleaseEvent()" << endl;
#endif

    if (tool ())
        tool ()->keyReleaseEvent (e);

    e->accept ();
}


// protected virtual [base QWidget]
void kpView::focusInEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView(" << name () << ")::focusInEvent()" << endl;
#endif
    if (tool ())
        tool ()->focusInEvent (e);
}

// protected virtual [base QWidget]
void kpView::focusOutEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView(" << name () << ")::focusOutEvent()" << endl;
#endif
    if (tool ())
        tool ()->focusOutEvent (e);
}


// protected virtual [base QWidget]
void kpView::enterEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView(" << name () << ")::enterEvent()" << endl;
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
    kdDebug () << "kpView(" << name () << ")::leaveEvent()" << endl;
#endif

    setHasMouse (false);
    if (tool ())
        tool ()->leaveEvent (e);
}


// protected virtual [base QWidget]
void kpView::dragEnterEvent (QDragEnterEvent *)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::dragEnterEvent()" << endl;
#endif

    setHasMouse (true);
}

// protected virtual [base QWidget]
void kpView::dragLeaveEvent (QDragLeaveEvent *)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::dragLeaveEvent" << endl;
#endif

    setHasMouse (false);
}


// public virtual [base QWidget]
void kpView::resize (int w, int h)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name ()
               << ")::resize(" << w << "," << h << ")"
               << endl;
#endif

    QWidget::resize (w, h);
}

// protected virtual [base QWidget]
void kpView::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::resizeEvent("
               << e->size ()
               << " vs actual=" << size ()
               << ") old=" << e->oldSize () << endl;
#endif

    QWidget::resizeEvent (e);

    emit sizeChanged (width (), height ());
    emit sizeChanged (size ());
}


// private virtual
void kpView::imStartEvent (QIMEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::imStartEvent" << endl;
#endif

    if (tool ())
        tool ()->imStartEvent (e);
    e->accept();
}

// private virtual
void kpView::imComposeEvent (QIMEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::imComposeEvent" << endl;
#endif

    if (tool ())
        tool ()->imComposeEvent (e);
    e->accept();
}

// private virtual
void kpView::imEndEvent (QIMEvent *e)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::imEndEvent" << endl;
#endif

    if (tool ())
        tool ()->imEndEvent (e);
    e->accept();
}


//
// Renderer
//

// protected
QRect kpView::paintEventGetDocRect (const QRect &viewRect) const
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "kpView::paintEventGetDocRect(" << viewRect << ")" << endl;
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
        // - harmless since paintEventDrawRect() clips for us anyway
        docRect.setBottomRight (docRect.bottomRight () + QPoint (2, 2));
    }

#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\tdocRect=" << docRect << endl;
#endif
    kpDocument *doc = document ();
    if (doc)
    {
        docRect = docRect.intersect (doc->rect ());
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tintersect with doc=" << docRect << endl;
    #endif
    }

    return docRect;
}

// public
void kpView::drawTransparentBackground (QPainter *painter,
                                        int /*viewWidth*/, int /*viewHeight*/,
                                        const QRect &rect,
                                        bool isPreview)
{
    const int cellSize = !isPreview ? 16 : 10;

    int starty = rect.y ();
    if (starty % cellSize)
        starty -= (starty % cellSize);

    int startx = rect.x ();
    if (startx % cellSize)
        startx -= (startx % cellSize);

    painter->save ();
    for (int y = starty; y <= rect.bottom (); y += cellSize)
    {
        for (int x = startx; x <= rect.right (); x += cellSize)
        {
            bool parity = (x / cellSize + y / cellSize) % 2;
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

            painter->fillRect (x - rect.x (), y - rect.y (), cellSize, cellSize,
                               col);
        }
    }
    painter->restore ();
}

// protected
void kpView::paintEventDrawCheckerBoard (QPainter *painter, const QRect &viewRect)
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    drawTransparentBackground (painter,
                               doc->width () * zoomLevelX () / 100,
                               doc->height () * zoomLevelY () / 100,
                               viewRect);
}

// protected
void kpView::paintEventDrawSelection (QPixmap *destPixmap, const QRect &docRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "kpView::paintEventDrawSelection() docRect=" << docRect << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tno doc - abort" << endl;
    #endif
        return;
    }

    kpSelection *sel = doc->selection ();
    if (!sel)
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tno sel - abort" << endl;
    #endif
        return;
    }


    //
    // Draw selection pixmap (if there is one)
    //
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\tdraw sel pixmap @ " << sel->topLeft () << endl;
#endif
    sel->paint (destPixmap, docRect);


    //
    // Draw selection border
    //

    kpViewManager *vm = viewManager ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\tsel border visible="
               << vm->selectionBorderVisible ()
               << endl;
#endif
    if (vm->selectionBorderVisible ())
    {
        QPainter destPixmapPainter (destPixmap);
        destPixmapPainter.setRasterOp (Qt::XorROP);
        destPixmapPainter.setPen (QPen (Qt::white, 1, Qt::DotLine));

        destPixmapPainter.setBackgroundMode (QPainter::OpaqueMode);
        destPixmapPainter.setBackgroundColor (Qt::blue);

        QBitmap maskBitmap;
        QPainter maskBitmapPainter;
        if (destPixmap->mask ())
        {
            maskBitmap = *destPixmap->mask ();
            maskBitmapPainter.begin (&maskBitmap);
            maskBitmapPainter.setPen (Qt::color1/*opaque*/);
        }


    #define PAINTER_CMD(cmd)                 \
    {                                        \
        destPixmapPainter . cmd;             \
        if (maskBitmapPainter.isActive ())   \
            maskBitmapPainter . cmd;         \
    }

        QRect boundingRect = sel->boundingRect ();
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tsel boundingRect="
                   << boundingRect
                   << endl;
    #endif

        if (boundingRect.topLeft () != boundingRect.bottomRight ())
        {
            switch (sel->type ())
            {
            case kpSelection::Rectangle:
            case kpSelection::Text:
            #if DEBUG_KP_VIEW_RENDERER && 1
                kdDebug () << "\tselection border = rectangle" << endl;
                kdDebug () << "\t\tx=" << boundingRect.x () - docRect.x ()
                           << " y=" << boundingRect.y () - docRect.y ()
                           << " w=" << boundingRect.width ()
                           << " h=" << boundingRect.height ()
                           << endl;
            #endif
                PAINTER_CMD (drawRect (boundingRect.x () - docRect.x (),
                                       boundingRect.y () - docRect.y (),
                                       boundingRect.width (),
                                       boundingRect.height ()));
                break;

            case kpSelection::Ellipse:
            #if DEBUG_KP_VIEW_RENDERER && 1
                kdDebug () << "\tselection border = ellipse" << endl;
            #endif
                PAINTER_CMD (drawEllipse (boundingRect.x () - docRect.x (),
                                          boundingRect.y () - docRect.y (),
                                          boundingRect.width (),
                                          boundingRect.height ()));
                break;

            case kpSelection::Points:
            {
            #if DEBUG_KP_VIEW_RENDERER
                kdDebug () << "\tselection border = freeForm" << endl;
            #endif
                QPointArray points = sel->points ();
                points.detach ();
                points.translate (-docRect.x (), -docRect.y ());
                if (vm->selectionBorderFinished ())
                {
                    PAINTER_CMD (drawPolygon (points));
                }
                else
                {
                    PAINTER_CMD (drawPolyline (points));
                }

                break;
            }

            default:
                kdError () << "kpView::paintEventDrawSelection() unknown sel border type" << endl;
                break;
            }


            if (vm->selectionBorderFinished () &&
                (sel->type () == kpSelection::Ellipse ||
                 sel->type () == kpSelection::Points))
            {
                destPixmapPainter.save ();

                destPixmapPainter.setRasterOp (Qt::NotROP);
                PAINTER_CMD (drawRect (boundingRect.x () - docRect.x (),
                                       boundingRect.y () - docRect.y (),
                                       boundingRect.width (),
                                       boundingRect.height ()));

                destPixmapPainter.restore ();
            }
        }
        else
        {
            // SYNC: Work around Qt bug: can't draw 1x1 rectangle
            PAINTER_CMD (drawPoint (boundingRect.topLeft () - docRect.topLeft ()));
        }

    #undef PAINTER_CMD

        destPixmapPainter.end ();
        if (maskBitmapPainter.isActive ())
            maskBitmapPainter.end ();

        destPixmap->setMask (maskBitmap);
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
        // TODO: Fix code duplication: kpViewManager::{setTextCursorPosition,updateTextCursor}() & kpView::paintEventDrawSelection()
        QPoint topLeft = sel->pointForTextRowCol (vm->textCursorRow (), vm->textCursorCol ());
        if (topLeft != KP_INVALID_POINT)
        {
            QRect rect = QRect (topLeft.x (), topLeft.y (),
                                1, sel->textStyle ().fontMetrics ().height ());
            rect = rect.intersect (sel->textAreaRect ());
            if (!rect.isEmpty ())
            {
                rect.moveBy (-docRect.x (), -docRect.y ());

                QBitmap maskBitmap;
                QPainter destPixmapPainter, maskBitmapPainter;

                if (destPixmap->mask ())
                {
                    maskBitmap = *destPixmap->mask ();
                    maskBitmapPainter.begin (&maskBitmap);
                    maskBitmapPainter.fillRect (rect, Qt::color1/*opaque*/);
                    maskBitmapPainter.end ();
                }

                destPixmapPainter.begin (destPixmap);
                destPixmapPainter.setRasterOp (Qt::XorROP);
                destPixmapPainter.fillRect (rect, Qt::white);
                destPixmapPainter.end ();

                if (!maskBitmap.isNull ())
                    destPixmap->setMask (maskBitmap);
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
void kpView::paintEventDrawSelectionResizeHandles (QPainter *painter, const QRect &viewRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "kpView::paintEventDrawSelectionResizeHandles("
               << viewRect << ")" << endl;
#endif

    if (!selectionLargeEnoughToHaveResizeHandles ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tsel not large enough to have resize handles" << endl;
    #endif
        return;
    }

    kpViewManager *vm = viewManager ();
    if (!vm || !vm->selectionBorderVisible () || !vm->selectionBorderFinished ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tsel border not visible or not finished" << endl;
    #endif

        return;
    }

    const QRect selViewRect = selectionViewRect ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\tselViewRect=" << selViewRect << endl;
#endif
    if (!selViewRect.intersects (viewRect))
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tdoesn't intersect viewRect" << endl;
    #endif
        return;
    }

    QRegion selResizeHandlesRegion = selectionResizeHandlesViewRegion (true/*for renderer*/);
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\tsel resize handles view region="
               << selResizeHandlesRegion << endl;
#endif
    selResizeHandlesRegion.translate (-viewRect.x (), -viewRect.y ());

    painter->save ();

    QColor fillColor;
    if (selectionResizeHandleAtomicSizeCloseToZoomLevel ())
    {
        fillColor = Qt::blue;
        painter->setRasterOp (Qt::CopyROP);
    }
    else
    {
        fillColor = Qt::white;
        painter->setRasterOp (Qt::XorROP);
    }

    QMemArray <QRect> rects = selResizeHandlesRegion.rects ();
    for (QMemArray <QRect>::ConstIterator it = rects.begin ();
         it != rects.end ();
         it++)
    {
        painter->fillRect (*it, fillColor);
    }

    painter->restore ();
}

// protected
void kpView::paintEventDrawTempPixmap (QPixmap *destPixmap, const QRect &docRect)
{
    kpViewManager *vm = viewManager ();
    if (!vm)
        return;

    const kpTempPixmap *tpm = vm->tempPixmap ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "kpView::paintEventDrawTempPixmap() tempPixmap="
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
    int tileHeight = 16 * vzoomMultiple;  // CONFIG
    for (int y = starty - viewRect.y (); y <= viewRect.bottom () - viewRect.y (); y += vzoomMultiple)
    {
        if (0 && tileHeight > 0 && y % tileHeight == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }

        painter->drawLine (0, y, viewRect.right () - viewRect.left (), y);

        if (0 && tileHeight > 0 && y % tileHeight == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    }

    // vertical lines
    int startx = viewRect.left ();
    if (startx % hzoomMultiple)
        startx = (startx + hzoomMultiple) / hzoomMultiple * hzoomMultiple;
    int tileWidth = 16 * hzoomMultiple;  // CONFIG
    for (int x = startx - viewRect.x (); x <= viewRect.right () - viewRect.x (); x += hzoomMultiple)
    {
        if (0 && tileWidth > 0 && x % tileWidth == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }

        painter->drawLine (x, 0, x, viewRect.bottom () - viewRect.top ());

        if (0 && tileWidth > 0 && x % tileWidth == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    }
}


void kpView::paintEventDrawRect (const QRect &viewRect)
{
#if DEBUG_KP_VIEW_RENDERER
    kdDebug () << "\tkpView::paintEventDrawRect(viewRect=" << viewRect
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
    kdDebug () << "\tdocRect=" << docRect << endl;
#endif

// uncomment to cause deliberate flicker (identifies needless updates)
#if DEBUG_KP_VIEW_RENDERER && 0
    QPainter flickerPainter (this);
    flickerPainter.fillRect (viewRect, Qt::red);
    flickerPainter.end ();
#endif


    //
    // Prepare Back Buffer
    //

    if (!d->m_backBuffer ||
        d->m_backBuffer->width () < viewRect.width () ||
        d->m_backBuffer->height () < viewRect.height () ||
        d->m_backBuffer->width () > width () ||
        d->m_backBuffer->height () > height ())
    {
        // don't use QPixmap::resize() as that wastes time copying pixels
        // that will be overwritten anyway
        //
        // OPT: Should use doubling trick or at least go up in multiples
        //      to reduce X server pressure.
        delete d->m_backBuffer;
        d->m_backBuffer = new QPixmap (viewRect.width (), viewRect.height ());
    }

// uncomment to catch bits of the view that the renderer forgot to update
#if 0
    d->m_backBuffer->fill (Qt::green);
#endif

    QPainter backBufferPainter;
    backBufferPainter.begin (d->m_backBuffer);


    //
    // Draw checkboard for transparent images and/or views with borders
    //

    QPixmap docPixmap;

    bool tempPixmapWillBeRendered = false;

    if (!docRect.isEmpty ())
    {
        docPixmap = doc->getPixmapAt (docRect);

        tempPixmapWillBeRendered =
            (!doc->selection () &&
             vm->tempPixmap () &&
             vm->tempPixmap ()->isVisible (vm) &&
             docRect.intersects (vm->tempPixmap ()->rect ()));

    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\ttempPixmapWillBeRendered=" << tempPixmapWillBeRendered
                   << " (sel=" << doc->selection ()
                   << " tempPixmap=" << vm->tempPixmap ()
                   << " tempPixmap.isVisible=" << (vm->tempPixmap () ? vm->tempPixmap ()->isVisible (vm) : false)
                   << " docRect.intersects(tempPixmap.rect)=" << (vm->tempPixmap () ? docRect.intersects (vm->tempPixmap ()->rect ()) : false)
                   << ")"
                   << endl;
    #endif
    }

    if (docPixmap.mask () ||
        (tempPixmapWillBeRendered && vm->tempPixmap ()->mayChangeDocumentMask ()))
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tmask=" << (bool) docPixmap.mask ()
                   << endl;
    #endif
        paintEventDrawCheckerBoard (&backBufferPainter, viewRect);
    }
    else
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tno mask" << endl;
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
        kdDebug () << "\torigin=" << origin () << endl;
    #endif
        // blit scaled version of docPixmap + tempPixmap onto Back Buffer
    #if DEBUG_KP_VIEW_RENDERER && 1
        QTime scaleTimer; scaleTimer.start ();
    #endif
        backBufferPainter.translate (origin ().x () - viewRect.x (),
                                     origin ().y () - viewRect.y ());
        backBufferPainter.scale (double (zoomLevelX ()) / 100.0,
                                 double (zoomLevelY ()) / 100.0);
        backBufferPainter.drawPixmap (docRect, docPixmap);
        backBufferPainter.resetXForm ();  // back to 1-1 scaling
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tscale time=" << scaleTimer.elapsed () << endl;
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
        paintEventDrawGridLines (&backBufferPainter, viewRect);
    #if DEBUG_KP_VIEW_RENDERER && 1
        kdDebug () << "\tgrid time=" << gridTimer.elapsed () << endl;
    #endif
    }


    const QRect bvsvRect = buddyViewScrollableContainerRectangle ();
    if (!bvsvRect.isEmpty ())
    {
        backBufferPainter.save ();

        backBufferPainter.setRasterOp (Qt::XorROP);
        backBufferPainter.setPen (Qt::white);
        backBufferPainter.translate (-viewRect.x (), -viewRect.y ());
        backBufferPainter.drawRect (bvsvRect);

        backBufferPainter.restore ();
    }


    if (!docRect.isEmpty ())
    {
        if (doc->selection ())
        {
            // Draw resize handles on top of possible grid lines
            paintEventDrawSelectionResizeHandles (&backBufferPainter, viewRect);
        }
    }


    //
    // Blit Back Buffer to View
    //

    backBufferPainter.end ();

    bitBlt (this, viewRect.topLeft (),
            d->m_backBuffer, QRect (0, 0, viewRect.width (), viewRect.height ()));
}


// protected virtual [base QWidget]
void kpView::paintEvent (QPaintEvent *e)
{
    // sync: kpViewPrivate
    // WARNING: document(), viewManager() and friends might be 0 in this method.
    // TODO: I'm not 100% convinced that we always check if their friends are 0.

#if DEBUG_KP_VIEW_RENDERER && 1
    QTime timer;
    timer.start ();
#endif

    kpViewManager *vm = viewManager ();

#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "kpView(" << name () << ")::paintEvent() vm=" << (bool) vm
               << " queueUpdates=" << (vm && vm->queueUpdates ())
               << " fastUpdates=" << (vm && vm->fastUpdates ())
               << " viewRect=" << e->rect ()
               << " erased=" << e->erased ()
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


    QRegion viewRegion = clipRegion ().intersect (e->region ());
    QMemArray <QRect> rects = viewRegion.rects ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\t#rects = " << rects.count () << endl;
#endif

    for (QMemArray <QRect>::ConstIterator it = rects.begin ();
         it != rects.end ();
         it++)
    {
        paintEventDrawRect (*it);
    }


#if DEBUG_KP_VIEW_RENDERER && 1
    kdDebug () << "\tall done in: " << timer.restart () << "ms" << endl;
#endif
}


#include <kpview.moc>
