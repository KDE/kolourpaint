
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __kp_color_toolbar_h__
#define __kp_color_toolbar_h__

#include <qcolor.h>

#include <kcolordialog.h>
#include <ktoolbar.h>

class QBoxLayout;
class KColorButton;

// TODO: provide a pretty way of displaying foreground, background &
//       transparent colour
class kpDualColorButton : public QWidget
{
Q_OBJECT

public:
    kpDualColorButton (QWidget *parent,
                       Qt::Orientation o = Qt::Vertical,
                       const char *name = 0);
    virtual ~kpDualColorButton ();

    Qt::Orientation orientation () const;
    void setOrientation (Qt::Orientation o);
    
    QColor color (int which) const;
    void setColor (int which, const QColor &color);

    QColor foregroundColor () const;
    void setForegroundColor (const QColor &color);

    QColor backgroundColor () const;
    void setBackgroundColor (const QColor &color);

private:
    KColorButton *m_colorButton [2];
    QBoxLayout *m_boxLayout;
};

class kpColorCells : public KColorCells
{
Q_OBJECT

public:
    kpColorCells (QWidget *parent,
                  Qt::Orientation o = Qt::Horizontal,
                  const char *name = 0);
    virtual ~kpColorCells ();

    Qt::Orientation orientation () const;
    void setOrientation (Qt::Orientation o);
    
signals:
    void foregroundColorChanged (const QColor &color);
    void backgroundColorChanged (const QColor &color);

protected:
    Qt::Orientation m_orientation;

    virtual void paintCell (QPainter *painter, int row, int col);
    virtual void mouseReleaseEvent (QMouseEvent *e);

    int m_mouseButton;

protected slots:
    void slotColorSelected (int cell);
    void slotColorDoubleClicked (int cell);
};

class kpColorToolBar : public KToolBar
{
Q_OBJECT

public:
    kpColorToolBar (QWidget *parent, const char *name = 0);
    virtual ~kpColorToolBar ();

    QColor color (int which) const;
    void setColor (int which, const QColor &color);

    QColor foregroundColor () const;

    QColor backgroundColor () const;

signals:
    void foregroundColorChanged (const QColor &color);
    void backgroundColorChanged (const QColor &color);

public slots:
    void setForegroundColor (const QColor &color);
    void setBackgroundColor (const QColor &color);

private:
    virtual void setOrientation (Qt::Orientation o);

    QBoxLayout *m_boxLayout;
    kpDualColorButton *m_dualColorButton;
    kpColorCells *m_colorCells;
};

#endif  // __kp_color_toolbar_h__
