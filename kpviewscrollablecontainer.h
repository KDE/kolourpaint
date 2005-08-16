
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


#ifndef KP_VIEW_SCROLLABLE_CONTAINER_H
#define KP_VIEW_SCROLLABLE_CONTAINER_H


#include <qpoint.h>
#include <qscrollview.h>
#include <qsize.h>


class QCursor;
class QRect;
class QTimer;

class kpGrip;
class kpView;
class kpMainWindow;


// TODO: refactor by sharing iface's with kpTool
class kpGrip : public QWidget
{
Q_OBJECT

public:
    enum GripType
    {
        Right = 1, Bottom = 2,
        BottomRight = Right | Bottom
    };

    kpGrip (GripType type,
            QWidget *parent, const char *name = 0);
    virtual ~kpGrip ();

    GripType type () const;

    static const QCursor &cursorForType (GripType type);

    QRect hotRect (bool toGlobal = false) const;

    bool isDrawing () const;

signals:
    void beganDraw ();
    void continuedDraw (int viewDX, int viewDY, bool dueToDragScroll);
    void cancelledDraw ();
    void endedDraw (int viewDX, int viewDY);

    void statusMessageChanged (const QString &string);

    void releasedAllButtons ();

public:
    QString haventBegunDrawUserMessage () const;

    QString userMessage () const;
    void setUserMessage (const QString &message);

protected:
    void updatePixmap ();
    void cancel ();

protected:
    virtual void keyReleaseEvent (QKeyEvent *e);
    virtual void mousePressEvent (QMouseEvent *e);
public:
    QPoint viewDeltaPoint () const;
    void mouseMovedTo (const QPoint &point, bool dueToDragScroll);
protected:
    virtual void mouseMoveEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void resizeEvent (QResizeEvent *e);

    virtual void enterEvent (QEvent *e);
    virtual void leaveEvent (QEvent *e);

    virtual void paintEvent (QPaintEvent *e);

protected:
    GripType m_type;
    QPoint m_startPoint, m_currentPoint;
    QString m_userMessage;
    bool m_shouldReleaseMouseButtons;
};


class kpViewScrollableContainer : public QScrollView
{
Q_OBJECT

public:
    kpViewScrollableContainer (kpMainWindow *parent, const char *name = 0);
    virtual ~kpViewScrollableContainer ();

    // Same as contentsX() and contentsY() except that after
    // contentsMovingSoon() is emitted and before the scrollview actually
    // scrolls, they return the would be values of contentsX() and
    // contentsY() after scrolling.
    int contentsXSoon ();
    int contentsYSoon ();

signals:
    // connect to this instead of contentsMoving(int,int) so that
    // contentsXSoon() and contentsYSoon() work
    void contentsMovingSoon (int contentsX, int contentsY);

    void beganDocResize ();
    void continuedDocResize (const QSize &size);
    void cancelledDocResize ();
    void endedDocResize (const QSize &size);

    // (string.isEmpty() if kpViewScrollableContainer has nothing to say)
    void statusMessageChanged (const QString &string);

    void resized ();

public:
    QSize newDocSize () const;
    bool haveMovedFromOriginalDocSize () const;
    QString statusMessage () const;
    void clearStatusMessage ();

protected:
    void connectGripSignals (kpGrip *grip);

    QSize newDocSize (int viewDX, int viewDY) const;

    void calculateDocResizingGrip ();
    kpGrip *docResizingGrip () const;

    int bottomResizeLineWidth () const;
    int rightResizeLineWidth () const;

    QRect bottomResizeLineRect () const;
    QRect rightResizeLineRect () const;
    QRect bottomRightResizeLineRect () const;

    QPoint mapViewToViewport (const QPoint &viewPoint);
    QRect mapViewToViewport (const QRect &viewRect);

    QRect mapViewportToGlobal (const QRect &viewportRect);
    QRect mapViewToGlobal (const QRect &viewRect);

    void repaintWidgetAtResizeLineViewRect (QWidget *widget,
                                            const QRect &resizeLineViewRect);
    void repaintWidgetAtResizeLines (QWidget *widget);
    void eraseResizeLines ();

    void drawResizeLines ();

    void updateResizeLines (int viewX, int viewY,
                            int viewDX, int viewDY);

protected slots:
    void slotGripBeganDraw ();
    void slotGripContinuedDraw (int viewDX, int viewDY, bool dueToScrollView);
    void slotGripCancelledDraw ();
    void slotGripEndedDraw (int viewDX, int viewDY);

    void slotGripStatusMessageChanged (const QString &string);

public slots:
    void recalculateStatusMessage ();

protected slots:
    void slotContentsMoving (int x, int y);
    void slotContentsMoved ();

protected:
    void disconnectViewSignals ();
    void connectViewSignals ();

public:
    // Calls setView(<widget>) after adding <widget> if it's a kpView.
    virtual void addChild (QWidget *widget, int x = 0, int y = 0);

    kpView *view () const;
    void setView (kpView *view);

public slots:
    void updateGrips ();
protected slots:
    void slotViewDestroyed ();

public slots:
    // TODO: Why the QPoint's?
    //       Why the need for view's zoomLevel?  We have the view() anyway.
    bool beginDragScroll (const QPoint &, const QPoint &,
                          int zoomLevel,
                          bool *didSomething);
    bool beginDragScroll (const QPoint &, const QPoint &,
                          int zoomLevel);
    bool endDragScroll ();

protected slots:
    bool slotDragScroll (bool *didSomething);
    bool slotDragScroll ();

protected:
    QRect noDragScrollRect () const;

    virtual void contentsDragMoveEvent (QDragMoveEvent *e);
    virtual void contentsMouseMoveEvent (QMouseEvent *e);
    virtual void contentsWheelEvent (QWheelEvent *e);
    virtual void mouseMoveEvent (QMouseEvent *e);
    virtual bool eventFilter (QObject *watchedObject, QEvent *e);
    virtual void viewportPaintEvent (QPaintEvent *e);
    virtual void paintEvent (QPaintEvent *e);
    virtual void resizeEvent (QResizeEvent *e);

protected:
    kpMainWindow *m_mainWindow;
    int m_contentsXSoon, m_contentsYSoon;
    kpView *m_view;
    kpGrip *m_bottomGrip, *m_rightGrip, *m_bottomRightGrip;
    kpGrip *m_docResizingGrip;
    QTimer *m_dragScrollTimer;
    int m_zoomLevel;
    bool m_scrollTimerRunOnce;
    int m_resizeRoundedLastViewX, m_resizeRoundedLastViewY;
    int m_resizeRoundedLastViewDX, m_resizeRoundedLastViewDY;
    bool m_haveMovedFromOriginalDocSize;
    QString m_gripStatusMessage;
};


#endif  // KP_VIEW_SCROLLABLE_CONTAINER_H
