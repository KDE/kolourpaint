
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


#define DEBUG_KP_FLOOD_FILL 0


#include <kpfloodfill.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kpdefs.h>

#include <kppixmapfx.h>
#include <kptool.h>


kpFloodFill::kpFloodFill (QPixmap *pixmap, int x, int y,
                         const kpColor &color, int processedColorSimilarity)
    : m_pixmapPtr (pixmap), m_x (x), m_y (y),
      m_color (color), m_processedColorSimilarity (processedColorSimilarity),
      m_initState (0)
{
}

kpFloodFill::~kpFloodFill ()
{
}


// private
int kpFloodFill::fillLinesListSize (const QValueList <kpFloodFill::FillLine> &fillLines) const
{
    return (fillLines.size () * kpFloodFill::FillLine::size ());
}
    
// public
int kpFloodFill::size () const
{
    int fillLinesCacheSize = 0;
    for (QValueVector < QValueList <kpFloodFill::FillLine > >::const_iterator it = m_fillLinesCache.begin ();
         it != m_fillLinesCache.end ();
         it++)
    {
        fillLinesCacheSize += fillLinesListSize (*it);
    }
    
    return fillLinesListSize (m_fillLines) +
           kpPixmapFX::imageSize (m_image) +
           fillLinesCacheSize;
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

        QPainter painter, maskPainter;
        QBitmap maskBitmap;

        if (m_pixmapPtr->mask () || m_color.isTransparent ())
        {
            maskBitmap = kpPixmapFX::getNonNullMask (*m_pixmapPtr);
            maskPainter.begin (&maskBitmap);
            maskPainter.setPen (m_color.maskColor ());
        }

        if (m_color.isOpaque ())
        {
            painter.begin (m_pixmapPtr);
            painter.setPen (m_color.toQColor ());
        }

        const QValueList <FillLine>::ConstIterator fillLinesEnd = m_fillLines.end ();
        for (QValueList <FillLine>::ConstIterator it = m_fillLines.begin ();
             it != fillLinesEnd;
             it++)
        {
            QPoint p1 = QPoint ((*it).m_x1, (*it).m_y);
            QPoint p2 = QPoint ((*it).m_x2, (*it).m_y);

            if (painter.isActive ())
                painter.drawLine (p1, p2);

            if (maskPainter.isActive ())
                maskPainter.drawLine (p1, p2);
        }

        if (painter.isActive ())
            painter.end ();

        if (maskPainter.isActive ())
            maskPainter.end ();

        if (!maskBitmap.isNull ())
            m_pixmapPtr->setMask (maskBitmap);

        QApplication::restoreOverrideCursor ();
    }
    else
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kdDebug () << "kpFloodFill::fill() performing NOP fill" << endl;
    #endif
    }

    return true;
}

bool kpFloodFill::prepareColorToChange ()
{
#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "kpFloodFill::prepareColorToChange" << endl;
#endif

    m_colorToChange = kpPixmapFX::getColorAtPixel (*m_pixmapPtr, QPoint (m_x, m_y));

    if (m_colorToChange.isOpaque ())
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kdDebug () << "\tcolorToChange: r=" << m_colorToChange.red ()
                   << ", b=" << m_colorToChange.blue ()
                   << ", g=" << m_colorToChange.green ()
                   << endl;
    #endif
    }
    else
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kdDebug () << "\tcolorToChange: transparent" << endl;
    #endif
    }

    m_initState = 1;
    return true;
}

// Derived from the zSprite2 Graphics Engine

bool kpFloodFill::prepare ()
{
#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "kpFloodFill::prepare()" << endl;
#endif
    m_boundingRect = QRect ();

    if (m_initState < 1 && !prepareColorToChange ())
    {
        kdError () << "kpFloodFill:prepare() could not prepareColorToChange()!" << endl;
        return false;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "\tperforming NOP check" << endl;
#endif

    // get the color we need to replace
    if (m_processedColorSimilarity == 0 && m_color == m_colorToChange)
    {
        // need to do absolutely nothing (this is a significant optimisation
        // for people who randomly click a lot over already-filled areas)
        m_initState = 2;  // sync with all "return true"'s
        return true;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "\tconverting to image" << endl;
#endif

    // is this the only way to read pixels?
    m_image = kpPixmapFX::convertToImage (*m_pixmapPtr);
    if (m_image.isNull ())
    {
        kdError () << "kpFloodFill::prepare() could not convert to QImage" << endl;
        return false;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "\tcreating fillLinesCache" << endl;
#endif

    // ready cache
    m_fillLinesCache.resize (m_pixmapPtr->height ());

#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "\tcreating fill lines" << endl;
#endif

    // draw initial line
    addLine (m_y, findMinX (m_y, m_x), findMaxX (m_y, m_x));

    for (QValueList <FillLine>::ConstIterator it = m_fillLines.begin ();
         it != m_fillLines.end ();
         it++)
    {
    #if DEBUG_KP_FLOOD_FILL && 0
        kdDebug () << "Expanding from y=" << (*it).m_y
                   << " x1=" << (*it).m_x1
                   << " x2=" << (*it).m_x2
                   << endl;
    #endif

        // make more lines above and below current line
        findAndAddLines (*it, -1);
        findAndAddLines (*it, +1);
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kdDebug () << "\tfinalising memory usage" << endl;
#endif

    // finalize memory usage
    m_image.reset ();
    m_fillLinesCache.clear ();

    m_initState = 2;  // sync with all "return true"'s
    return true;
}

void kpFloodFill::addLine (int y, int x1, int x2)
{
#if DEBUG_KP_FLOOD_FILL && 0
    kdDebug () << "kpFillCommand::fillAddLine (" << y << "," << x1 << "," << x2 << ")" << endl;
#endif

    m_fillLines.append (FillLine (y, x1, x2));
    m_fillLinesCache [y].append (FillLine (y /* OPT */, x1, x2));
    m_boundingRect = m_boundingRect.unite (QRect (QPoint (x1, y), QPoint (x2, y)));
}

kpColor kpFloodFill::pixelColor (int x, int y, bool *beenHere) const
{
    if (beenHere)
        *beenHere = false;

    if (y >= (int) m_fillLinesCache.count ())
    {
        kdError () << "kpFloodFill::pixelColor("
                   << x << ","
                   << y << ") y out of range=" << m_pixmapPtr->height () << endl;
        return kpColor::invalid;
    }

    const QValueList <FillLine>::ConstIterator theEnd = m_fillLinesCache [y].end ();
    for (QValueList <FillLine>::ConstIterator it = m_fillLinesCache [y].begin ();
         it != theEnd;
         it++)
    {
        if (x >= (*it).m_x1 && x <= (*it).m_x2)
        {
            if (beenHere)
                *beenHere = true;
            return m_color;
        }
    }

    return kpPixmapFX::getColorAtPixel (m_image, QPoint (x, y));
}

bool kpFloodFill::shouldGoTo (int x, int y) const
{
    bool beenThere;
    const kpColor col = pixelColor (x, y, &beenThere);

    return (!beenThere && col.isSimilarTo (m_colorToChange, m_processedColorSimilarity));
}

void kpFloodFill::findAndAddLines (const FillLine &fillLine, int dy)
{
    // out of bounds?
    if (fillLine.m_y + dy < 0 || fillLine.m_y + dy >= m_pixmapPtr->height ())
        return;

    for (int xnow = fillLine.m_x1; xnow <= fillLine.m_x2; xnow++)
    {
        // At current position, right colour?
        if (shouldGoTo (xnow, fillLine.m_y + dy))
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
int kpFloodFill::findMinX (int y, int x) const
{
    for (;;)
    {
        if (x < 0)
            return 0;

        if (shouldGoTo (x, y))
            x--;
        else
            return x + 1;
    }
}

// finds the maximum x value at a certain line to be filled
int kpFloodFill::findMaxX (int y, int x) const
{
    for (;;)
    {
        if (x > m_pixmapPtr->width () - 1)
            return m_pixmapPtr->width () - 1;

        if (shouldGoTo (x, y))
            x++;
        else
            return x - 1;
    }
}
