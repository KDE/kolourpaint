
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

#include <qapplication.h>  // DEP: for setOverrideCursor
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kpdefs.h>

#include <kpfloodfill.h>


kpFloodFill::kpFloodFill (QPixmap *pixmap, int x, int y, const QColor &color)
    : m_pixmapPtr (pixmap), m_x (x), m_y (y), m_color (color),
      m_initState (0)
{
}

kpFloodFill::~kpFloodFill ()
{
}

QRect kpFloodFill::boundingRect () const
{
    return m_boundingRect;
}

bool kpFloodFill::fill ()
{
    if (m_initState < 2 && !prepare ())
    {
        kdError () << "kpFloodFill:fill() could not prepare()!" << endl;
        return false;
    }

    // not trying to do a NOP fill
    if (m_boundingRect.isValid ())
    {
        QApplication::setOverrideCursor (Qt::waitCursor);

        QPainter painter;
        painter.begin (m_pixmapPtr);
        painter.setPen (m_color);
        for (QValueList <FillLine>::ConstIterator it = m_fillLines.begin ();
             it != m_fillLines.end ();
             it++)
        {
            painter.drawLine (QPoint ((*it).m_x1, (*it).m_y),
                              QPoint ((*it).m_x2, (*it).m_y));
        }
        painter.end ();

        QApplication::restoreOverrideCursor ();
    }
    else
        kdDebug () << "kpFloodFill::fill() performing NOP fill" << endl;

    return true;
}

bool kpFloodFill::prepareColorToChange ()
{
    kdDebug () << "kpFloodFill::prepareColorToChange" << endl;

    QPixmap pixmap (1, 1);
    QPainter painter;
    painter.begin (&pixmap);
    painter.drawPixmap (QPoint (0, 0), *m_pixmapPtr, QRect (m_x, m_y, 1, 1));
    painter.end ();
    QImage image = pixmap.convertToImage ();
    if (image.isNull ())
    {
        kdError () << "kpFloodFill::prepare() could not convert to QImage" << endl;
        return false;
    }

    m_colorToChange = image.pixel (0, 0);

    kdDebug () << "\tcolorToChange: r=" << qRed (m_colorToChange)
                      << ", b=" << qBlue (m_colorToChange)
                      << ", g=" << qGreen (m_colorToChange)
                      << endl;
    m_initState = 1;
    return true;
}

// Derived from the zSprite2 Graphics Engine

bool kpFloodFill::prepare ()
{
    kdDebug () << "kpFloodFill::prepare()" << endl;
    m_boundingRect = QRect ();

    if (m_initState < 1 && !prepareColorToChange ())
    {
        kdError () << "kpFloodFill:prepare() could not prepareColorToChange()!" << endl;
        return false;
    }

    kdDebug () << "\tperforming NOP check" << endl;

    // get the color we need to replace
    if (m_colorToChange == m_color.rgb ())
    {
        // need to do absolutely nothing (this is a significant optimisation
        // for people who randomly click a lot over already-filled areas)
        m_initState = 2;  // sync with all "return true"'s
        return true;
    }

    kdDebug () << "\tconverting to image" << endl;

    // is this the only way to read pixels?
    m_image = m_pixmapPtr->convertToImage ();
    if (m_image.isNull ())
    {
        kdError () << "kpFloodFill::prepare() could not convert to QImage" << endl;
        return false;
    }

    kdDebug () << "\tcreating fillLinesCache" << endl;

    // ready cache
    m_fillLinesCache.resize (m_pixmapPtr->height ());

    kdDebug () << "\tcreating fill lines" << endl;

    // draw initial line
    addLine (m_y, findMinX (m_y, m_x), findMaxX (m_y, m_x));

    for (QValueList <FillLine>::ConstIterator it = m_fillLines.begin ();
         it != m_fillLines.end ();
         it++)
    {
    #if 0
        kdDebug () << "Expanding from y=" << (*it).m_y
                          << " x1=" << (*it).m_x1
                          << " x2=" << (*it).m_x2
                          << endl;
    #endif

        // make more lines above and below current line
        findAndAddLines (*it, -1);
        findAndAddLines (*it, +1);
    }

    kdDebug () << "\tfinalising memory usage" << endl;

    // finalize memory usage
    m_image.reset ();
    m_fillLinesCache.clear ();

    m_initState = 2;  // sync with all "return true"'s
    return true;
}

void kpFloodFill::addLine (int y, int x1, int x2)
{
#if 0
    kdDebug () << "kpFillCommand::fillAddLine (" << y << "," << x1 << "," << x2 << ")" << endl;
#endif

    m_fillLines.append (FillLine (y, x1, x2));
    m_fillLinesCache [y].append (FillLine (y /* OPT */, x1, x2));
    m_boundingRect = m_boundingRect.unite (QRect (QPoint (x1, y), QPoint (x2, y)));
}

QRgb kpFloodFill::pixelColor (int x, int y)
{
    if (y >= (int) m_fillLinesCache.count ())
    {
        kdError () << "kpFloodFill::pixelColor("
                          << x << ","
                          << y << ") y out of range=" << m_pixmapPtr->height () << endl;
        return QRgb ();
    }

    for (QValueList <FillLine>::ConstIterator it = m_fillLinesCache [y].begin ();
         it != m_fillLinesCache [y].end ();
         it++)
    {
        if (x >= (*it).m_x1 && x <= (*it).m_x2)
            return m_color.rgb ();
    }

    return m_image.pixel (x, y);
}

void kpFloodFill::findAndAddLines (const FillLine &fillLine, int dy)
{
    // out of bounds?
    if (fillLine.m_y + dy < 0 || fillLine.m_y + dy >= m_pixmapPtr->height ())
        return;

    for (int xnow = fillLine.m_x1; xnow <= fillLine.m_x2; xnow++)
    {
        // At current position, right colour?
        if (pixelColor (xnow, fillLine.m_y + dy) == m_colorToChange)
        {
            // Find minimum and maximum x values
            int minxnow = findMinX (fillLine.m_y + dy, xnow);
            int maxxnow = findMaxX (fillLine.m_y + dy, xnow);

            // Draw line
            addLine (fillLine.m_y + dy, minxnow, maxxnow);

            // Move x pointer
            xnow = maxxnow;
        }
    }
}

// finds the minimum x value at a certain line to be filled
int kpFloodFill::findMinX (int y, int x)
{
    for (;;)
    {
        if (x < 0)
            return 0;

        if (pixelColor (x, y) == m_colorToChange)
            x--;
        else
            return x + 1;
    }
}

// finds the maximum x value at a certain line to be filled
int kpFloodFill::findMaxX (int y, int x)
{
    for (;;)
    {
        if (x > m_pixmapPtr->width () - 1)
            return m_pixmapPtr->width () - 1;

        if (pixelColor (x, y) == m_colorToChange)
            x++;
        else
            return x - 1;
    }
}
