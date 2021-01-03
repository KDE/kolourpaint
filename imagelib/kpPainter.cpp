
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


#define DEBUG_KP_PAINTER 0


#include "kpPainter.h"

#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"
#include "tools/flow/kpToolFlowBase.h"

#include <cstdio>

#include <QPainter>
#include <QPolygon>
#include <QRandomGenerator>

#include "kpLogCategories.h"

//---------------------------------------------------------------------

// public static
bool kpPainter::pointsAreCardinallyAdjacent (const QPoint &p, const QPoint &q)
{
    int dx = qAbs (p.x () - q.x ());
    int dy = qAbs (p.y () - q.y ());

    return (dx + dy == 1);
}

//---------------------------------------------------------------------

// public static
QList <QPoint> kpPainter::interpolatePoints (const QPoint &startPoint,
    const QPoint &endPoint,
    bool cardinalAdjacency,
    double probability)
{
#if DEBUG_KP_PAINTER
    qCDebug(kpLogImagelib) << "CALL(startPoint=" << startPoint
              << ",endPoint=" << endPoint << ")";
#endif

    QList <QPoint> ret;

    Q_ASSERT (probability >= 0.0 && probability <= 1.0);
    const int probabilityTimes1000 = qRound (probability * 1000);
#define SHOULD_DRAW()  ( (probabilityTimes1000 == 1000) /*avoid QRandomGenerator call*/ ||  \
                         (QRandomGenerator::global()->bounded(1000) < probabilityTimes1000) )


    // Derived from the zSprite2 Graphics Engine.
    // "MODIFIED" comment shows deviation from zSprite2 and Bresenham's line
    // algorithm.

    const int x1 = startPoint.x (),
        y1 = startPoint.y (),
        x2 = endPoint.x (),
        y2 = endPoint.y ();

    // Difference of x and y values
    const int dx = x2 - x1;
    const int dy = y2 - y1;

    // Absolute values of differences
    const int ix = qAbs (dx);
    const int iy = qAbs (dy);

    // Larger of the x and y differences
    const int inc = ix > iy ? ix : iy;

    // Plot location
    int plotx = x1;
    int ploty = y1;

    int x = 0;
    int y = 0;

    if (SHOULD_DRAW ()) {
        ret.append (QPoint (plotx, ploty));
    }


    for (int i = 0; i <= inc; i++)
    {
        // oldplotx is equally as valid but would look different
        // (but nobody will notice which one it is)
        const int oldploty = ploty;
        int plot = 0;

        x += ix;
        y += iy;

        if (x > inc)
        {
            plot++;
            x -= inc;

            if (dx < 0) {
                plotx--;
            }
            else {
                plotx++;
            }
        }

        if (y > inc)
        {
            plot++;
            y -= inc;

            if (dy < 0) {
                ploty--;
            }
            else {
                ploty++;
            }
        }

        if (plot)
        {
            if (cardinalAdjacency && plot == 2)
            {
                // MODIFIED: Every point is
                // horizontally or vertically adjacent to another point (if there
                // is more than 1 point, of course).  This is in contrast to the
                // ordinary line algorithm which can create diagonal adjacencies.

                if (SHOULD_DRAW ()) {
                    ret.append (QPoint (plotx, oldploty));
                }
            }

            if (SHOULD_DRAW ()) {
                ret.append (QPoint (plotx, ploty));
            }
        }
    }

#undef SHOULD_DRAW

    return ret;
}

//---------------------------------------------------------------------

// public static
void kpPainter::fillRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color)
{
    kpPixmapFX::fillRect (image, x, y, width, height, color);
}

//---------------------------------------------------------------------


// <rgbPainter> are operating on the original image
// (the original image is not passed to this function).
//
// <image> = subset of the original image containing all the pixels in
//           <imageRect>
// <drawRect> = the rectangle, relative to the painters, whose pixels we
//              want to change
static bool ReadableImageWashRect (QPainter *rgbPainter,
        const QImage &image,
        const kpColor &colorToReplace,
        const QRect &imageRect, const QRect &drawRect,
        int processedColorSimilarity)
{
    bool didSomething = false;

#if DEBUG_KP_PAINTER && 0
    qCDebug(kpLogImagelib) << "kppixmapfx.cpp:WashRect(imageRect=" << imageRect
               << ",drawRect=" << drawRect
               << ")" << endl;
#endif

    // If you're going to pass painter pointers, those painters had better be
    // active (i.e. QPainter::begin() has been called).
    Q_ASSERT (!rgbPainter || rgbPainter->isActive ());

// make use of scanline coherence
#define FLUSH_LINE()                                             \
{                                                                \
    if (rgbPainter) {                                            \
        if (startDrawX == x - 1)                                 \
            rgbPainter->drawPoint (startDrawX + imageRect.x (),  \
                y + imageRect.y ());                             \
        else                                                     \
            rgbPainter->drawLine (startDrawX + imageRect.x (),   \
                y + imageRect.y (),                              \
                x - 1 + imageRect.x (),                          \
                y + imageRect.y ());                             \
    }                                                            \
    didSomething = true;                                         \
    startDrawX = -1;                                             \
}

    const int maxY = drawRect.bottom () - imageRect.top ();

    const int minX = drawRect.left () - imageRect.left ();
    const int maxX = drawRect.right () - imageRect.left ();

    for (int y = drawRect.top () - imageRect.top ();
         y <= maxY;
         y++)
    {
        int startDrawX = -1;

        int x;  // for FLUSH_LINE()
        for (x = minX; x <= maxX; x++)
        {
        #if DEBUG_KP_PAINTER && 0
            fprintf (stderr, "y=%i x=%i colorAtPixel=%08X colorToReplace=%08X ... ",
                     y, x,
                     kpPixmapFX::getColorAtPixel (image, QPoint (x, y)).toQRgb (),
                     colorToReplace.toQRgb ());
        #endif
            if (kpPixmapFX::getColorAtPixel (image, QPoint (x, y)).isSimilarTo (colorToReplace, processedColorSimilarity))
            {
            #if DEBUG_KP_PAINTER && 0
                fprintf (stderr, "similar\n");
            #endif
                if (startDrawX < 0) {
                    startDrawX = x;
                }
            }
            else
            {
            #if DEBUG_KP_PAINTER && 0
                fprintf (stderr, "different\n");
            #endif
                if (startDrawX >= 0) {
                    FLUSH_LINE ();
                }
            }
        }

        if (startDrawX >= 0) {
            FLUSH_LINE ();
        }
    }

#undef FLUSH_LINE

    return didSomething;
}

//---------------------------------------------------------------------

struct WashPack
{
    QPoint startPoint, endPoint;
    kpColor color;
    int penWidth{}, penHeight{};
    kpColor colorToReplace;
    int processedColorSimilarity{};

    QRect readableImageRect;
    QImage readableImage;
};

//---------------------------------------------------------------------

static QRect Wash (kpImage *image,
        const QPoint &startPoint, const QPoint &endPoint,
        const kpColor &color, int penWidth, int penHeight,
        const kpColor &colorToReplace,
        int processedColorSimilarity,
        QRect (*drawFunc) (QPainter * /*rgbPainter*/, void * /*data*/))
{
    WashPack pack;
    pack.startPoint = startPoint; pack.endPoint = endPoint;
    pack.color = color;
    pack.penWidth = penWidth; pack.penHeight = penHeight;
    pack.colorToReplace = colorToReplace;
    pack.processedColorSimilarity = processedColorSimilarity;


    // Get the rectangle that bounds the changes and the pixmap for that
    // rectangle.
    const QRect normalizedRect = kpPainter::normalizedRect(pack.startPoint, pack.endPoint);
    pack.readableImageRect = kpTool::neededRect (normalizedRect,
        qMax (pack.penWidth, pack.penHeight));
#if DEBUG_KP_PAINTER
    qCDebug(kpLogImagelib) << "kppainter.cpp:Wash() startPoint=" << startPoint
              << " endPoint=" << endPoint
              << " --> normalizedRect=" << normalizedRect
              << " readableImageRect=" << pack.readableImageRect
              << endl;
#endif
    pack.readableImage = kpPixmapFX::getPixmapAt (*image, pack.readableImageRect);

    QPainter painter(image);
    return (*drawFunc)(&painter, &pack);
}

//---------------------------------------------------------------------

void WashHelperSetup (QPainter *rgbPainter, const WashPack *pack)
{
    // Set the drawing colors for the painters.

    if (rgbPainter) {
        rgbPainter->setPen (pack->color.toQColor());
    }
}

//---------------------------------------------------------------------

static QRect WashLineHelper (QPainter *rgbPainter, void *data)
{
#if DEBUG_KP_PAINTER && 0
    qCDebug(kpLogImagelib) << "Washing pixmap (w=" << rect.width ()
                << ",h=" << rect.height () << ")" << endl;
    QTime timer;
    int convAndWashTime;
#endif

    auto *pack = static_cast <WashPack *> (data);

    // Setup painters.
    ::WashHelperSetup (rgbPainter, pack);


    bool didSomething = false;

    QList <QPoint> points = kpPainter::interpolatePoints (pack->startPoint, pack->endPoint);
    foreach (const QPoint &p, points)
    {
        // OPT: This may be reading and possibly writing pixels that were
        //      visited on a previous iteration, since the pen is usually
        //      bigger than 1 pixel.  Maybe we could use QRegion to determine
        //      all the non-intersecting regions and only wash each region once.
        //
        //      Profiling needs to be done as QRegion is known to be a CPU hog.
        if (::ReadableImageWashRect (rgbPainter,
                pack->readableImage,
                pack->colorToReplace,
                pack->readableImageRect,
                kpToolFlowBase::hotRectForMousePointAndBrushWidthHeight (
                    p, pack->penWidth, pack->penHeight),
                pack->processedColorSimilarity))
        {
            didSomething = true;
        }
    }


#if DEBUG_KP_PAINTER && 0
    int ms = timer.restart ();
    qCDebug(kpLogImagelib) << "\ttried to wash: " << ms << "ms"
                << " (" << (ms ? (rect.width () * rect.height () / ms) : -1234)
                << " pixels/ms)"
                << endl;
    convAndWashTime += ms;
#endif


    // TODO: Rectangle may be too big.  Use QRect::united() incrementally?
    //       Efficiency?
    return didSomething ? pack->readableImageRect : QRect ();
}

//---------------------------------------------------------------------

// public static
QRect kpPainter::washLine (kpImage *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth, int penHeight,
        const kpColor &colorToReplace,
        int processedColorSimilarity)
{
    return ::Wash (image,
        QPoint (x1, y1), QPoint (x2, y2),
        color, penWidth, penHeight,
        colorToReplace,
        processedColorSimilarity,
        &::WashLineHelper);
}

//---------------------------------------------------------------------

static QRect WashRectHelper (QPainter *rgbPainter, void *data)
{
    auto *pack = static_cast <WashPack *> (data);
#if DEBUG_KP_PAINTER && 0
    qCDebug(kpLogImagelib) << "Washing pixmap (w=" << rect.width ()
                << ",h=" << rect.height () << ")" << endl;
    QTime timer;
    int convAndWashTime;
#endif

    // Setup painters.
    ::WashHelperSetup (rgbPainter, pack);


    const QRect drawRect (pack->startPoint, pack->endPoint);

    bool didSomething = false;

    if (::ReadableImageWashRect (rgbPainter,
            pack->readableImage,
            pack->colorToReplace,
            pack->readableImageRect,
            drawRect,
            pack->processedColorSimilarity))
    {
        didSomething = true;
    }


#if DEBUG_KP_PAINTER && 0
    int ms = timer.restart ();
    qCDebug(kpLogImagelib) << "\ttried to wash: " << ms << "ms"
                << " (" << (ms ? (rect.width () * rect.height () / ms) : -1234)
                << " pixels/ms)"
                << endl;
    convAndWashTime += ms;
#endif


    return didSomething ? drawRect : QRect ();
}

//---------------------------------------------------------------------

// public static
QRect kpPainter::washRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color,
        const kpColor &colorToReplace,
        int processedColorSimilarity)
{
    return ::Wash (image,
        QPoint (x, y), QPoint (x + width - 1, y + height - 1),
        color, 1/*pen width*/, 1/*pen height*/,
        colorToReplace,
        processedColorSimilarity,
        &::WashRectHelper);
}

//---------------------------------------------------------------------

// public static
void kpPainter::sprayPoints (kpImage *image,
        const QList <QPoint> &points,
        const kpColor &color,
        int spraycanSize)
{
#if DEBUG_KP_PAINTER
    qCDebug(kpLogImagelib) << "kpPainter::sprayPoints()";
#endif

    Q_ASSERT (spraycanSize > 0);

    QPainter painter(image);
    const int radius = spraycanSize / 2;

    // Set the drawing colors for the painters.

    painter.setPen(color.toQColor());

    for (const auto &p : points)
    {
        for (int i = 0; i < 10; i++)
        {
            const int dx = (QRandomGenerator::global()->generate() % spraycanSize) - radius;
            const int dy = (QRandomGenerator::global()->generate() % spraycanSize) - radius;

            // Make it look circular.
            // TODO: Can be done better by doing a random vector angle & length
            //       but would sin and cos be too slow?
            if ((dx * dx) + (dy * dy) > (radius * radius)) {
                continue;
            }

            const QPoint p2 (p.x () + dx, p.y () + dy);

            painter.drawPoint(p2);
        }
    }
}

//---------------------------------------------------------------------
