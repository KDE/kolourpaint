
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
#include <qframe.h>
#include <qwidget.h>

#include <kcolordialog.h>
#include <ktoolbar.h>


class QGridLayout;
class KColorButton;

class kpMainWindow;


//
// Widget similar to KDualColorButton.
// Main differences:
// - more consistent feel with other KolourPaint widgets
//   (esp. kpColorPalette)
// - displays the transparent colour using the special pixmap
//   used by kpTransparentColorCell
// - no obscure "current" colour
//
class kpDualColorButton : public QFrame
{
Q_OBJECT

public:
    kpDualColorButton (kpMainWindow *mainWindow,
                       QWidget *parent, const char *name = 0);
    virtual ~kpDualColorButton ();

    QColor color (int which) const;
    QColor foregroundColor () const;
    QColor backgroundColor () const;
    
public slots:
    void setColor (int which, const QColor &color);
    void setForegroundColor (const QColor &color);
    void setBackgroundColor (const QColor &color);

signals:
    void foregroundColorChanged (const QColor &color);
    void backgroundColorChanged (const QColor &color);

public:
    virtual QSize sizeHint () const;
    
protected:
    QRect swapPixmapRect () const;
    QRect foregroundBackgroundRect () const;
    QRect foregroundRect () const;
    QRect backgroundRect () const;

    //virtual void dragEnterEvent (QDragEnterEvent *e);
    virtual void dragMoveEvent (QDragMoveEvent *e);
    virtual void dropEvent (QDropEvent *e);
    
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseDoubleClickEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);

    virtual void drawContents (QPainter *p);
    
    kpMainWindow *m_mainWindow;
    QColor m_color [2];
    QPixmap *m_backBuffer;
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


class kpTransparentColorCell : public QFrame
{
Q_OBJECT

public:
    kpTransparentColorCell (QWidget *parent, const char *name = 0);
    virtual ~kpTransparentColorCell ();
    
    virtual QSize sizeHint () const;
    
signals:
    void transparentColorSelected (int mouseButton);
    
    // lazy
    void foregroundColorChanged (const QColor &color);
    void backgroundColorChanged (const QColor &color);

protected:
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    
    virtual void drawContents (QPainter *p);
    
    QPixmap m_pixmap;
};


class kpColorPalette : public QWidget
{
Q_OBJECT

public:
    kpColorPalette (QWidget *parent,
                    Qt::Orientation o = Qt::Horizontal,
                    const char *name = 0);
    virtual ~kpColorPalette ();
    
    Qt::Orientation orientation () const;
    void setOrientation (Qt::Orientation o);
    
signals:
    void foregroundColorChanged (const QColor &color);
    void backgroundColorChanged (const QColor &color);
    
protected:
    Qt::Orientation m_orientation;

    QBoxLayout *m_boxLayout;
    kpTransparentColorCell *m_transparentColorCell;
    kpColorCells *m_colorCells;    
};


class kpColorToolBar : public KToolBar
{
Q_OBJECT

public:
    kpColorToolBar (kpMainWindow *mainWindow, const char *name = 0);
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
    kpMainWindow *m_mainWindow;

    Qt::Orientation m_lastDockedOrientation;
    bool m_lastDockedOrientationSet;
    virtual void setOrientation (Qt::Orientation o);

    QBoxLayout *m_boxLayout;
    kpDualColorButton *m_dualColorButton;
    kpColorPalette *m_colorPalette;
};

#endif  // __kp_color_toolbar_h__
