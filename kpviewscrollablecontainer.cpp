
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

#define DEBUG_KP_VIEW_SCROLLABLE_CONTAINER 0

#include <kpviewscrollablecontainer.h>

#include <qcursor.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kppixmapfx.h>
#include <kpview.h>
#include <kpwidgetmapper.h>


// (Pulled from out of Thurston's hat)
static const int DragScrollLeftTopMargin = 0;
static const int DragScrollRightBottomMargin = 16;  // scrollbarish
static const int DragScrollInterval = 1000 / 10;
static const int DragScrollInitialInterval = DragScrollInterval * 2;
static const int DragScrollNumPixels = 5;
static const int DragDistanceFromRectMaxFor1stMultiplier = 50;
static const int DragDistanceFromRectMaxFor2ndMultiplier = 100;

static const int GripSize = 7;
static const int GripHandleSize = 7;


kpGrip::kpGrip (GripType type,
                QWidget *parent, const char *name)
    : QWidget (parent, name),
      m_type (type),
      m_startPoint (KP_INVALID_POINT),
      m_currentPoint (KP_INVALID_POINT),
      m_shouldReleaseMouseButtons (false)
{
    setCursor (cursorForType (m_type));

    setMouseTracking (true);  // mouseMoveEvent's even when no mousebtn down

    updatePixmap ();
}

kpGrip::~kpGrip ()
{
}


// public
kpGrip::GripType kpGrip::type () const
{
    return m_type;
}


// public static
const QCursor &kpGrip::cursorForType (GripType type)
{
    switch (type)
    {
    case Bottom:
        return Qt::sizeVerCursor;
        break;  // one day you'll forget

    case Right:
        return Qt::sizeHorCursor;
        break;  // one day you'll forget

    case BottomRight:
        return Qt::sizeFDiagCursor;
        break;  // one day you'll forget
    }

    return Qt::arrowCursor;
}


// public
QRect kpGrip::hotRect (bool toGlobal) const
{
    QRect ret;

    switch (m_type)
    {
    case Bottom:
    {
        const int handleX = (width () - GripHandleSize) / 2;
        ret = QRect (handleX, 0,
                     GripHandleSize, height ());
        break;
    }
    case Right:
    {
        const int handleY = (height () - GripHandleSize) / 2;
        ret = QRect (0, handleY,
                     width (), GripHandleSize);
        break;
    }
    case BottomRight:
        // pixmap all opaque
        ret = rect ();
        break;

    default:
        return QRect ();
    }

    return (toGlobal ? QRect (mapToGlobal (ret.topLeft ()),
                              mapToGlobal (ret.bottomRight ()))
                     : ret);
}


// public
bool kpGrip::isDrawing () const
{
    return (m_startPoint != KP_INVALID_POINT);
}


// public
QString kpGrip::haventBegunDrawUserMessage () const
{
    return i18n ("Left drag the handle to resize the image.");
}


// public
QString kpGrip::userMessage () const
{
    return m_userMessage;
}

// public
void kpGrip::setUserMessage (const QString &message)
{
    // Don't do NOP checking here since another grip might have changed
    // the message so an apparent NOP for this grip is not a NOP in the
    // global sense (kpViewScrollableContainer::slotGripStatusMessageChanged()).

    m_userMessage = message;
    emit statusMessageChanged (message);
}


// protected
void kpGrip::updatePixmap ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::updatePixmap() rect=" << rect () << endl;
#endif
    if (width () <= 0 || height () <= 0)
        return;

    QPixmap pixmap (width (), height ());
    pixmap.fill (colorGroup ().highlight ());
    kpPixmapFX::ensureTransparentAt (&pixmap, pixmap.rect ());
    const QRect hr = hotRect ();
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "\thotRect=" << hr << endl;
#endif
    if (hr.isValid ())
        kpPixmapFX::ensureOpaqueAt (&pixmap, hr);

    setBackgroundPixmap (pixmap);
    if (pixmap.mask ())
        setMask (*pixmap.mask ());
}


// protected
void kpGrip::cancel ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::cancel()" << endl;
#endif
    if (m_currentPoint == KP_INVALID_POINT)
        return;

    m_startPoint = KP_INVALID_POINT;
    m_currentPoint = KP_INVALID_POINT;

    setUserMessage (i18n ("Resize Image: Let go of all the mouse buttons."));
    setCursor (Qt::arrowCursor);
    m_shouldReleaseMouseButtons = true;

    releaseKeyboard ();
    emit cancelledDraw ();
}


// protected virtual [base QWidget]
void kpGrip::keyReleaseEvent (QKeyEvent *e)
{
    if (m_startPoint != KP_INVALID_POINT &&
        e->key () == Qt::Key_Escape)
    {
        cancel ();
    }
}

// protected virtual [base QWidget]
void kpGrip::mousePressEvent (QMouseEvent *e)
{
    if (m_startPoint == KP_INVALID_POINT &&
        (e->stateAfter () & Qt::MouseButtonMask) == Qt::LeftButton)
    {
        m_startPoint = e->pos ();
        m_currentPoint = e->pos ();
        emit beganDraw ();
        grabKeyboard ();

        setUserMessage (i18n ("Resize Image: Right click to cancel."));
        setCursor (cursorForType (m_type));
    }
    else
    {
        if (m_startPoint != KP_INVALID_POINT)
            cancel ();
    }
}

// public
QPoint kpGrip::viewDeltaPoint () const
{
    if (m_startPoint == KP_INVALID_POINT)
        return KP_INVALID_POINT;

    const QPoint point = mapFromGlobal (QCursor::pos ());

    // TODO: this is getting out of sync with m_currentPoint

    return QPoint (((m_type & Right) ? point.x () - m_startPoint.x () : 0),
                   ((m_type & Bottom) ? point.y () - m_startPoint.y () : 0));

}

// public
void kpGrip::mouseMovedTo (const QPoint &point, bool dueToDragScroll)
{
    if (m_startPoint == KP_INVALID_POINT)
        return;

    m_currentPoint = point;

    emit continuedDraw (((m_type & Right) ? point.x () - m_startPoint.x () : 0),
                        ((m_type & Bottom) ? point.y () - m_startPoint.y () : 0),
                        dueToDragScroll);
}

// protected virtual [base QWidget]
void kpGrip::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::mouseMoveEvent() m_startPoint=" << m_startPoint
               << " stateAfter=" << e->stateAfter ()
               << endl;
#endif

    if (m_startPoint == KP_INVALID_POINT)
    {
        if ((e->stateAfter () & Qt::MouseButtonMask) == 0)
            setUserMessage (haventBegunDrawUserMessage ());
        return;
    }

    mouseMovedTo (e->pos (), false/*not due to drag scroll*/);
}

// protected virtual [base QWidget]
void kpGrip::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::mouseReleaseEvent() m_startPoint=" << m_startPoint
               << " stateAfter=" << e->stateAfter ()
               << endl;
#endif

    if (m_startPoint != KP_INVALID_POINT)
    {
        const int dx = m_currentPoint.x () - m_startPoint.x (),
                  dy = m_currentPoint.y () - m_startPoint.y ();

        m_currentPoint = KP_INVALID_POINT;
        m_startPoint = KP_INVALID_POINT;

        releaseKeyboard ();
        emit endedDraw ((m_type & Right) ? dx : 0,
                        (m_type & Bottom) ? dy : 0);
    }

    if ((e->stateAfter () & Qt::MouseButtonMask) == 0)
    {
        m_shouldReleaseMouseButtons = false;
        setUserMessage (QString::null);
        setCursor (cursorForType (m_type));

        releaseKeyboard ();
        emit releasedAllButtons ();
    }
}


// protected virtual [base QWidget]
void kpGrip::resizeEvent (QResizeEvent *)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::resizeEvent()" << endl;
#endif
    updatePixmap ();
}


// protected virtual [base QWidget]
void kpGrip::enterEvent (QEvent * /*e*/)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::enterEvent()"
               << " m_startPoint=" << m_startPoint
               << " shouldReleaseMouseButtons="
               << m_shouldReleaseMouseButtons << endl;
#endif

    if (m_startPoint == KP_INVALID_POINT &&
        !m_shouldReleaseMouseButtons)
    {
    #if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
        kdDebug () << "\tsending message" << endl;
    #endif
        setUserMessage (haventBegunDrawUserMessage ());
    }
}

// protected virtual [base QWidget]
void kpGrip::leaveEvent (QEvent * /*e*/)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpGrip::leaveEvent()"
               << " m_startPoint=" << m_startPoint
               << " shouldReleaseMouseButtons="
               << m_shouldReleaseMouseButtons << endl;
#endif
    if (m_startPoint == KP_INVALID_POINT &&
        !m_shouldReleaseMouseButtons)
    {
        setUserMessage (QString::null);
    }
}


// protected virtual [base QWidget]
void kpGrip::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 0
    kdDebug () << "kpGrip::paintEvent(" << e->rect () << ")" << endl;
#endif
    QWidget::paintEvent (e);
}


// TODO: Are we checking for m_view == 0 often enough?
kpViewScrollableContainer::kpViewScrollableContainer (kpMainWindow *parent,
                                                      const char *name)
    : QScrollView ((QWidget *) parent, name, Qt::WStaticContents | Qt::WNoAutoErase),
      m_mainWindow (parent),
      m_contentsXSoon (-1), m_contentsYSoon (-1),
      m_view (0),
      m_bottomGrip (new kpGrip (kpGrip::Bottom, viewport (), "Bottom Grip")),
      m_rightGrip (new kpGrip (kpGrip::Right, viewport (), "Right Grip")),
      m_bottomRightGrip (new kpGrip (kpGrip::BottomRight, viewport (), "BottomRight Grip")),
      m_docResizingGrip (0),
      m_dragScrollTimer (new QTimer (this)),
      m_zoomLevel (100),
      m_scrollTimerRunOnce (false),
      m_resizeRoundedLastViewX (-1), m_resizeRoundedLastViewY (-1),
      m_resizeRoundedLastViewDX (0), m_resizeRoundedLastViewDY (0),
      m_haveMovedFromOriginalDocSize (false)

{
    m_bottomGrip->setFixedHeight (GripSize);
    m_bottomGrip->hide ();
    addChild (m_bottomGrip);
    connectGripSignals (m_bottomGrip);

    m_rightGrip->setFixedWidth (GripSize);
    m_rightGrip->hide ();
    addChild (m_rightGrip);
    connectGripSignals (m_rightGrip);

    m_bottomRightGrip->setFixedSize (GripSize, GripSize);
    m_bottomRightGrip->hide ();
    addChild (m_bottomRightGrip);
    connectGripSignals (m_bottomRightGrip);


    connect (this, SIGNAL (contentsMoving (int, int)),
             this, SLOT (slotContentsMoving (int, int)));

    connect (m_dragScrollTimer, SIGNAL (timeout ()),
             this, SLOT (slotDragScroll ()));
}

kpViewScrollableContainer::~kpViewScrollableContainer ()
{
}


// public
int kpViewScrollableContainer::contentsXSoon ()
{
    if (m_contentsXSoon < 0)
        return contentsX ();
    else
        return m_contentsXSoon;
}

// public
int kpViewScrollableContainer::contentsYSoon ()
{
    if (m_contentsYSoon < 0)
        return contentsY ();
    else
        return m_contentsYSoon;
}


// protected
void kpViewScrollableContainer::connectGripSignals (kpGrip *grip)
{
    connect (grip, SIGNAL (beganDraw ()),
             this, SLOT (slotGripBeganDraw ()));
    connect (grip, SIGNAL (continuedDraw (int, int, bool)),
             this, SLOT (slotGripContinuedDraw (int, int, bool)));
    connect (grip, SIGNAL (cancelledDraw ()),
             this, SLOT (slotGripCancelledDraw ()));
    connect (grip, SIGNAL (endedDraw (int, int)),
             this, SLOT (slotGripEndedDraw (int, int)));

    connect (grip, SIGNAL (statusMessageChanged (const QString &)),
             this, SLOT (slotGripStatusMessageChanged (const QString &)));

    connect (grip, SIGNAL (releasedAllButtons ()),
             this, SLOT (recalculateStatusMessage ()));
}


// public
QSize kpViewScrollableContainer::newDocSize () const
{
    return newDocSize (m_resizeRoundedLastViewDX,
                       m_resizeRoundedLastViewDY);
}

// public
bool kpViewScrollableContainer::haveMovedFromOriginalDocSize () const
{
    return m_haveMovedFromOriginalDocSize;
}

// public
QString kpViewScrollableContainer::statusMessage () const
{
    return m_gripStatusMessage;
}

// public
void kpViewScrollableContainer::clearStatusMessage ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 1
    kdDebug () << "kpViewScrollableContainer::clearStatusMessage()" << endl;
#endif
    m_bottomRightGrip->setUserMessage (QString::null);
    m_bottomGrip->setUserMessage (QString::null);
    m_rightGrip->setUserMessage (QString::null);
}


// protected
QSize kpViewScrollableContainer::newDocSize (int viewDX, int viewDY) const
{
    if (!m_view)
        return QSize ();

    if (!docResizingGrip ())
        return QSize ();

    const int docX = (int) m_view->transformViewToDocX (m_view->width () + viewDX);
    const int docY = (int) m_view->transformViewToDocY (m_view->height () + viewDY);

    return QSize (QMAX (1, docX), QMAX (1, docY));
}


// protected
void kpViewScrollableContainer::calculateDocResizingGrip ()
{
    if (m_bottomRightGrip->isDrawing ())
        m_docResizingGrip = m_bottomRightGrip;
    else if (m_bottomGrip->isDrawing ())
        m_docResizingGrip = m_bottomGrip;
    else if (m_rightGrip->isDrawing ())
        m_docResizingGrip = m_rightGrip;
    else
        m_docResizingGrip = 0;
}

// protected
kpGrip *kpViewScrollableContainer::docResizingGrip () const
{
    return m_docResizingGrip;
}


// protected
int kpViewScrollableContainer::bottomResizeLineWidth () const
{
    if (!docResizingGrip ())
        return -1;

    if (!m_view)
        return -1;

    if (docResizingGrip ()->type () & kpGrip::Bottom)
        return QMAX (m_view->zoomLevelY () / 100, 1);
    else
        return 1;
}

// protected
int kpViewScrollableContainer::rightResizeLineWidth () const
{
    if (!docResizingGrip ())
        return -1;

    if (!m_view)
        return -1;

    if (docResizingGrip ()->type () & kpGrip::Right)
        return QMAX (m_view->zoomLevelX () / 100, 1);
    else
        return 1;
}


// protected
QRect kpViewScrollableContainer::bottomResizeLineRect () const
{
    if (m_resizeRoundedLastViewX < 0 || m_resizeRoundedLastViewY < 0)
        return QRect ();

    return QRect (QPoint (0,
                          m_resizeRoundedLastViewY),
                  QPoint (m_resizeRoundedLastViewX - 1,
                          m_resizeRoundedLastViewY + bottomResizeLineWidth () - 1));
}

// protected
QRect kpViewScrollableContainer::rightResizeLineRect () const
{
    if (m_resizeRoundedLastViewX < 0 || m_resizeRoundedLastViewY < 0)
        return QRect ();

    return QRect (QPoint (m_resizeRoundedLastViewX,
                          0),
                  QPoint (m_resizeRoundedLastViewX + rightResizeLineWidth () - 1,
                          m_resizeRoundedLastViewY - 1));
}

// protected
QRect kpViewScrollableContainer::bottomRightResizeLineRect () const
{
    if (m_resizeRoundedLastViewX < 0 || m_resizeRoundedLastViewY < 0)
        return QRect ();

    return QRect (QPoint (m_resizeRoundedLastViewX,
                          m_resizeRoundedLastViewY),
                  QPoint (m_resizeRoundedLastViewX + rightResizeLineWidth () - 1,
                          m_resizeRoundedLastViewY + bottomResizeLineWidth () - 1));
}


// TODO: are these 2 correct?  Remember that viewport()->x() == 1, viewport()->y() == 1

// protected
QPoint kpViewScrollableContainer::mapViewToViewport (const QPoint &viewPoint)
{
    return viewPoint - QPoint (contentsX (), contentsY ());
}

// protected
QRect kpViewScrollableContainer::mapViewToViewport (const QRect &viewRect)
{
    if (!viewRect.isValid ())
        return QRect ();

    QRect ret = viewRect;
    ret.moveBy (-contentsX (), -contentsY ());
    return ret;
}


// protected
QRect kpViewScrollableContainer::mapViewportToGlobal (const QRect &viewportRect)
{
    return kpWidgetMapper::toGlobal (viewport (), viewportRect);
}

// protected
QRect kpViewScrollableContainer::mapViewToGlobal (const QRect &viewRect)
{
    return mapViewportToGlobal (mapViewToViewport (viewRect));
}


// protected
void kpViewScrollableContainer::repaintWidgetAtResizeLineViewRect (
    QWidget *widget, const QRect &resizeLineViewRect)
{
    const QRect resizeLineGlobalRect = mapViewToGlobal (resizeLineViewRect);
    const QRect widgetGlobalRect = kpWidgetMapper::toGlobal (widget,
                                                             widget->rect ());

    const QRect redrawGlobalRect =
        resizeLineGlobalRect.intersect (widgetGlobalRect);

    const QRect redrawWidgetRect =
        kpWidgetMapper::fromGlobal (widget, redrawGlobalRect);


    if (redrawWidgetRect.isValid ())
    {
        // TODO: should be "!widget->testWFlags (Qt::WRepaintNoErase)"
        //       but for some reason, doesn't work for viewport().
        const bool erase = !dynamic_cast <kpView *> (widget);
        widget->repaint (redrawWidgetRect, erase);
    }
}

// protected
void kpViewScrollableContainer::repaintWidgetAtResizeLines (QWidget *widget)
{
    repaintWidgetAtResizeLineViewRect (widget, rightResizeLineRect ());
    repaintWidgetAtResizeLineViewRect (widget, bottomResizeLineRect ());
    repaintWidgetAtResizeLineViewRect (widget, bottomRightResizeLineRect ());
}

// protected
void kpViewScrollableContainer::eraseResizeLines ()
{
    if (m_resizeRoundedLastViewX >= 0 && m_resizeRoundedLastViewY >= 0)
    {
        repaintWidgetAtResizeLines (viewport ());
        repaintWidgetAtResizeLines (m_view);

        repaintWidgetAtResizeLines (m_bottomGrip);
        repaintWidgetAtResizeLines (m_rightGrip);
        repaintWidgetAtResizeLines (m_bottomRightGrip);
    }
}


// protected
void kpViewScrollableContainer::drawResizeLines ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 0
    kdDebug () << "kpViewScrollableContainer::drawResizeLines()"
               << " lastViewX=" << m_resizeRoundedLastViewX
               << " lastViewY=" << m_resizeRoundedLastViewY
               << endl;
#endif


    QPainter p (viewport (), true/*unclipped*/);
    p.setRasterOp (Qt::NotROP);

    const QRect rightRect = rightResizeLineRect ();
    if (rightRect.isValid ())
        p.fillRect (mapViewToViewport (rightRect), Qt::white);

    const QRect bottomRect = bottomResizeLineRect ();
    if (bottomRect.isValid ())
        p.fillRect (mapViewToViewport (bottomRect), Qt::white);

    const QRect bottomRightRect = bottomRightResizeLineRect ();
    if (bottomRightRect.isValid ())
        p.fillRect (mapViewToViewport (bottomRightRect), Qt::white);

    p.end ();
}


// protected
void kpViewScrollableContainer::updateResizeLines (int viewX, int viewY,
                                                   int viewDX, int viewDY)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 0
    kdDebug () << "kpViewScrollableContainer::updateResizeLines("
               << viewX << "," << viewY << ")"
               << " oldViewX=" << m_resizeRoundedLastViewX
               << " oldViewY=" << m_resizeRoundedLastViewY
               << " viewDX=" << viewDX
               << " viewDY=" << viewDY
               << endl;
#endif

    eraseResizeLines ();


    if (viewX >= 0 && viewY >= 0)
    {
        m_resizeRoundedLastViewX = (int) m_view->transformDocToViewX ((int) m_view->transformViewToDocX (viewX));
        m_resizeRoundedLastViewY = (int) m_view->transformDocToViewY ((int) m_view->transformViewToDocY (viewY));

        m_resizeRoundedLastViewDX = viewDX;
        m_resizeRoundedLastViewDY = viewDY;
    }
    else
    {
        m_resizeRoundedLastViewX = -1;
        m_resizeRoundedLastViewY = -1;

        m_resizeRoundedLastViewDX = 0;
        m_resizeRoundedLastViewDY = 0;
    }

    // TODO: This is suboptimal since if another window pops up on top of
    //       KolourPaint then disappears, the lines are not redrawn
    //       (although this doesn't happen very frequently since we grab the
    //       keyboard and mouse when resizing):
    //
    //         e.g. sleep 5 && gedit & sleep 10 && killall gedit
    //
    //       Should be done in the paintEvent's of every child of the
    //       scrollview.
    drawResizeLines ();
}


// protected slot
void kpViewScrollableContainer::slotGripBeganDraw ()
{
    if (!m_view)
        return;

    calculateDocResizingGrip ();

    m_haveMovedFromOriginalDocSize = false;

    updateResizeLines (m_view->width (), m_view->height (),
                       0/*viewDX*/, 0/*viewDY*/);

    emit beganDocResize ();
}

// protected slot
void kpViewScrollableContainer::slotGripContinuedDraw (int inViewDX, int inViewDY,
                                                       bool dueToDragScroll)
{
    int viewDX = inViewDX,
        viewDY = inViewDY;

#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::slotGripContinuedDraw("
               << viewDX << "," << viewDY << ") size="
               << newDocSize (viewDX, viewDY)
               << " dueToDragScroll=" << dueToDragScroll
               << endl;
#endif

    if (!m_view)
        return;

    if (!dueToDragScroll &&
        beginDragScroll (QPoint (), QPoint (), m_view->zoomLevelX ()))
    {
        const QPoint newViewDeltaPoint = docResizingGrip ()->viewDeltaPoint ();
        viewDX = newViewDeltaPoint.x ();
        viewDY = newViewDeltaPoint.y ();
    #if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
        kdDebug () << "\tdrag scrolled - new view delta point="
                   << newViewDeltaPoint
                   << endl;
    #endif
    }

    m_haveMovedFromOriginalDocSize = true;

    updateResizeLines (QMAX (1, QMAX (m_view->width () + viewDX, (int) m_view->transformDocToViewX (1))),
                       QMAX (1, QMAX (m_view->height () + viewDY, (int) m_view->transformDocToViewY (1))),
                       viewDX, viewDY);

    emit continuedDocResize (newDocSize ());
}

// protected slot
void kpViewScrollableContainer::slotGripCancelledDraw ()
{
    m_haveMovedFromOriginalDocSize = false;

    updateResizeLines (-1, -1, 0, 0);

    calculateDocResizingGrip ();

    emit cancelledDocResize ();

    endDragScroll ();
}

// protected slot
void kpViewScrollableContainer::slotGripEndedDraw (int viewDX, int viewDY)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::slotGripEndedDraw("
               << viewDX << "," << viewDY << ") size="
               << newDocSize (viewDX, viewDY)
               << endl;
#endif

    if (!m_view)
        return;

    const QSize newSize = newDocSize (viewDX, viewDY);

    m_haveMovedFromOriginalDocSize = false;

    // must erase lines before view size changes
    updateResizeLines (-1, -1, 0, 0);

    calculateDocResizingGrip ();

    emit endedDocResize (newSize);

    endDragScroll ();
}


// protected slot
void kpViewScrollableContainer::slotGripStatusMessageChanged (const QString &string)
{
    if (string == m_gripStatusMessage)
        return;

    m_gripStatusMessage = string;
    emit statusMessageChanged (string);
}


// public slot
void kpViewScrollableContainer::recalculateStatusMessage ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollabelContainer::recalculateStatusMessage()" << endl;
    kdDebug () << "\tQCursor::pos=" << QCursor::pos ()
               << " global visibleRect="
               << kpWidgetMapper::toGlobal (this,
                      QRect (0, 0, visibleWidth (), visibleHeight ()))
               << " brGrip.hotRect=" << m_bottomRightGrip->hotRect (true)
               << " bGrip.hotRect=" << m_bottomGrip->hotRect (true)
               << " rGrip.hotRect=" << m_rightGrip->hotRect (true)
               << endl;
#endif

    // HACK: After dragging to a new size, handles move so that they are now
    //       under the mouse pointer but no mouseMoveEvent() is generated for
    //       any grip.  This also handles the case of cancelling over any
    //       grip.
    //
    if (kpWidgetMapper::toGlobal (this,
                                  QRect (0, 0, visibleWidth (), visibleHeight ()))
            .contains (QCursor::pos ()))
    {
        if (m_bottomRightGrip->isShown () &&
            m_bottomRightGrip->hotRect (true/*to global*/)
                .contains (QCursor::pos ()))
        {
            m_bottomRightGrip->setUserMessage (i18n ("Left drag the handle to resize the image."));
        }
        else if (m_bottomGrip->isShown () &&
                m_bottomGrip->hotRect (true/*to global*/)
                    .contains (QCursor::pos ()))
        {
            m_bottomGrip->setUserMessage (i18n ("Left drag the handle to resize the image."));
        }
        else if (m_rightGrip->isShown () &&
                m_rightGrip->hotRect (true/*to global*/)
                    .contains (QCursor::pos ()))
        {
            m_rightGrip->setUserMessage (i18n ("Left drag the handle to resize the image."));
        }
        else
        {
            clearStatusMessage ();
        }
    }
    else
    {
        clearStatusMessage ();
    }
}


// protected slot
void kpViewScrollableContainer::slotContentsMoving (int x, int y)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::slotContentsMoving("
               << x << "," << y << ")"
               << " contentsX=" << contentsX ()
               << " contentsY=" << contentsY () << endl;
#endif

    m_contentsXSoon = x, m_contentsYSoon = y;
    emit contentsMovingSoon (m_contentsXSoon, m_contentsYSoon);

    // Reduce flicker - don't let QScrollView scroll to-be-erased lines
    eraseResizeLines ();

    QTimer::singleShot (0, this, SLOT (slotContentsMoved ()));
}

// protected slot
void kpViewScrollableContainer::slotContentsMoved ()
{
    m_contentsXSoon = m_contentsYSoon = -1;

    kpGrip *grip = docResizingGrip ();
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::slotContentsMoved()"
               << " grip=" << grip
               << " contentsX=" << contentsX ()
               << " contentsY=" << contentsY () << endl;
#endif
    if (!grip)
        return;

    grip->mouseMovedTo (grip->mapFromGlobal (QCursor::pos ()),
                        true/*moved due to drag scroll*/);
}


// protected
void kpViewScrollableContainer::disconnectViewSignals ()
{
    disconnect (m_view, SIGNAL (sizeChanged (const QSize &)),
                this, SLOT (updateGrips ()));
    disconnect (m_view, SIGNAL (destroyed ()),
                this, SLOT (slotViewDestroyed ()));
}

// protected
void kpViewScrollableContainer::connectViewSignals ()
{
    connect (m_view, SIGNAL (sizeChanged (const QSize &)),
             this, SLOT (updateGrips ()));
    connect (m_view, SIGNAL (destroyed ()),
             this, SLOT (slotViewDestroyed ()));
}


// public virtual [base QScrollView]
void kpViewScrollableContainer::addChild (QWidget *widget, int x, int y)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::addChild(" << widget
               << "," << x << "," << y << endl;
#endif

    QScrollView::addChild (widget, x, y);

    kpView *view = dynamic_cast <kpView *> (widget);
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "\tcast to kpView: " << view << endl;
#endif
    if (view)
    {
        setView (view);
    }
}


// public
kpView *kpViewScrollableContainer::view () const
{
    return m_view;
}

// public
void kpViewScrollableContainer::setView (kpView *view)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::setView(" << view << ")" << endl;
#endif

    if (m_view == view)
        return;

    if (m_view)
    {
        disconnectViewSignals ();
    }

    m_view = view;

    updateGrips ();

    if (m_view)
    {
        connectViewSignals ();
    }
}


// public slot
void kpViewScrollableContainer::updateGrips ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::updateGrips() m_view="
               << m_view << endl;
#endif

    if (m_view)
    {
        m_bottomGrip->setFixedWidth (m_view->width ());
        moveChild (m_bottomGrip, 0, m_view->height ());

        m_rightGrip->setFixedHeight (m_view->height ());
        moveChild (m_rightGrip, m_view->width (), 0);

        moveChild (m_bottomRightGrip, m_view->width (), m_view->height ());
    }

    m_bottomGrip->setShown (bool (m_view));
    m_rightGrip->setShown (bool (m_view));
    m_bottomRightGrip->setShown (bool (m_view));

#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "\tcontentsRect=" << contentsRect ()
               << " visibleRect=" << visibleRect ()
               << " viewportRect=" << viewport ()->rect ()
               << endl;
#endif

    if (m_view)
    {
        resizeContents (m_view->width () + m_rightGrip->width (),
                        m_view->height () + m_bottomGrip->height ());
    }
    else
    {
        resizeContents (0, 0);
    }

    recalculateStatusMessage ();
}

// protected slot
void kpViewScrollableContainer::slotViewDestroyed ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::slotViewDestroyed() m_view="
               << m_view << endl;
#endif

    m_view = 0;
    updateGrips ();
}


// public slot
bool kpViewScrollableContainer::beginDragScroll (const QPoint &/*docPoint*/,
                                                 const QPoint &/*lastDocPoint*/,
                                                 int zoomLevel,
                                                 bool *didSomething)
{
    if (didSomething)
        *didSomething = false;

    m_zoomLevel = zoomLevel;

    const QPoint p = mapFromGlobal (QCursor::pos ());

#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::beginDragScroll() p=" << p
               << " dragScrollTimerRunOnce=" << m_scrollTimerRunOnce
               << endl;
#endif

    bool stopDragScroll = true;
    bool scrolled = false;

    if (!noDragScrollRect ().contains (p))
    {
        if (m_dragScrollTimer->isActive ())
        {
            if (m_scrollTimerRunOnce)
            {
                scrolled = slotDragScroll ();
            }
        }
        else
        {
            m_scrollTimerRunOnce = false;
            m_dragScrollTimer->start (DragScrollInitialInterval);
        }

        stopDragScroll = false;
    }

    if (stopDragScroll)
        m_dragScrollTimer->stop ();

    if (didSomething)
        *didSomething = scrolled;

    return scrolled;
}

// public slot
bool kpViewScrollableContainer::beginDragScroll (const QPoint &docPoint,
                                                 const QPoint &lastDocPoint,
                                                 int zoomLevel)
{
    return beginDragScroll (docPoint, lastDocPoint, zoomLevel,
                            0/*don't want scrolled notification*/);
}


// public slot
bool kpViewScrollableContainer::endDragScroll ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::endDragScroll()" << endl;
#endif

    if (m_dragScrollTimer->isActive ())
    {
        m_dragScrollTimer->stop ();
        return true;
    }
    else
    {
        return false;
    }
}


static const int distanceFromRectToMultiplier (int dist)
{
    if (dist < 0)
        return 0;
    else if (dist < DragDistanceFromRectMaxFor1stMultiplier)
        return 1;
    else if (dist < DragDistanceFromRectMaxFor2ndMultiplier)
        return 2;
    else
        return 4;
}


// protected slot
bool kpViewScrollableContainer::slotDragScroll (bool *didSomething)
{
    bool scrolled = false;

    if (didSomething)
        *didSomething = false;


    const QRect rect = noDragScrollRect ();
    const QPoint pos = mapFromGlobal (QCursor::pos ());

#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::slotDragScroll()"
                  << " noDragScrollRect=" << rect
                  << " pos=" << pos
                  << " contentsX=" << contentsX ()
                  << " contentsY=" << contentsY () << endl;
#endif

    int dx = 0, dy = 0;
    int dxMultiplier = 0, dyMultiplier = 0;

    if (pos.x () < rect.left ())
    {
        dx = -DragScrollNumPixels;
        dxMultiplier = distanceFromRectToMultiplier (rect.left () - pos.x ());
    }
    else if (pos.x () > rect.right ())
    {
        dx = +DragScrollNumPixels;
        dxMultiplier = distanceFromRectToMultiplier (pos.x () - rect.right ());
    }

    if (pos.y () < rect.top ())
    {
        dy = -DragScrollNumPixels;
        dyMultiplier = distanceFromRectToMultiplier (rect.top () - pos.y ());
    }
    else if (pos.y () > rect.bottom ())
    {
        dy = +DragScrollNumPixels;
        dyMultiplier = distanceFromRectToMultiplier (pos.y () - rect.bottom ());
    }

#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 0
    kdDebug () << "kpViewScrollableContainer::slotDragScroll()"
                  << " dx=" << dx << " * " << dxMultiplier
                  << " dy=" << dy << " * " << dyMultiplier
                  << " zoomLevel=" << m_zoomLevel
                  << endl;
#endif

    dx *= dxMultiplier;// * QMAX (1, m_zoomLevel / 100);
    dy *= dyMultiplier;// * QMAX (1, m_zoomLevel / 100);

    if (dx || dy)
    {
        const int oldContentsX = contentsX (),
                  oldContentsY = contentsY ();

        scrollBy (dx, dy);

    #if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 1
        kdDebug () << "\tafter scrollBy():"
                   << " contentsX=" << contentsX ()
                   << " contentsY=" << contentsY () << endl;
    #endif

        scrolled = (oldContentsX != contentsX () ||
                    oldContentsY != contentsY ());

        if (scrolled)
        {
            QRegion region = QRect (contentsX (), contentsY (),
                                    visibleWidth (), visibleHeight ());
            region -= QRect (oldContentsX, oldContentsY,
                             visibleWidth (), visibleHeight ());

            // Repaint newly exposed region immediately to reduce tearing
            // of scrollView.
            m_view->repaint (region, false/*no erase*/);
        }
    }


    m_dragScrollTimer->changeInterval (DragScrollInterval);
    m_scrollTimerRunOnce = true;


    if (didSomething)
        *didSomething = scrolled;

    return scrolled;
}

// protected virtual [base QScrollView]
void kpViewScrollableContainer::contentsDragMoveEvent (QDragMoveEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::contentsDragMoveEvent"
               << e->pos ()
               << endl;
#endif

    QScrollView::contentsDragMoveEvent (e);
}

// protected slot
bool kpViewScrollableContainer::slotDragScroll ()
{
    return slotDragScroll (0/*don't want scrolled notification*/);
}


// protected virtual [base QScrollView]
void kpViewScrollableContainer::contentsMouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::contentsMouseMoveEvent"
               << e->pos ()
               << endl;
#endif

    QScrollView::contentsMouseMoveEvent (e);
}

// protected virtual [base QScrollView]
void kpViewScrollableContainer::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::mouseMoveEvent"
               << e->pos ()
               << endl;
#endif

    QScrollView::mouseMoveEvent (e);
}


// protected virtual [base QScrollView]
void kpViewScrollableContainer::contentsWheelEvent (QWheelEvent *e)
{
    e->ignore ();
    
    if (m_view)
        m_view->wheelEvent (e);
        
    if (!e->isAccepted ())
        QScrollView::contentsWheelEvent (e);
}


QRect kpViewScrollableContainer::noDragScrollRect () const
{
    return QRect (DragScrollLeftTopMargin, DragScrollLeftTopMargin,
                  width () - DragScrollLeftTopMargin - DragScrollRightBottomMargin,
                  height () - DragScrollLeftTopMargin - DragScrollRightBottomMargin);
}

// protected virtual [base QScrollView]
bool kpViewScrollableContainer::eventFilter (QObject *watchedObject, QEvent *event)
{
    return QScrollView::eventFilter (watchedObject, event);
}

// protected virtual [base QScrollView]
void kpViewScrollableContainer::viewportPaintEvent (QPaintEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kdDebug () << "kpViewScrollableContainer::viewportPaintEvent("
               << e->rect ()
               << ")" << endl;
#endif

    QScrollView::viewportPaintEvent (e);
}

// protected virtual [base QFrame]
void kpViewScrollableContainer::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 0
    kdDebug () << "kpViewScrollableContainer::paintEvent("
               << e->rect ()
               << ")" << endl;
#endif

    QScrollView::paintEvent (e);
}

// protected virtual [base QScrollView]
void kpViewScrollableContainer::resizeEvent (QResizeEvent *e)
{
    QScrollView::resizeEvent (e);

    emit resized ();
}


#include <kpviewscrollablecontainer.moc>
