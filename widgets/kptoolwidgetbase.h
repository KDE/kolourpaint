
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

#ifndef __kptoolwidgetbase_h__
#define __kptoolwidgetbase_h__

#include <qframe.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qvaluevector.h>

class QPainter;

class kpToolWidgetBase : public QFrame
{
Q_OBJECT

public:
    kpToolWidgetBase (QWidget *parent);
    virtual ~kpToolWidgetBase ();

    int addOption (const QPixmap &pixmap, bool center = false, bool doUpdate = true);

    int selected (void) const;

signals:
    void optionSelected (int which);

protected slots:
    virtual void setSelected (int which);

protected:
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void drawContents (QPainter *painter);

    void setInvertSelectedPixmap (bool yes = true) { m_invertSelectedPixmap = yes; }
    bool m_invertSelectedPixmap;
    
    QValueVector <QPixmap> m_pixmaps;
    QValueVector <QRect> m_pixmapRects;
    int m_y, m_x, m_highest;

    int m_selected;
};

#endif  // __kptoolwidgetbase_h__
