
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


#include <qrect.h>
#include <qstring.h>

#include <klocalizedstring.h>

#include <kpColor.h>


class QBitmap;
class QColor;
class QImage;
class QPixmap;
class QMatrix;
class QPainter;
class QPen;
class QImage;
class QPoint;
class QPolygon;
class QRect;
class QString;
class QWidget;


class kpAbstractSelection;


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
    static QMatrix skewMatrix (int width, int height, double hangle, double vangle);
    static QMatrix skewMatrix (const QImage &pixmap, double hangle, double vangle);

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
    static QMatrix rotateMatrix (int width, int height, double angle);
    static QMatrix rotateMatrix (const QImage &pixmap, double angle);

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

    static void drawLine (QImage *image,
        int x1, int y1, int x2, int y2,
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
    // Cubic Beizer.
    static void drawCurve (QImage *image,
        const QPoint &startPoint,
        const QPoint &controlPointP, const QPoint &controlPointQ,
        const QPoint &endPoint,
        const kpColor &color, int penWidth);

    static void fillRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &color,
        const kpColor &stippleColor = kpColor::Invalid);

    // Draws a rectangle / rounded rectangle / ellipse with top-left at
    // (x, y) with width <width> and height <height>.  Unlike QPainter,
    // this rectangle will really fit inside <width>x<height> and won't
    // be one pixel higher or wider etc.
    //
    // <width> and <height> must be >= 0.
    //
    // <fcolor> must not be invalid.  However, <bcolor> may be invalid
    // to signify an unfilled rectangle / rounded rectangle /ellipse.
    static void drawRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::Invalid,
        const kpColor &fStippleColor = kpColor::Invalid);
    static void drawRoundedRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::Invalid,
        const kpColor &fStippleColor = kpColor::Invalid);
    static void drawEllipse (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::Invalid,
        const kpColor &fStippleColor = kpColor::Invalid);


//
// Drawing Using Raster Operations
//
//
// 2. Raster Operation Emulation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Qt4 does not actually support raster operations, unlike Qt3.  So for
// now, these methods ignore any given colors and produce a stipple of
// <colorHint1> and <colorHint2>.
//
// LOTODO: For the widgetDraw*() methods (which aren't bound by the
//         no-alpha-channel requirement), if XRENDER is currently active,
//         we could do nice alpha effects instead of stippling.
//
// Should Qt support raster operations again, these methods should be
// changed to use them with the given colors.  <colorHint1> and
// <colorHint2> would then be ignored.  Note that transparent pixels that
// these raster operations might be drawing on -- and hence, blending with --
// might have uninitialized RGB values.  This has to be dealt with somehow
// (the KolourPaint/KDE3 approach is to simply use these uninitialized
//  values -- although it's a bit dodgy, it works well enough as you usually
//  get a stipple of arbitrary colors).
// The testcase for KDE3, which might still apply is:
//
//     1. Open an image with transparent pixels
//     2. Press CTRL+A to look at an XOR border
//     3. Fill the transparent pixels with any color to initialize the
//        RGB values
//     4. Fill those pixels so that they are transparent again
//     5. Press CTRL+A to look at an XOR border and compare with 2.
//

public:

    //
    // Simulated Stippled Raster XOR
    //

    // (used for polygonal selection border)
    static void drawStippledXORPolygon (QImage *image,
        const QPolygon &points,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2,
        bool isFinal = true);

    // Same as drawRect() but the border consists of stippled lines of
    // <fcolor1> and <fcolor2>, XOR'ed with the existing contents of the
    // pixmap.  Pen width is set to 1.
    //
    // (used for rectangular selection borders)
    static void drawStippledXORRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2);

    // The painter is clipped to <clipRect> if it is not empty.
    // (used for thumbnail rectangle)
    //
    // WARNING: Just for this method, neither <colorHint1> nor <colorHint2>
    //          are allowed to be transparent.
    static void widgetDrawStippledXORRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2,
        const QRect &clipRect = QRect ());


    //
    // Simulated Raster XOR Filling
    //

    // (used for text cursor)
    static void fillXORRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint1, const kpColor &colorHint2);

    // (used for selection resize handles)
    //
    // WARNING: Just for this method, neither <colorHint1> nor <colorHint2>
    //          are allowed to be transparent.
    static void widgetFillXORRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint1, const kpColor &colorHint2);


    //
    // Simulated Raster NOP
    //

    // (used for rectangular bounding border for non-rectangular selections
    //  and when dragging a rectangle to zoom into with the Zoom Tool)
    static void drawNOTRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &colorHint1, const kpColor &colorHint2);

    // (used for document resizing lines)
    //
    // WARNING: Just for this method, neither <colorHint1> nor <colorHint2>
    //          are allowed to be transparent.
    static void widgetFillNOTRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &colorHint1, const kpColor &colorHint2);
};


#endif  // KP_PIXMAP_FX_H
