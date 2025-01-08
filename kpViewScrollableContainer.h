
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_VIEW_SCROLLABLE_CONTAINER_H
#define KP_VIEW_SCROLLABLE_CONTAINER_H

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
    enum GripType {
        Right = 1,
        Bottom = 2,
        BottomRight = Right | Bottom
    };

    kpGrip(GripType type, QWidget *parent);

    GripType type() const;

    static QCursor cursorForType(GripType type);

    bool containsCursor();

    bool isDrawing() const;

    static const int Size;

Q_SIGNALS:
    void beganDraw();
    void continuedDraw(int viewDX, int viewDY, bool dueToDragScroll);
    void cancelledDraw();
    void endedDraw(int viewDX, int viewDY);

    void statusMessageChanged(const QString &string);

    void releasedAllButtons();

public:
    QString haventBegunDrawUserMessage() const;

    QString userMessage() const;
    void setUserMessage(const QString &message);

protected:
    void cancel();

protected:
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

public:
    QPoint viewDeltaPoint() const;
    void mouseMovedTo(const QPoint &point, bool dueToDragScroll);

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;

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
    explicit kpViewScrollableContainer(QWidget *parent);

    QSize newDocSize() const;
    bool haveMovedFromOriginalDocSize() const;
    QString statusMessage() const;
    void clearStatusMessage();

    kpView *view() const;
    void setView(kpView *view);

    void drawResizeLines(); // public only for kpOverlay

Q_SIGNALS:
    void contentsMoved();

    void beganDocResize();
    void continuedDocResize(const QSize &size);
    void cancelledDocResize();
    void endedDocResize(const QSize &size);

    // (string.isEmpty() if kpViewScrollableContainer has nothing to say)
    void statusMessageChanged(const QString &string);

    void resized();

public Q_SLOTS:
    void recalculateStatusMessage();

    void updateGrips();

    // TODO: Why the need for view's zoomLevel?  We have the view() anyway.
    bool beginDragScroll(int zoomLevel, bool *didSomething);
    bool beginDragScroll(int zoomLevel);
    bool endDragScroll();

private:
    void connectGripSignals(kpGrip *grip);

    QSize newDocSize(int viewDX, int viewDY) const;

    void calculateDocResizingGrip();
    kpGrip *docResizingGrip() const;

    int bottomResizeLineWidth() const;
    int rightResizeLineWidth() const;

    QRect bottomResizeLineRect() const;
    QRect rightResizeLineRect() const;
    QRect bottomRightResizeLineRect() const;

    QRect mapViewToViewport(const QRect &viewRect);

    void updateResizeLines(int viewX, int viewY, int viewDX, int viewDY);

    void disconnectViewSignals();
    void connectViewSignals();

    QRect noDragScrollRect() const;

    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private Q_SLOTS:
    void slotGripBeganDraw();
    void slotGripContinuedDraw(int viewDX, int viewDY, bool dueToScrollView);
    void slotGripCancelledDraw();
    void slotGripEndedDraw(int viewDX, int viewDY);

    void slotGripStatusMessageChanged(const QString &string);

    void slotContentsMoved();
    void slotViewDestroyed();
    bool slotDragScroll(bool *didSomething = nullptr);

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

#endif // KP_VIEW_SCROLLABLE_CONTAINER_H
