
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransparentColorCell_H
#define kpTransparentColorCell_H

#include <QFrame>

class kpColor;

class kpTransparentColorCell : public QFrame
{
    Q_OBJECT

public:
    explicit kpTransparentColorCell(QWidget *parent);

    QSize sizeHint() const override;

Q_SIGNALS:
    void transparentColorSelected(int mouseButton);

    // lazy
    void foregroundColorChanged(const kpColor &color);
    void backgroundColorChanged(const kpColor &color);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void paintEvent(QPaintEvent *e) override;

    QPixmap m_pixmap;
};

#endif // kpTransparentColorCell_H
