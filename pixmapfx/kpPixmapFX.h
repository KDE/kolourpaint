
// REFACTOR: Split this class into one for each distinct functionality category
//           (e.g. effects, mask operations)?

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


#ifndef KP_PIXMAP_FX_H
#define KP_PIXMAP_FX_H


#include <QRect>
#include <QString>

#include <KLocalizedString>

#include "imagelib/kpColor.h"


class QColor;
class QImage;
class QTransform;
class QPen;
class QImage;
class QPoint;
class QPolygon;
class QRect;




//
// QPixmap (view) Manipulation.
//
// Anything that is supposed to be manipulating the document contents
// (i.e. kpImage), should be moved to kpPainter.
//
// kpPainter uses us for its Qt backend but we don't use kpPainter.
// TODO: We should not use kpColor nor kpImage for the same reason.
//
class kpPixmapFX
{
//
// Get/Set Parts of Pixmap
//

public:

    //
    // Returns the pixel and mask data found at the <rect> in <pm>.
    //
    static QImage getPixmapAt (const QImage &pm, const QRect &rect);

    //
    // Sets the pixel and mask data at <destRect> in <*destPixmapPtr>
    // to <srcPixmap>.  Neither <destRect>'s width nor height are allowed
    // to be bigger than <srcPixmap>'s (you can't copy more than you have).
    // On the other hand, you can copy less than the size of <srcPixmap>
    // - no scaling is done.
    //
    static void setPixmapAt (QImage *destPixmapPtr, const QRect &destRect,
                             const QImage &srcPixmap);

    //
    // Sets the pixel and mask data at the rectangle in <*destPixmapPtr>,
    // with the top-left <destAt> and dimensions <srcPixmap.rect()>,
    // to <srcPixmap>.
    //
    static void setPixmapAt (QImage *destPixmapPtr, const QPoint &destAt,
                             const QImage &srcPixmap);
    static void setPixmapAt (QImage *destPixmapPtr, int destX, int destY,
                             const QImage &srcPixmap);

    //
    // Draws <srcPixmap> on top of <*destPixmapPtr> at <destAt>.
    // The mask of <*destPixmapPtr> is adjusted so that all opaque
    // pixels in <srcPixmap> will be opaque in <*destPixmapPtr>.
    //
    static void paintPixmapAt (QImage *destPixmapPtr, const QPoint &destAt,
                               const QImage &srcPixmap);
    static void paintPixmapAt (QImage *destPixmapPtr, int destX, int destY,
                               const QImage &srcPixmap);

    //
    // Returns the colour of the pixel at <at> in <pm>.
    // If the pixel is transparent, a value is returned such that
    // kpTool::isColorTransparent(<return_value>) will return true.
    //
    static kpColor getColorAtPixel (const QImage &pm, const QPoint &at);
    static kpColor getColorAtPixel (const QImage &pm, int x, int y);

//
// Transforms
//

public:

    //
    // Resizes an image to the given width and height,
    // filling any new areas with <backgroundColor>.
    //
    static void resize (QImage *destPtr, int w, int h,
                        const kpColor &backgroundColor);
    static QImage resize (const QImage &pm, int w, int h,
                           const kpColor &backgroundColor);

    //
    // Scales an image to the given width and height.
    // If <pretty> is true, a smooth scale will be used.
    //
    static void scale (QImage *destPtr, int w, int h, bool pretty = false);
    static QImage scale (const QImage &pm, int w, int h, bool pretty = false);


    // The minimum difference between 2 angles (in degrees) such that they are
    // considered different.  This gives you at least enough precision to
    // rotate an image whose width <= 10000 such that its height increases
    // by just 1 (and similarly with height <= 10000 and width).
    //
    // Currently used for skew & rotate operations.
    static const double AngleInDegreesEpsilon;


    //
    // Skews an image.
    //
    // <hangle>             horizontal angle clockwise (-90 < x < 90)
    // <vangle>             vertical angle clockwise (-90 < x < 90)
    // <backgroundColor>    color to fill new areas with
    // <targetWidth>        if > 0, the desired width of the resultant pixmap
    // <targetHeight>       if > 0, the desired height of the resultant pixmap
    //
    // Using <targetWidth> & <targetHeight> to generate preview pixmaps is
    // significantly more efficient than skewing and then scaling yourself.
    //
    static QTransform skewMatrix (int width, int height, double hangle, double vangle);
    static QTransform skewMatrix (const QImage &pixmap, double hangle, double vangle);

    static void skew (QImage *destPixmapPtr, double hangle, double vangle,
                      const kpColor &backgroundColor,
                      int targetWidth = -1, int targetHeight = -1);
    static QImage skew (const QImage &pm, double hangle, double vangle,
                         const kpColor &backgroundColor,
                         int targetWidth = -1, int targetHeight = -1);

    //
    // Rotates an image.
    //
    // <angle>              clockwise angle to rotate by
    // <backgroundColor>    color to fill new areas with
    // <targetWidth>        if > 0, the desired width of the resultant pixmap
    // <targetHeight>       if > 0, the desired height of the resultant pixmap
    //
    // Using <targetWidth> & <targetHeight> to generate preview pixmaps is
    // significantly more efficient than rotating and then scaling yourself.
    //
    static QTransform rotateMatrix (int width, int height, double angle);
    static QTransform rotateMatrix (const QImage &pixmap, double angle);

    static bool isLosslessRotation (double angle);

    static void rotate (QImage *destPixmapPtr, double angle,
                        const kpColor &backgroundColor,
                        int targetWidth = -1, int targetHeight = -1);
    static QImage rotate (const QImage &pm, double angle,
                           const kpColor &backgroundColor,
                           int targetWidth = -1, int targetHeight = -1);

//
// Drawing Shapes
//

public:

    // Returns a pen suitable for drawing a rectangle with 90 degree
    // corners ("MiterJoin").  This is necessary since Qt4 defaults to
    // "BevelJoin".  <qtWidth> is passed straight to QPen without modification.
    static QPen QPainterDrawRectPen (const QColor &color, int qtWidth);

    // Returns a pen suitable for drawing lines / polylines / polygons /
    // curves with rounded corners.  This is necessary since Qt4 defaults
    // to square corners ("SquareCap") and "BevelJoin".
    // <qtWidth> is passed straight to QPen without modification.
    static QPen QPainterDrawLinePen (const QColor &color, int qtWidth);

    static bool Only1PixelInPointArray(const QPolygon &points);

    // Draws a line from (x1,y1) to (x2,y2) onto <image>, with <color>
    // and <width>.  The corners are rounded and centred at those
    // coordinates so if <width> > 1, the line is likely to extend past
    // a rectangle with those corners.
    //
    // If <stippleColor> is valid, it draws a stippled line alternating
    // between long strips of <color> and short strips of <stippleColor>.
    static void drawPolyline (QImage *image,
        const QPolygon &points,
        const kpColor &color, int penWidth,
        const kpColor &stippleColor = kpColor::Invalid);

    // <isFinal> = shape completed else drawing but haven't finalised.
    // If not <isFinal>, the edge that would form the closure, if the
    // shape were finalised now, is highlighted specially.
    //
    // Odd-even fill.
    static void drawPolygon (QImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor = kpColor::Invalid,
        bool isFinal = true,
        const kpColor &fStippleColor = kpColor::Invalid);

    static void fillRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &color,
        const kpColor &stippleColor = kpColor::Invalid);

    static void drawStippleRect(QImage *image,
            int x, int y, int width, int height,
            const kpColor &fcolor,
            const kpColor &fStippleColor);

};


#endif  // KP_PIXMAP_FX_H
