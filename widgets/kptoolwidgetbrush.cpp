
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

#include <qbitmap.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kptoolwidgetbrush.h>

/* sync: <brushes> */
static int brushSize [][3] =
{
    {9, 5, 1/*like Pen*/},
    {9, 5, 3},
    {9, 5, 3},
    {9, 5, 3}
};

kpToolWidgetBrush::kpToolWidgetBrush (QWidget *parent)
    : kpToolWidgetBase (parent)
{
    QPixmap *pixmap = m_brushPixmaps;
    
    for (int shape = 0; shape < 4; shape++)
    {
        for (int i = 0; i < 3; i++)
        {
            QPainter painter;
            pixmap->resize (44 / 3, 11);
            
            // HACK until layout of widgetbase is fixed
            if (i == 2)
                pixmap->resize (44 - 44 / 3 * 2, 11);

            const int s = brushSize [shape][i];
            const QRect rect = QRect ((pixmap->width () - s) / 2,
                                      (pixmap->height () - s) / 2,
                                      s,
                                      s);

            kdDebug () << "kpToolWidgetBrush::kpToolWidgetBrush() rect=" << rect << endl;

            pixmap->fill (Qt::white);
            painter.begin (pixmap);
            painter.setBrush (Qt::black);

            // sync: <brushes>
            switch (shape)
            {
            case 0:
                painter.drawEllipse (rect);
                break;
            case 1:
                painter.drawRect (rect);
                break;
            case 2:
                painter.drawLine (rect.topRight (), rect.bottomLeft ());
                break;
            case 3:
                painter.drawLine (rect.topLeft (), rect.bottomRight ());
                break;
            }
            painter.end ();

            pixmap->setMask (pixmap->createHeuristicMask ());
            kpToolWidgetBase::addOption (*pixmap, brushName (shape, i)/*tooltip*/);

            pixmap++;
        }
    }

    kpToolWidgetBase::setSelected (0);
}

kpToolWidgetBrush::~kpToolWidgetBrush ()
{
}


// private
QString kpToolWidgetBrush::brushName (int shape, int whichSize)
{
    int s = brushSize [shape][whichSize];
    
    if (s == 1)
        return i18n ("1x1");
    
    QString shapeName;

    // sync: <brushes>
    switch (shape)
    {
    case 0:
        shapeName = i18n ("Ellipse");
        break;
    case 1:
        shapeName = i18n ("Square");
        break;
    case 2:
        // TODO: is this really the name of a shape? :)
        shapeName = i18n ("Slash");
        break;
    case 3:
        // TODO: is this really the name of a shape? :)
        shapeName = i18n ("Backslash");
        break;
    }
    
    if (shapeName.isEmpty ())
        return QString::null;
    
    return i18n ("%1x%2 %3").arg (s).arg (s).arg (shapeName);
}

QPixmap kpToolWidgetBrush::brush () const
{
    return m_brushPixmaps [kpToolWidgetBase::selected ()];
}

bool kpToolWidgetBrush::brushIsDiagonalLine () const
{
    // sync: <brushes>
    return kpToolWidgetBase::selected () >= 2 * 3;
}

// virtual protected slot
void kpToolWidgetBrush::setSelected (int which)
{
    kpToolWidgetBase::setSelected (which);
    emit brushChanged (brush (), brushIsDiagonalLine ());
};

#include <kptoolwidgetbrush.moc>
