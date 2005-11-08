
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


#ifndef __kp_color_toolbar_h__
#define __kp_color_toolbar_h__


#include <qframe.h>
#include <qwidget.h>

#include <kcolordialog.h>
#include <ktoolbar.h>

#include <kpcolor.h>
#include <kpcolorsimilaritycube.h>


class QGridLayout;
class KColorButton;

class kpColorSimilarityCube;
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
    kpColor m_color [2];
    kpColor m_oldColor [2];
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

    // lazy
    void foregroundColorChanged (const kpColor &color);
    void backgroundColorChanged (const kpColor &color);

protected:
    Qt::Orientation m_orientation;

    virtual void dropEvent (QDropEvent *e);
    virtual void paintCell (QPainter *painter, int row, int col);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void resizeEvent (QResizeEvent *e);

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
    void foregroundColorChanged (const kpColor &color);
    void backgroundColorChanged (const kpColor &color);

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
    void foregroundColorChanged (const kpColor &color);
    void backgroundColorChanged (const kpColor &color);

protected:
    Qt::Orientation m_orientation;

    QBoxLayout *m_boxLayout;
    kpTransparentColorCell *m_transparentColorCell;
    kpColorCells *m_colorCells;
};


class kpColorSimilarityToolBarItem : public kpColorSimilarityCube
{
Q_OBJECT

public:
    kpColorSimilarityToolBarItem (kpMainWindow *mainWindow,
                                  QWidget *parent,
                                  const char *name = 0);
    virtual ~kpColorSimilarityToolBarItem ();

public:
    int processedColorSimilarity () const;

public slots:
    void setColorSimilarity (double similarity);

signals:
    void colorSimilarityChanged (double similarity, int processedSimilarity);

public:
    // (only valid in slots connected to colorSimilarityChanged());
    double oldColorSimilarity () const;

private:
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseDoubleClickEvent (QMouseEvent *e);

private:
    kpMainWindow *m_mainWindow;

    double m_oldColorSimilarity;
    int m_processedColorSimilarity;
};


class kpColorToolBar : public KToolBar
{
Q_OBJECT

public:
    kpColorToolBar (const QString &label, kpMainWindow *mainWindow, const char *name = 0);
    virtual ~kpColorToolBar ();

    kpColor color (int which) const;
    void setColor (int which, const kpColor &color);

    kpColor foregroundColor () const;
    kpColor backgroundColor () const;

    double colorSimilarity () const;
    void setColorSimilarity (double similarity);
    int processedColorSimilarity () const;

signals:
    // If you connect to this signal, ignore the following
    // foregroundColorChanged() and backgroundColorChanged() signals
    void colorsSwapped (const kpColor &newForegroundColor,
                        const kpColor &newBackgroundColor);

    void foregroundColorChanged (const kpColor &color);
    void backgroundColorChanged (const kpColor &color);
    void colorSimilarityChanged (double similarity, int processedSimilarity);

public:
    // (only valid in slots connected to foregroundColorChanged())
    kpColor oldForegroundColor () const;
    // (only valid in slots connected to backgroundColorChanged())
    kpColor oldBackgroundColor () const;

    // (only valid in slots connected to colorSimilarityChanged())
    double oldColorSimilarity () const;

public slots:
    void setForegroundColor (const kpColor &color);
    void setBackgroundColor (const kpColor &color);

private:
    kpMainWindow *m_mainWindow;

    Qt::Orientation m_lastDockedOrientation;
    bool m_lastDockedOrientationSet;
    virtual void setOrientation (Qt::Orientation o);

    QBoxLayout *m_boxLayout;
    kpDualColorButton *m_dualColorButton;
    kpColorPalette *m_colorPalette;
    kpColorSimilarityToolBarItem *m_colorSimilarityToolBarItem;
};

#endif  // __kp_color_toolbar_h__
