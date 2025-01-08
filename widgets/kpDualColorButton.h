
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpDualColorButton_H
#define kpDualColorButton_H

#include <QFrame>

#include "imagelib/kpColor.h"

class kpDualColorButton : public QFrame
{
    Q_OBJECT

public:
    explicit kpDualColorButton(QWidget *parent);

    kpColor color(int which) const;
    kpColor foregroundColor() const;
    kpColor backgroundColor() const;

public Q_SLOTS:
    void setColor(int which, const kpColor &color);
    void setForegroundColor(const kpColor &color);
    void setBackgroundColor(const kpColor &color);

Q_SIGNALS:
    // If you connect to this signal, ignore the following
    // foregroundColorChanged() and backgroundColorChanged() signals
    void colorsSwapped(const kpColor &newForegroundColor, const kpColor &newBackgroundColor);

    void foregroundColorChanged(const kpColor &color);
    void backgroundColorChanged(const kpColor &color);

public:
    // (only valid in slots connected to foregroundColorChanged())
    kpColor oldForegroundColor() const;
    // (only valid in slots connected to backgroundColorChanged())
    kpColor oldBackgroundColor() const;

public:
    QSize sizeHint() const override;

protected:
    QRect swapPixmapRect() const;
    QRect foregroundBackgroundRect() const;
    QRect foregroundRect() const;
    QRect backgroundRect() const;

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void mouseDoubleClickEvent(QMouseEvent *e) override;

    void paintEvent(QPaintEvent *e) override;

    QPoint m_dragStartPoint;
    kpColor m_color[2];
    kpColor m_oldColor[2];
};

#endif // kpDualColorButton_H
