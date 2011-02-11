
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


#ifndef KP_VIEW_SCROLLABLE_CONTAINER_H
#define KP_VIEW_SCROLLABLE_CONTAINER_H


#include <qlabel.h>
#include <qpoint.h>
#include <q3scrollview.h>
#include <qsize.h>


class QCursor;
class QDragMoveEvent;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QRect;
class QResizeEvent;
class QTimer;

class kpView;
class kpMainWindow;
class kpOverlay;


//---------------------------------------------------------------------

// REFACTOR: refactor by sharing iface's with kpTool
class kpGrip : public QWidget
{
Q_OBJECT

public:
    enum GripType
    {
        Right = 1, Bottom = 2,
        BottomRight = Right | Bottom
    };

    kpGrip (GripType type, QWidget *parent);

    GripType type () const;

    static QCursor cursorForType (GripType type);

    bool containsCursor();

    bool isDrawing () const;

    static const int Size;

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

    virtual void enterEvent (QEvent *e);
    virtual void leaveEvent (QEvent *e);

protected:
    GripType m_type;
    QPoint m_startPoint, m_currentPoint;
    QString m_userMessage;
    bool m_shouldReleaseMouseButtons;
};

//---------------------------------------------------------------------

class kpViewScrollableContainer : public Q3ScrollView
{
Q_OBJECT

public:
    kpViewScrollableContainer (kpMainWindow *parent);

    // Same as contentsX() and contentsY() except that after
    // contentsMovingSoon() is emitted and before the scrollview actually
    // scrolls, they return the would be values of contentsX() and
    // contentsY() after scrolling.
    int contentsXSoon ();
    int contentsYSoon ();

    QSize newDocSize () const;
    bool haveMovedFromOriginalDocSize () const;
    QString statusMessage () const;
    void clearStatusMessage ();

    // Calls setView(<widget>) after adding <widget> if it's a kpView.
    virtual void addChild (QWidget *widget, int x = 0, int y = 0);

    kpView *view () const;
    void setView (kpView *view);

    void drawResizeLines();  // public only for kpOverlay

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

public slots:
    void recalculateStatusMessage ();

    void updateGrips ();

    // TODO: Why the QPoint's?
    //       Why the need for view's zoomLevel?  We have the view() anyway.
    bool beginDragScroll (const QPoint &, const QPoint &,
                          int zoomLevel,
                          bool *didSomething);
    bool beginDragScroll (const QPoint &, const QPoint &,
                          int zoomLevel);
    bool endDragScroll ();

private:
    void connectGripSignals (kpGrip *grip);

    QSize newDocSize (int viewDX, int viewDY) const;

    void calculateDocResizingGrip ();
    kpGrip *docResizingGrip () const;

    int bottomResizeLineWidth () const;
    int rightResizeLineWidth () const;

    QRect bottomResizeLineRect () const;
    QRect rightResizeLineRect () const;
    QRect bottomRightResizeLineRect () const;

    QRect mapViewToViewport (const QRect &viewRect);

    void updateResizeLines (int viewX, int viewY,
                            int viewDX, int viewDY);

    void disconnectViewSignals ();
    void connectViewSignals ();

    QRect noDragScrollRect () const;

    virtual void contentsWheelEvent (QWheelEvent *e);
    virtual void resizeEvent (QResizeEvent *e);

private slots:
    void slotGripBeganDraw ();
    void slotGripContinuedDraw (int viewDX, int viewDY, bool dueToScrollView);
    void slotGripCancelledDraw ();
    void slotGripEndedDraw (int viewDX, int viewDY);

    void slotGripStatusMessageChanged (const QString &string);

    void slotContentsMoving (int x, int y);
    void slotContentsMoved ();
    void slotViewDestroyed ();
    bool slotDragScroll (bool *didSomething);
    bool slotDragScroll ();

private:
    kpMainWindow *m_mainWindow;
    int m_contentsXSoon, m_contentsYSoon;
    kpView *m_view;
    kpOverlay *m_overlay;
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
