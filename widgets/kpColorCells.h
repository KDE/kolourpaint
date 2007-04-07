
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


#ifndef kpColorCells_H
#define kpColorCells_H


#include <kcolordialog.h>


class QDropEvent;

class kpColor;


class kpColorCells : public KColorCells
{
Q_OBJECT

public:
    kpColorCells (QWidget *parent,
                  Qt::Orientation o = Qt::Horizontal);
    virtual ~kpColorCells ();

    Qt::Orientation orientation () const;
    void setOrientation (Qt::Orientation o);

signals:
    void foregroundColorChanged (const QColor &color);
    void backgroundColorChanged (const QColor &color);

    // lazy
    void foregroundColorChanged (const kpColor &color);
    void backgroundColorChanged (const kpColor &color);

protected:
    Qt::Orientation m_orientation;

    virtual void dropEvent (QDropEvent *e);
    virtual void paintCell (QPainter *painter, int row, int col);
    virtual void contextMenuEvent (QContextMenuEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void resizeEvent (QResizeEvent *e);

    int m_mouseButton;

protected slots:
    void slotColorSelected (int cell);
    void slotColorDoubleClicked (int cell,const QColor&);
};


#endif  // kpColorCells_H
