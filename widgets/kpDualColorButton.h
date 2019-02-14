
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


#ifndef kpDualColorButton_H
#define kpDualColorButton_H


#include <QFrame>

#include "imagelib/kpColor.h"


class kpDualColorButton : public QFrame
{
Q_OBJECT

public:
    kpDualColorButton (QWidget *parent);

    kpColor color (int which) const;
    kpColor foregroundColor () const;
    kpColor backgroundColor () const;

public slots:
    void setColor (int which, const kpColor &color);
    void setForegroundColor (const kpColor &color);
    void setBackgroundColor (const kpColor &color);

signals:
    // If you connect to this signal, ignore the following
    // foregroundColorChanged() and backgroundColorChanged() signals
    void colorsSwapped (const kpColor &newForegroundColor,
                        const kpColor &newBackgroundColor);

    void foregroundColorChanged (const kpColor &color);
    void backgroundColorChanged (const kpColor &color);

public:
    // (only valid in slots connected to foregroundColorChanged())
    kpColor oldForegroundColor () const;
    // (only valid in slots connected to backgroundColorChanged())
    kpColor oldBackgroundColor () const;

public:
    QSize sizeHint () const override;

protected:
    QRect swapPixmapRect () const;
    QRect foregroundBackgroundRect () const;
    QRect foregroundRect () const;
    QRect backgroundRect () const;

    void dragEnterEvent (QDragEnterEvent *e) override;
    void dragMoveEvent (QDragMoveEvent *e) override;
    void dropEvent (QDropEvent *e) override;

    void mousePressEvent (QMouseEvent *e) override;
    void mouseMoveEvent (QMouseEvent *e) override;
    void mouseReleaseEvent (QMouseEvent *e) override;

    void mouseDoubleClickEvent (QMouseEvent *e) override;

    void paintEvent (QPaintEvent *e) override;

    QPoint m_dragStartPoint;
    kpColor m_color [2];
    kpColor m_oldColor [2];
};


#endif  // kpDualColorButton_H
