
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#ifndef __kptoolwidgetbase_h__
#define __kptoolwidgetbase_h__

#include <qframe.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qvaluevector.h>
#include <qwidget.h>


class QPainter;

// TODO: frame becomes a combobox when its parent kpToolToolBar becomes too small
class kpToolWidgetBase : public QFrame
{
Q_OBJECT

public:
    kpToolWidgetBase (QWidget *parent);
    virtual ~kpToolWidgetBase ();

    void addOption (const QPixmap &pixmap, const QString &toolTip = QString::null);
    void startNewOptionRow ();

private:
    QValueVector <int> spreadOutElements (const QValueVector <int> &sizes, int maxSize);

public:
    void relayoutOptions ();

    int selectedRow () const;
    int selectedCol () const;

    int selected () const;

public slots:
    virtual void setSelected (int row, int col);

signals:
    void optionSelected (int row, int col);

protected:
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void drawContents (QPainter *painter);
    
    void setInvertSelectedPixmap (bool yes = true) { m_invertSelectedPixmap = yes; }
    bool m_invertSelectedPixmap;

    // coulbe be a QFrame or a ComboBox
    QWidget *m_baseWidget;

    QValueVector < QValueVector <QPixmap> > m_pixmaps;
    QValueVector < QValueVector <QString> > m_toolTips;
    
    QValueVector < QValueVector <QRect> > m_pixmapRects;

    int m_selectedRow, m_selectedCol;
};

#endif  // __kptoolwidgetbase_h__
