
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#include "kpFloodFill.h"

#include <QApplication>
#include <QImage>
#include <QList>
#include <QPainter>

#include "kpLogCategories.h"

#include "kpColor.h"
#include "kpDefs.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"

//---------------------------------------------------------------------

class kpFillLine
{
public:
    kpFillLine (int y = -1, int x1 = -1, int x2 = -1)
        : m_y (y), m_x1 (x1), m_x2 (x2)
    {
    }

    static kpCommandSize::SizeType size ()
    {
        return sizeof (kpFillLine);
    }

    int m_y, m_x1, m_x2;
};

//---------------------------------------------------------------------

static kpCommandSize::SizeType FillLinesListSize (const QList <kpFillLine> &fillLines)
{
    return (fillLines.size () * kpFillLine::size ());
}

//---------------------------------------------------------------------

struct kpFloodFillPrivate
{
    //
    // Copy of whatever was passed to the constructor.
    //

    kpImage *imagePtr = nullptr;
    int x = 0, y = 0;
    kpColor color;
    int processedColorSimilarity = 0;


    //
    // Set by Step 1.
    //

    kpColor colorToChange;


    //
    // Set by Step 2.
    //

    QList <kpFillLine> fillLines;
    QList < QList <kpFillLine> > fillLinesCache;

    QRect boundingRect;

    bool prepared = false;
};

//---------------------------------------------------------------------

kpFloodFill::kpFloodFill (kpImage *image, int x, int y,
                         const kpColor &color, int processedColorSimilarity)
    : d (new kpFloodFillPrivate ())
{
    d->imagePtr = image;
    d->x = x;
    d->y = y;
    d->color = color;
    d->processedColorSimilarity = processedColorSimilarity;

    d->prepared = false;
}

//---------------------------------------------------------------------

kpFloodFill::~kpFloodFill ()
{
    delete d;
}

//---------------------------------------------------------------------

// public
kpColor kpFloodFill::color () const
{
    return d->color;
}

//---------------------------------------------------------------------

// public
int kpFloodFill::processedColorSimilarity () const
{
    return d->processedColorSimilarity;
}

//---------------------------------------------------------------------

// public
kpCommandSize::SizeType kpFloodFill::size () const
{
    kpCommandSize::SizeType fillLinesCacheSize = 0;
    for (const auto &linesList : d->fillLinesCache)
    {
        fillLinesCacheSize += ::FillLinesListSize (linesList);
    }

    return ::FillLinesListSize(d->fillLines) +
           kpCommandSize::QImageSize(d->imagePtr) +
           fillLinesCacheSize;
}

//---------------------------------------------------------------------

// public
void kpFloodFill::prepareColorToChange ()
{
    if (d->colorToChange.isValid ()) {
        return;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    qCDebug(kpLogImagelib) << "kpFloodFill::prepareColorToChange()";
#endif

    d->colorToChange = kpPixmapFX::getColorAtPixel (*d->imagePtr, QPoint (d->x, d->y));
}

//---------------------------------------------------------------------

// public
kpColor kpFloodFill::colorToChange ()
{
    prepareColorToChange ();

    return d->colorToChange;
}

//---------------------------------------------------------------------

// Derived from the zSprite2 Graphics Engine

// private
kpColor kpFloodFill::pixelColor (int x, int y, bool *beenHere) const
{
    if (beenHere) {
        *beenHere = false;
    }

    Q_ASSERT (y >= 0 && y < static_cast<int> (d->fillLinesCache.count ()));

    for (const auto &line : d->fillLinesCache [y])
    {
        if (x >= line.m_x1 && x <= line.m_x2)
        {
            if (beenHere) {
                *beenHere = true;
            }
            return d->color;
        }
    }

    return kpPixmapFX::getColorAtPixel (*(d->imagePtr), QPoint (x, y));
}

//---------------------------------------------------------------------

// private
bool kpFloodFill::shouldGoTo (int x, int y) const
{
    bool beenThere;
    const kpColor col = pixelColor (x, y, &beenThere);

    return (!beenThere && col.isSimilarTo (d->colorToChange, d->processedColorSimilarity));
}

//---------------------------------------------------------------------

// private
int kpFloodFill::findMinX (int y, int x) const
{
    for (;;)
    {
        if (x < 0) {
            return 0;
        }

        if (shouldGoTo (x, y)) {
            x--;
        }
        else {
            return x + 1;
        }
    }
}

//---------------------------------------------------------------------

// private
int kpFloodFill::findMaxX (int y, int x) const
{
    for (;;)
    {
        if (x > d->imagePtr->width () - 1) {
            return d->imagePtr->width () - 1;
        }

        if (shouldGoTo (x, y)) {
            x++;
        }
        else {
            return x - 1;
        }
    }
}

//---------------------------------------------------------------------

// private
void kpFloodFill::addLine (int y, int x1, int x2)
{
#if DEBUG_KP_FLOOD_FILL && 0
    qCDebug(kpLogImagelib) << "kpFillCommand::fillAddLine ("
              << y << "," << x1 << "," << x2 << ")" << endl;
#endif

    d->fillLines.append (kpFillLine (y, x1, x2));
    d->fillLinesCache [y].append (
        kpFillLine (y/*OPT: can determine from array index*/, x1, x2));
    d->boundingRect = d->boundingRect.united (QRect (QPoint (x1, y), QPoint (x2, y)));
}

//---------------------------------------------------------------------

// private
void kpFloodFill::findAndAddLines (const kpFillLine &fillLine, int dy)
{
    // out of bounds?
    if (fillLine.m_y + dy < 0 || fillLine.m_y + dy >= d->imagePtr->height ()) {
        return;
    }

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

//---------------------------------------------------------------------

// public
void kpFloodFill::prepare ()
{
    if (d->prepared) {
        return;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    qCDebug(kpLogImagelib) << "kpFloodFill::prepare()";
#endif

    prepareColorToChange ();

    d->boundingRect = QRect ();


#if DEBUG_KP_FLOOD_FILL && 1
    qCDebug(kpLogImagelib) << "\tperforming NOP check";
#endif

    // get the color we need to replace
    if (d->processedColorSimilarity == 0 && d->color == d->colorToChange)
    {
        // need to do absolutely nothing (this is a significant optimization
        // for people who randomly click a lot over already-filled areas)
        d->prepared = true;  // sync with all "return true"'s
        return;
    }

#if DEBUG_KP_FLOOD_FILL && 1
    qCDebug(kpLogImagelib) << "\tcreating fillLinesCache";
#endif

    // ready cache
    for (int i = 0; i < d->imagePtr->height (); i++) {
         d->fillLinesCache.append (QList <kpFillLine> ());
    }

#if DEBUG_KP_FLOOD_FILL && 1
    qCDebug(kpLogImagelib) << "\tcreating fill lines";
#endif

    // draw initial line
    addLine (d->y, findMinX (d->y, d->x), findMaxX (d->y, d->x));

    for (int i = 0; i < d->fillLines.count(); i++)
    {
      kpFillLine &fl = d->fillLines[i];

    #if DEBUG_KP_FLOOD_FILL && 0
        qCDebug(kpLogImagelib) << "Expanding from y=" << fl.m_y
                   << " x1=" << fl.m_x1
                   << " x2=" << fl.m_x2
                   << endl;
    #endif

        //
        // Make more lines above and below current line.
        //
        // WARNING: Adds to end of "fillLines" (the list we are iterating through).
        findAndAddLines(fl, -1);
        findAndAddLines(fl, +1);
    }

#if DEBUG_KP_FLOOD_FILL && 1
    qCDebug(kpLogImagelib) << "\tfinalising memory usage";
#endif

    // finalize memory usage
    d->fillLinesCache.clear ();

    d->prepared = true;  // sync with all "return true"'s
}

//---------------------------------------------------------------------

// public
QRect kpFloodFill::boundingRect ()
{
    prepare ();

    return d->boundingRect;
}

//---------------------------------------------------------------------
// public
void kpFloodFill::fill()
{
    prepare();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QPainter painter(d->imagePtr);

    // by definition, flood fill with a fully transparent color erases the pixels
    // and sets them to be fully transparent
    if ( d->color.isTransparent() ) {
      painter.setCompositionMode(QPainter::CompositionMode_Clear);
    }

    painter.setPen(d->color.toQColor());

    for (const auto &l : d->fillLines)
    {
      if ( l.m_x1 == l.m_x2 ) {
        painter.drawPoint(l.m_x1, l.m_y);
      }
      else {
        painter.drawLine(l.m_x1, l.m_y, l.m_x2, l.m_y);
      }
    }

    QApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------
