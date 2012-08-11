/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2011      Martin Koller <kollix@aon.at>
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

#include <kpViewScrollableContainer.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <QScrollBar>

#include <kdebug.h>
#include <klocale.h>

#include <kpDefs.h>
#include <kpPixmapFX.h>
#include <kpView.h>
#include <kpWidgetMapper.h>

//---------------------------------------------------------------------

// (Pulled from out of Thurston's hat)
static const int DragScrollLeftTopMargin = 0;
static const int DragScrollRightBottomMargin = 16;  // scrollbarish
static const int DragScrollInterval = 150;
static const int DragScrollInitialInterval = DragScrollInterval * 2;
static const int DragScrollNumPixels = 10;
static const int DragDistanceFromRectMaxFor1stMultiplier = 50;
static const int DragDistanceFromRectMaxFor2ndMultiplier = 100;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
// a transparent widget above all others in the viewport used only while resizing the document
// to be able to show the resize lines above everything else

class kpOverlay : public QWidget
{
  public:
    kpOverlay(QWidget *parent, kpViewScrollableContainer *container)
      : QWidget(parent), m_container(container)
    {
    }

    virtual void paintEvent(QPaintEvent *)
    {
      m_container->drawResizeLines();
    }

  private:
    kpViewScrollableContainer *m_container;

};

//---------------------------------------------------------------------

const int kpGrip::Size = 5;

//---------------------------------------------------------------------

kpGrip::kpGrip (GripType type, QWidget *parent)
    : QWidget(parent),
      m_type (type),
      m_startPoint (KP_INVALID_POINT),
      m_currentPoint (KP_INVALID_POINT),
      m_shouldReleaseMouseButtons (false)
{
    setCursor(cursorForType(m_type));

    setMouseTracking(true);  // mouseMoveEvent's even when no mousebtn down

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Highlight);

    setFixedSize(kpGrip::Size, kpGrip::Size);
}

//---------------------------------------------------------------------

// public
kpGrip::GripType kpGrip::type () const
{
    return m_type;
}

//---------------------------------------------------------------------

// public static
QCursor kpGrip::cursorForType (GripType type)
{
    switch (type)
    {
    case kpGrip::Bottom:
        return Qt::SizeVerCursor;
        break;  // one day you'll forget

    case kpGrip::Right:
        return Qt::SizeHorCursor;
        break;  // one day you'll forget

    case kpGrip::BottomRight:
        return Qt::SizeFDiagCursor;
        break;  // one day you'll forget
    }

    return Qt::ArrowCursor;
}

//---------------------------------------------------------------------

// public
bool kpGrip::containsCursor()
{
    return isVisible() &&
           QRect(mapToGlobal(rect().topLeft()),
                 mapToGlobal(rect().bottomRight())).contains(QCursor::pos());
}

//---------------------------------------------------------------------

// public
bool kpGrip::isDrawing () const
{
    return (m_startPoint != KP_INVALID_POINT);
}

//---------------------------------------------------------------------

// public
QString kpGrip::haventBegunDrawUserMessage () const
{
    return i18n ("Left drag the handle to resize the image.");
}

//---------------------------------------------------------------------

// public
QString kpGrip::userMessage () const
{
    return m_userMessage;
}

//---------------------------------------------------------------------

// public
void kpGrip::setUserMessage (const QString &message)
{
    // Don't do NOP checking here since another grip might have changed
    // the message so an apparent NOP for this grip is not a NOP in the
    // global sense (kpViewScrollableContainer::slotGripStatusMessageChanged()).

    m_userMessage = message;
    emit statusMessageChanged (message);
}

//---------------------------------------------------------------------

// protected
void kpGrip::cancel ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpGrip::cancel()";
#endif
    if (m_currentPoint == KP_INVALID_POINT)
        return;

    m_startPoint = KP_INVALID_POINT;
    m_currentPoint = KP_INVALID_POINT;

    setUserMessage (i18n ("Resize Image: Let go of all the mouse buttons."));
    setCursor (Qt::ArrowCursor);
    m_shouldReleaseMouseButtons = true;

    releaseKeyboard ();
    emit cancelledDraw ();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpGrip::keyReleaseEvent (QKeyEvent *e)
{
    if (m_startPoint != KP_INVALID_POINT &&
        e->key () == Qt::Key_Escape)
    {
        cancel ();
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpGrip::mousePressEvent (QMouseEvent *e)
{
    if (m_startPoint == KP_INVALID_POINT &&
        (e->buttons () & Qt::MouseButtonMask) == Qt::LeftButton)
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

//---------------------------------------------------------------------

// public
QPoint kpGrip::viewDeltaPoint () const
{
    if (m_startPoint == KP_INVALID_POINT)
        return KP_INVALID_POINT;

    const QPoint point = mapFromGlobal (QCursor::pos ());

    // TODO: this is getting out of sync with m_currentPoint

    return QPoint (((m_type & kpGrip::Right) ? point.x () - m_startPoint.x () : 0),
                   ((m_type & kpGrip::Bottom) ? point.y () - m_startPoint.y () : 0));

}

//---------------------------------------------------------------------

// public
void kpGrip::mouseMovedTo (const QPoint &point, bool dueToDragScroll)
{
    if (m_startPoint == KP_INVALID_POINT)
        return;

    m_currentPoint = point;

    emit continuedDraw (((m_type & kpGrip::Right) ? point.x () - m_startPoint.x () : 0),
                        ((m_type & kpGrip::Bottom) ? point.y () - m_startPoint.y () : 0),
                        dueToDragScroll);
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpGrip::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpGrip::mouseMoveEvent() m_startPoint=" << m_startPoint
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << endl;
#endif

    if (m_startPoint == KP_INVALID_POINT)
    {
        if ((e->buttons () & Qt::MouseButtonMask) == 0)
            setUserMessage (haventBegunDrawUserMessage ());
        return;
    }

    mouseMovedTo (e->pos (), false/*not due to drag scroll*/);
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpGrip::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpGrip::mouseReleaseEvent() m_startPoint=" << m_startPoint
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << endl;
#endif

    if (m_startPoint != KP_INVALID_POINT)
    {
        const int dx = m_currentPoint.x () - m_startPoint.x (),
                  dy = m_currentPoint.y () - m_startPoint.y ();

        m_currentPoint = KP_INVALID_POINT;
        m_startPoint = KP_INVALID_POINT;

        releaseKeyboard ();
        emit endedDraw ((m_type & kpGrip::Right) ? dx : 0,
                        (m_type & kpGrip::Bottom) ? dy : 0);
    }

    if ((e->buttons () & Qt::MouseButtonMask) == 0)
    {
        m_shouldReleaseMouseButtons = false;
        setUserMessage(QString());
        setCursor (cursorForType (m_type));

        releaseKeyboard ();
        emit releasedAllButtons ();
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpGrip::enterEvent (QEvent * /*e*/)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpGrip::enterEvent()"
               << " m_startPoint=" << m_startPoint
               << " shouldReleaseMouseButtons="
               << m_shouldReleaseMouseButtons << endl;
#endif

    if (m_startPoint == KP_INVALID_POINT &&
        !m_shouldReleaseMouseButtons)
    {
    #if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
        kDebug () << "\tsending message";
    #endif
        setUserMessage (haventBegunDrawUserMessage ());
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpGrip::leaveEvent (QEvent * /*e*/)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpGrip::leaveEvent()"
               << " m_startPoint=" << m_startPoint
               << " shouldReleaseMouseButtons="
               << m_shouldReleaseMouseButtons << endl;
#endif
    if (m_startPoint == KP_INVALID_POINT &&
        !m_shouldReleaseMouseButtons)
    {
        setUserMessage(QString());
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

// TODO: Are we checking for m_view == 0 often enough?  Also an issue in KDE 3.
kpViewScrollableContainer::kpViewScrollableContainer(QWidget *parent)
    : QScrollArea(parent),
      m_view(0), m_overlay(new kpOverlay(viewport(), this)),
      m_docResizingGrip (0),
      m_dragScrollTimer (new QTimer (this)),
      m_zoomLevel (100),
      m_scrollTimerRunOnce (false),
      m_resizeRoundedLastViewX (-1), m_resizeRoundedLastViewY (-1),
      m_resizeRoundedLastViewDX (0), m_resizeRoundedLastViewDY (0),
      m_haveMovedFromOriginalDocSize (false)

{
    // the base widget holding the documents view plus the resize grips
    setWidget(new QWidget(viewport()));

    m_bottomGrip = new kpGrip(kpGrip::Bottom, widget());
    m_rightGrip = new kpGrip(kpGrip::Right, widget());
    m_bottomRightGrip = new kpGrip(kpGrip::BottomRight, widget());

    m_bottomGrip->setObjectName(QLatin1String("Bottom Grip"));
    m_rightGrip->setObjectName(QLatin1String("Right Grip"));
    m_bottomRightGrip->setObjectName(QLatin1String("BottomRight Grip"));

    m_bottomGrip->hide ();
    connectGripSignals (m_bottomGrip);

    m_rightGrip->hide ();
    connectGripSignals (m_rightGrip);

    m_bottomRightGrip->hide ();
    connectGripSignals (m_bottomRightGrip);


    connect (horizontalScrollBar(), SIGNAL(valueChanged(int)),
             this, SLOT(slotContentsMoved()));

    connect (verticalScrollBar(), SIGNAL(valueChanged(int)),
             this, SLOT(slotContentsMoved()));

    connect (m_dragScrollTimer, SIGNAL (timeout ()),
             this, SLOT (slotDragScroll ()));

    m_overlay->hide();
}

//---------------------------------------------------------------------

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

//---------------------------------------------------------------------

// public
QSize kpViewScrollableContainer::newDocSize () const
{
    return newDocSize (m_resizeRoundedLastViewDX,
                       m_resizeRoundedLastViewDY);
}

//---------------------------------------------------------------------

// public
bool kpViewScrollableContainer::haveMovedFromOriginalDocSize () const
{
    return m_haveMovedFromOriginalDocSize;
}

//---------------------------------------------------------------------

// public
QString kpViewScrollableContainer::statusMessage () const
{
    return m_gripStatusMessage;
}

//---------------------------------------------------------------------

// public
void kpViewScrollableContainer::clearStatusMessage ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 1
    kDebug () << "kpViewScrollableContainer::clearStatusMessage()";
#endif
    m_bottomRightGrip->setUserMessage(QString());
    m_bottomGrip->setUserMessage(QString());
    m_rightGrip->setUserMessage(QString());
}

//---------------------------------------------------------------------

// protected
QSize kpViewScrollableContainer::newDocSize (int viewDX, int viewDY) const
{
    if (!m_view)
        return QSize ();

    if (!docResizingGrip ())
        return QSize ();

    const int docX = (int) m_view->transformViewToDocX (m_view->width () + viewDX);
    const int docY = (int) m_view->transformViewToDocY (m_view->height () + viewDY);

    return QSize (qMax (1, docX), qMax (1, docY));
}

//---------------------------------------------------------------------

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

//---------------------------------------------------------------------

// protected
kpGrip *kpViewScrollableContainer::docResizingGrip () const
{
    return m_docResizingGrip;
}

//---------------------------------------------------------------------

// protected
int kpViewScrollableContainer::bottomResizeLineWidth () const
{
    if (!docResizingGrip ())
        return -1;

    if (!m_view)
        return -1;

    if (docResizingGrip ()->type () & kpGrip::Bottom)
        return qMax (m_view->zoomLevelY () / 100, 1);
    else
        return 1;
}

//---------------------------------------------------------------------

// protected
int kpViewScrollableContainer::rightResizeLineWidth () const
{
    if (!docResizingGrip ())
        return -1;

    if (!m_view)
        return -1;

    if (docResizingGrip ()->type () & kpGrip::Right)
        return qMax (m_view->zoomLevelX () / 100, 1);
    else
        return 1;
}

//---------------------------------------------------------------------

// protected
QRect kpViewScrollableContainer::bottomResizeLineRect () const
{
    if (m_resizeRoundedLastViewX < 0 || m_resizeRoundedLastViewY < 0)
        return QRect ();

    QRect visibleArea = QRect(QPoint(horizontalScrollBar()->value(),verticalScrollBar()->value()), viewport()->size());

    return QRect (QPoint (0,
                          m_resizeRoundedLastViewY),
                  QPoint (m_resizeRoundedLastViewX - 1,
                          m_resizeRoundedLastViewY + bottomResizeLineWidth () - 1)).intersected(visibleArea);
}

//---------------------------------------------------------------------

// protected
QRect kpViewScrollableContainer::rightResizeLineRect () const
{
    if (m_resizeRoundedLastViewX < 0 || m_resizeRoundedLastViewY < 0)
        return QRect ();

    QRect visibleArea = QRect(QPoint(horizontalScrollBar()->value(),verticalScrollBar()->value()), viewport()->size());

    return QRect (QPoint (m_resizeRoundedLastViewX,
                          0),
                  QPoint (m_resizeRoundedLastViewX + rightResizeLineWidth () - 1,
                          m_resizeRoundedLastViewY - 1)).intersected(visibleArea);
}

//---------------------------------------------------------------------

// protected
QRect kpViewScrollableContainer::bottomRightResizeLineRect () const
{
    if (m_resizeRoundedLastViewX < 0 || m_resizeRoundedLastViewY < 0)
        return QRect ();

    QRect visibleArea = QRect(QPoint(horizontalScrollBar()->value(),verticalScrollBar()->value()), viewport()->size());

    return QRect (QPoint (m_resizeRoundedLastViewX,
                          m_resizeRoundedLastViewY),
                  QPoint (m_resizeRoundedLastViewX + rightResizeLineWidth () - 1,
                          m_resizeRoundedLastViewY + bottomResizeLineWidth () - 1)).intersected(visibleArea);
}

//---------------------------------------------------------------------

// private
QRect kpViewScrollableContainer::mapViewToViewport (const QRect &viewRect)
{
    if (!viewRect.isValid ())
        return QRect ();

    QRect ret = viewRect;
    ret.translate (-horizontalScrollBar()->value() - viewport()->x(), -verticalScrollBar()->value() - viewport()->y());
    return ret;
}

//---------------------------------------------------------------------

void kpViewScrollableContainer::drawResizeLines ()
{
#define FILL_NOT_RECT(rect)                                              \
    kpPixmapFX::widgetFillNOTRect (m_overlay,                          \
        rect.x (), rect.y (), rect.width (), rect.height (),             \
        kpColor::Black/*1st hint color if "Raster NOT" not supported*/,  \
        kpColor::White/*2nd hint color if "Raster NOT" not supported*/)

    const QRect rightRect = rightResizeLineRect ();
    if (rightRect.isValid ())
        FILL_NOT_RECT (mapViewToViewport (rightRect));

    const QRect bottomRect = bottomResizeLineRect ();
    if (bottomRect.isValid ())
        FILL_NOT_RECT (mapViewToViewport (bottomRect));

    const QRect bottomRightRect = bottomRightResizeLineRect ();
    if (bottomRightRect.isValid ())
        FILL_NOT_RECT (mapViewToViewport (bottomRightRect));

#undef FILL_NOT_RECT
}

//---------------------------------------------------------------------

// protected
void kpViewScrollableContainer::updateResizeLines (int viewX, int viewY,
                                                   int viewDX, int viewDY)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER && 0
    kDebug () << "kpViewScrollableContainer::updateResizeLines("
               << viewX << "," << viewY << ")"
               << " oldViewX=" << m_resizeRoundedLastViewX
               << " oldViewY=" << m_resizeRoundedLastViewY
               << " viewDX=" << viewDX
               << " viewDY=" << viewDY
               << endl;
#endif


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

    m_overlay->update();
}

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotGripBeganDraw ()
{
    if (!m_view)
        return;

    m_overlay->resize(viewport()->size());  // make it cover whole viewport
    m_overlay->move(viewport()->pos());
    m_overlay->show();
    m_overlay->raise();  // make it top-most

    calculateDocResizingGrip ();

    m_haveMovedFromOriginalDocSize = false;

    updateResizeLines (m_view->width (), m_view->height (),
                       0/*viewDX*/, 0/*viewDY*/);

    emit beganDocResize ();
}

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotGripContinuedDraw (int inViewDX, int inViewDY,
                                                       bool dueToDragScroll)
{
    int viewDX = inViewDX,
        viewDY = inViewDY;

#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpViewScrollableContainer::slotGripContinuedDraw("
               << viewDX << "," << viewDY << ") size="
               << newDocSize (viewDX, viewDY)
               << " dueToDragScroll=" << dueToDragScroll
               << endl;
#endif

    if (!m_view)
        return;

    if (!dueToDragScroll &&
        beginDragScroll(m_view->zoomLevelX ()))
    {
        const QPoint newViewDeltaPoint = docResizingGrip ()->viewDeltaPoint ();
        viewDX = newViewDeltaPoint.x ();
        viewDY = newViewDeltaPoint.y ();
    #if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
        kDebug () << "\tdrag scrolled - new view delta point="
                   << newViewDeltaPoint
                   << endl;
    #endif
    }

    m_haveMovedFromOriginalDocSize = true;

    updateResizeLines (qMax (1, qMax (m_view->width () + viewDX, (int) m_view->transformDocToViewX (1))),
                       qMax (1, qMax (m_view->height () + viewDY, (int) m_view->transformDocToViewY (1))),
                       viewDX, viewDY);

    emit continuedDocResize (newDocSize ());
}

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotGripCancelledDraw ()
{
    m_haveMovedFromOriginalDocSize = false;

    updateResizeLines (-1, -1, 0, 0);

    calculateDocResizingGrip ();

    emit cancelledDocResize ();

    endDragScroll ();

    m_overlay->hide();
}

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotGripEndedDraw (int viewDX, int viewDY)
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpViewScrollableContainer::slotGripEndedDraw("
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

    m_overlay->hide();
}

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotGripStatusMessageChanged (const QString &string)
{
    if (string == m_gripStatusMessage)
        return;

    m_gripStatusMessage = string;
    emit statusMessageChanged (string);
}

//---------------------------------------------------------------------

// public slot
void kpViewScrollableContainer::recalculateStatusMessage ()
{
#if DEBUG_KP_VIEW_SCROLLABLE_CONTAINER
    kDebug () << "kpViewScrollabelContainer::recalculateStatusMessage()";
    kDebug () << "\tQCursor::pos=" << QCursor::pos ()
               << " global visibleRect="
               << kpWidgetMapper::toGlobal (this,
                      QRect(0, 0, viewport->width(), viewport->height()))
               << endl;
#endif

    // HACK: After dragging to a new size, handles move so that they are now
    //       under the mouse pointer but no mouseMoveEvent() is generated for
    //       any grip.  This also handles the case of canceling over any
    //       grip.
    //
    if (kpWidgetMapper::toGlobal (this,
                                  QRect(0, 0, viewport()->width(), viewport()->height()))
            .contains (QCursor::pos ()))
    {
        if ( m_bottomRightGrip->containsCursor() )
        {
            m_bottomRightGrip->setUserMessage (i18n ("Left drag the handle to resize the image."));
        }
        else if ( m_bottomGrip->containsCursor() )
        {
            m_bottomGrip->setUserMessage (i18n ("Left drag the handle to resize the image."));
        }
        else if ( m_rightGrip->containsCursor() )
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

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotContentsMoved ()
{
    kpGrip *grip = docResizingGrip ();
    if (grip)
    {
      grip->mouseMovedTo (grip->mapFromGlobal (QCursor::pos ()),
                          true/*moved due to drag scroll*/);
    }

    m_overlay->move(viewport()->pos());
    m_overlay->update();

    emit contentsMoved();
}

//---------------------------------------------------------------------

// protected
void kpViewScrollableContainer::disconnectViewSignals ()
{
    disconnect (m_view, SIGNAL (sizeChanged (const QSize &)),
                this, SLOT (updateGrips ()));
    disconnect (m_view, SIGNAL (destroyed ()),
                this, SLOT (slotViewDestroyed ()));
}

//---------------------------------------------------------------------

// protected
void kpViewScrollableContainer::connectViewSignals ()
{
    connect (m_view, SIGNAL (sizeChanged (const QSize &)),
             this, SLOT (updateGrips ()));
    connect (m_view, SIGNAL (destroyed ()),
             this, SLOT (slotViewDestroyed ()));
}

//---------------------------------------------------------------------

// public
kpView *kpViewScrollableContainer::view () const
{
    return m_view;
}

//---------------------------------------------------------------------

// public
void kpViewScrollableContainer::setView (kpView *view)
{
    if (m_view == view)
        return;

    if (m_view)
    {
      disconnectViewSignals ();
    }

    m_view = view;

    if ( m_view )
    {
      m_view->setParent(widget());
      m_view->show();
    }

    updateGrips ();

    if (m_view)
    {
      connectViewSignals ();
    }
}

//---------------------------------------------------------------------

// public slot
void kpViewScrollableContainer::updateGrips ()
{
    if (m_view)
    {
      widget()->resize(m_view->size() + m_bottomRightGrip->size());

      // to make the grip more easily "touchable" make it as high as the view
      m_rightGrip->setFixedHeight(m_view->height());
      m_rightGrip->move(m_view->width(), 0);

      // to make the grip more easily "touchable" make it as wide as the view
      m_bottomGrip->setFixedWidth(m_view->width());
      m_bottomGrip->move(0, m_view->height ());

      m_bottomRightGrip->move(m_view->width(), m_view->height());
    }

    m_bottomGrip->setHidden (m_view == 0);
    m_rightGrip->setHidden (m_view == 0);
    m_bottomRightGrip->setHidden (m_view == 0);

    recalculateStatusMessage ();
}

//---------------------------------------------------------------------

// protected slot
void kpViewScrollableContainer::slotViewDestroyed ()
{
    m_view = 0;
    updateGrips ();
}

//---------------------------------------------------------------------

// public slot
bool kpViewScrollableContainer::beginDragScroll(int zoomLevel, bool *didSomething)
{
    if (didSomething)
        *didSomething = false;

    m_zoomLevel = zoomLevel;

    const QPoint p = mapFromGlobal (QCursor::pos ());

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

//---------------------------------------------------------------------

// public slot
bool kpViewScrollableContainer::beginDragScroll(int zoomLevel)
{
    return beginDragScroll(zoomLevel,
                           0/*don't want scrolled notification*/);
}

//---------------------------------------------------------------------

// public slot
bool kpViewScrollableContainer::endDragScroll ()
{
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

//---------------------------------------------------------------------

static int distanceFromRectToMultiplier (int dist)
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

//---------------------------------------------------------------------

// protected slot
bool kpViewScrollableContainer::slotDragScroll (bool *didSomething)
{
    bool scrolled = false;

    if (didSomething)
        *didSomething = false;


    const QRect rect = noDragScrollRect ();
    const QPoint pos = mapFromGlobal (QCursor::pos ());

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

    dx *= dxMultiplier;// * qMax (1, m_zoomLevel / 100);
    dy *= dyMultiplier;// * qMax (1, m_zoomLevel / 100);

    if (dx || dy)
    {
        const int oldContentsX = horizontalScrollBar()->value (),
                  oldContentsY = verticalScrollBar()->value ();

        horizontalScrollBar()->setValue(oldContentsX + dx);
        verticalScrollBar()->setValue(oldContentsY + dy);

        scrolled = (oldContentsX != horizontalScrollBar()->value () ||
                    oldContentsY != verticalScrollBar()->value ());

        if (scrolled)
        {
            QRegion region = QRect (horizontalScrollBar()->value (), verticalScrollBar()->value (),
                                    viewport()->width(), viewport()->height());
            region -= QRect (oldContentsX, oldContentsY,
                             viewport()->width(), viewport()->height());

            // Repaint newly exposed region immediately to reduce tearing
            // of scrollView.
            m_view->repaint (region);
        }
    }

    m_dragScrollTimer->start (DragScrollInterval);
    m_scrollTimerRunOnce = true;

    if (didSomething)
        *didSomething = scrolled;

    return scrolled;
}

//---------------------------------------------------------------------

// protected slot
bool kpViewScrollableContainer::slotDragScroll ()
{
    return slotDragScroll (0/*don't want scrolled notification*/);
}

//---------------------------------------------------------------------

// protected virtual
void kpViewScrollableContainer::wheelEvent (QWheelEvent *e)
{
    e->ignore ();

    if (m_view)
        m_view->wheelEvent (e);

    if ( !e->isAccepted() )
        QScrollArea::wheelEvent(e);
}

//---------------------------------------------------------------------

QRect kpViewScrollableContainer::noDragScrollRect () const
{
    return QRect (DragScrollLeftTopMargin, DragScrollLeftTopMargin,
                  width () - DragScrollLeftTopMargin - DragScrollRightBottomMargin,
                  height () - DragScrollLeftTopMargin - DragScrollRightBottomMargin);
}

//---------------------------------------------------------------------

// protected virtual [base QScrollView]
void kpViewScrollableContainer::resizeEvent (QResizeEvent *e)
{
    QScrollArea::resizeEvent (e);

    emit resized ();
}

//---------------------------------------------------------------------

#include <kpViewScrollableContainer.moc>
