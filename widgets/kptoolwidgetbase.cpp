
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
#include <qcolor.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kptoolwidgetbase.h>

kpToolWidgetBase::kpToolWidgetBase (QWidget *parent)
    : QFrame (parent),
      m_invertSelectedPixmap (true),
      m_y (0), m_x (0), m_highest (0), m_selected (-1)
{
    setFrameStyle (QFrame::Panel | QFrame::Sunken);
    setFixedSize (44, 66);
}

int kpToolWidgetBase::addOption (const QPixmap &pixmap, const QString &toolTip,
                                 bool center, bool doUpdate)
{
    int n = m_pixmaps.count ();

    m_pixmaps.append (pixmap);
    
    if (center)
    {
        m_x = (width () - pixmap.width ()) / 2;
        if (m_x < 0)
            m_x = 0;
        m_y += m_highest;
        m_highest = 0;
    }
    
    m_pixmapRects.append (QRect (m_x, m_y, pixmap.width (), pixmap.height ()));
    if (!toolTip.isEmpty ())
        QToolTip::add (this, m_pixmapRects [n], toolTip);

    kdDebug () << "kpToolWidgetBase::addOption(): m_x=" << m_x
               << " m_y=" << m_y
               << " width=" << pixmap.width ()
               << " height=" << pixmap.height ()
               << endl;

    if (pixmap.height () > m_highest)
        m_highest = pixmap.height ();
    
    m_x += pixmap.width ();
    if (m_x >= width ())
    {
        m_x = 0;
        m_y += m_highest;
        m_highest = 0;
    }

    if (doUpdate)
        update (m_pixmapRects [n]);

    return n;
}

int kpToolWidgetBase::selected (void) const
{
    return m_selected;
}

// protected slot
void kpToolWidgetBase::setSelected (int which)
{
    if (which == m_selected)
        return;

    const int wasSelected = m_selected;

    m_selected = which;

    if (wasSelected >= 0)
        update (m_pixmapRects [wasSelected]);
    update (m_pixmapRects [which]);

    emit optionSelected (which);
}

// virtual protected
void kpToolWidgetBase::mousePressEvent (QMouseEvent *e)
{
    const int numPixmaps = m_pixmapRects.count ();
    for (int i = 0; i < numPixmaps; i++)
    {
        if (m_pixmapRects [i].contains (e->pos ()))
        {
            setSelected (i);
            e->accept ();
            return;
        }
    }

    e->ignore ();
}

// virtual protected
void kpToolWidgetBase::drawContents (QPainter *painter)
{
    kdDebug () << "kpToolWidgetBase::drawContents(): rect=" << contentsRect () << endl;

    const int numPixmaps = m_pixmaps.count ();
    for (int i = 0; i < numPixmaps; i++)
    {
        if (i != m_selected)
        {
            painter->drawPixmap (m_pixmapRects [i].topLeft (), m_pixmaps [i]);
        }
        else
        {
            painter->setBrush (QColor (0, 128, 255));  // 96, 192 also looks good
            /*for (int y2 = y; y2 < y + (*it).height (); y2++)
            {
                for (int x2 = x + (y2 % 2);
                     x2 < x + ((*it).width ());
                     x2 += 2)
                    painter->drawPoint (x2, y2);
            }*/
            
            painter->drawRect (m_pixmapRects [i]);

            QPixmap pixmap = m_pixmaps [i];
                
            if (m_invertSelectedPixmap)
            {
                QImage image = pixmap.convertToImage ();
                image.invertPixels ();
                pixmap.convertFromImage (image);
            }
            
            painter->drawPixmap (m_pixmapRects [i].topLeft (), pixmap);
        }
    }
}

kpToolWidgetBase::~kpToolWidgetBase ()
{
}

#include <kptoolwidgetbase.moc>
