
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_FLOOD_FILL 1


#include <kpfloodfill.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kpimage.h>
#include <kppixmapfx.h>
#include <kptool.h>


kpFloodFill::kpFloodFill (QPixmap *pixmap, int x, int y,
                         const kpColor &color, int processedColorSimilarity)
    : m_pixmapPtr (pixmap), m_x (x), m_y (y),
      m_color (color), m_processedColorSimilarity (processedColorSimilarity)
{
    m_prepared = false;
}

kpFloodFill::~kpFloodFill ()
{
}


// private
int kpFloodFill::fillLinesListSize (const QLinkedList <kpFloodFill::FillLine> &fillLines) const
{
    return (fillLines.size () * kpFloodFill::FillLine::size ());
}
    
// public
int kpFloodFill::size () const
{
    int fillLinesCacheSize = 0;
    for (QList < QLinkedList <kpFloodFill::FillLine > >::const_iterator it = m_fillLinesCache.begin ();
         it != m_fillLinesCache.end ();
         it++)
    {
        fillLinesCacheSize += fillLinesListSize (*it);
    }
    
    return fillLinesListSize (m_fillLines) +
           kpPixmapFX::imageSize (m_image) +
           fillLinesCacheSize;
}
    
// public
kpColor kpFloodFill::color () const
{
    return m_color;
}

// public
int kpFloodFill::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}


QRect kpFloodFill::boundingRect ()
{
    prepare ();
    
    return m_boundingRect;
}




struct DrawLinesPackage
{
    const QLinkedList <kpFloodFill::FillLine> *lines;
    kpColor color;
};

static void DrawLinesHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    const DrawLinesPackage *pack = static_cast <DrawLinesPackage *> (data);

#if DEBUG_KP_FLOOD_FILL
    kDebug () << "DrawLinesHelper() lines"
        << " color=" << (int *) pack->color.toQRgb ()
        << endl;
#endif

    p->setPen (kpPixmapFX::draw_ToQColor (pack->color, drawingOnRGBLayer));
            
    foreach (const kpFloodFill::FillLine l, *pack->lines)
    {
        const QPoint p1 (l.m_x1, l.m_y);
        const QPoint p2 (l.m_x2, l.m_y);

        p->drawLine (p1, p2);
    }
}

static void DrawLines (kpImage *image,
        const QLinkedList <kpFloodFill::FillLine> &lines,
        const kpColor &color)
{
    DrawLinesPackage pack;
    pack.lines = &lines;
    pack.color = color;

    kpPixmapFX::draw (image, &::DrawLinesHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}

void kpFloodFill::fill ()
{
    prepare ();


    QApplication::setOverrideCursor (Qt::WaitCursor);

    ::DrawLines (m_pixmapPtr, m_fillLines, m_color);

    QApplication::restoreOverrideCursor ();
}

kpColor kpFloodFill::colorToChange ()
{
    prepareColorToChange ();
    
    return m_colorToChange;
}

void kpFloodFill::prepareColorToChange ()
{
#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "kpFloodFill::prepareColorToChange" << endl;
#endif
    if (m_colorToChange.isValid ())
        return;

    m_colorToChange = kpPixmapFX::getColorAtPixel (*m_pixmapPtr, QPoint (m_x, m_y));

    if (m_colorToChange.isOpaque ())
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kDebug () << "\tcolorToChange: r=" << m_colorToChange.red ()
                   << ", b=" << m_colorToChange.blue ()
                   << ", g=" << m_colorToChange.green ()
                   << endl;
    #endif
    }
    else
    {
    #if DEBUG_KP_FLOOD_FILL && 1
        kDebug () << "\tcolorToChange: transparent" << endl;
    #endif
    }
}

// Derived from the zSprite2 Graphics Engine

void kpFloodFill::prepare ()
{
#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "kpFloodFill::prepare()" << endl;
#endif
    if (m_prepared)
        return;
        
    prepareColorToChange ();
        
    m_boundingRect = QRect ();


#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tperforming NOP check" << endl;
#endif

    // get the color we need to replace
    if (m_processedColorSimilarity == 0 && m_color == m_colorToChange)
    {
        // need to do absolutely nothing (this is a significant optimisation
        // for people who randomly click a lot over already-filled areas)
        m_prepared = true;  // sync with all "return true"'s
        return;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tconverting to image" << endl;
#endif

    // The only way to read pixels.  Sigh.
    m_image = kpPixmapFX::convertToImage (*m_pixmapPtr);

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tcreating fillLinesCache" << endl;
#endif

    // ready cache
    for (int i = 0; i < m_pixmapPtr->height (); i++)
         m_fillLinesCache.append (QLinkedList <FillLine> ());

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tcreating fill lines" << endl;
#endif

    // draw initial line
    addLine (m_y, findMinX (m_y, m_x), findMaxX (m_y, m_x));

    for (QLinkedList <FillLine>::ConstIterator it = m_fillLines.begin ();
         it != m_fillLines.end ();
         it++)
    {
    #if DEBUG_KP_FLOOD_FILL && 0
        kDebug () << "Expanding from y=" << (*it).m_y
                   << " x1=" << (*it).m_x1
                   << " x2=" << (*it).m_x2
                   << endl;
    #endif

        // make more lines above and below current line
        findAndAddLines (*it, -1);
        findAndAddLines (*it, +1);
    }

#if DEBUG_KP_FLOOD_FILL && 1
    kDebug () << "\tfinalising memory usage" << endl;
#endif

    // finalize memory usage
    m_image = QImage ();
    m_fillLinesCache.clear ();

    m_prepared = true;  // sync with all "return true"'s
}

void kpFloodFill::addLine (int y, int x1, int x2)
{
#if DEBUG_KP_FLOOD_FILL && 0
    kDebug () << "kpFillCommand::fillAddLine (" << y << "," << x1 << "," << x2 << ")" << endl;
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
        kError () << "kpFloodFill::pixelColor("
                   << x << ","
                   << y << ") y out of range=" << m_pixmapPtr->height () << endl;
        return kpColor::invalid;
    }

    const QLinkedList <FillLine>::ConstIterator theEnd = m_fillLinesCache [y].end ();
    for (QLinkedList <FillLine>::ConstIterator it = m_fillLinesCache [y].begin ();
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
