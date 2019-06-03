
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


#include <QLabel>
#include <QPoint>
#include <QScrollArea>
#include <QSize>


class QCursor;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QRect;
class QResizeEvent;
class QTimer;

class kpView;
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
    void keyReleaseEvent (QKeyEvent *e) override;
    void mousePressEvent (QMouseEvent *e) override;
public:
    QPoint viewDeltaPoint () const;
    void mouseMovedTo (const QPoint &point, bool dueToDragScroll);
protected:
    void mouseMoveEvent (QMouseEvent *e) override;
    void mouseReleaseEvent (QMouseEvent *e) override;

    void enterEvent (QEvent *e) override;
    void leaveEvent (QEvent *e) override;

protected:
    GripType m_type;
    QPoint m_startPoint, m_currentPoint;
    QString m_userMessage;
    bool m_shouldReleaseMouseButtons;
};

//---------------------------------------------------------------------

class kpViewScrollableContainer : public QScrollArea
{
Q_OBJECT

public:
    kpViewScrollableContainer(QWidget *parent);

    QSize newDocSize () const;
    bool haveMovedFromOriginalDocSize () const;
    QString statusMessage () const;
    void clearStatusMessage ();

    kpView *view () const;
    void setView (kpView *view);

    void drawResizeLines();  // public only for kpOverlay

signals:
    void contentsMoved();

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

    // TODO: Why the need for view's zoomLevel?  We have the view() anyway.
    bool beginDragScroll (int zoomLevel,
                          bool *didSomething);
    bool beginDragScroll (int zoomLevel);
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

    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void slotGripBeganDraw ();
    void slotGripContinuedDraw (int viewDX, int viewDY, bool dueToScrollView);
    void slotGripCancelledDraw ();
    void slotGripEndedDraw (int viewDX, int viewDY);

    void slotGripStatusMessageChanged (const QString &string);

    void slotContentsMoved ();
    void slotViewDestroyed ();
    bool slotDragScroll (bool *didSomething = nullptr);

private:
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
