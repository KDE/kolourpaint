
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#ifndef __kppixmapfx_h__
#define __kppixmapfx_h__


class QBitmap;
class QImage;
class QPixmap;
class QPoint;
class QRect;
class QWMatrix;

class kpColor;


class kpPixmapFX
{
public:
    /*
     * QPixmap/QImage Conversion Functions
     */

    /*
     * Converts <pixmap> to a QImage and returns it.
     */
    static QImage convertToImage (const QPixmap &pixmap);

    /*
     * Converts <image> to a QPixmap of the current display's depth and
     * returns it.
     *
     * If the flag <pretty> is set, it may dither the image making the
     * returned pixmap look better at the expense of exactness of conversion.
     *
     * This will automatically call ensureNoAlphaChannel().  If you pass a
     * pointer through the <hadAlphaChannel> parameter, it will return
     * whether or not the pixmap had an Alpha Channel (not just a mask)
     * before the call to ensureNoAlphaChannel().
     *
     * Never use a foreign QPixmap that is offered to you - always get the
     * foreign QImage and use this function to convert it to a sane QPixmap.
     */
    static QPixmap convertToPixmap (const QImage &image, bool pretty = false,
                                    bool *hadAlphaChannel = 0);


    /*
     * Get/Set Parts of Pixmap
     */


    /*
     * Returns the pixel and mask data found at the <rect> in <pm>.
     */
    static QPixmap getPixmapAt (const QPixmap &pm, const QRect &rect);

    /*
     * Sets the pixel and mask data at <destRect> in <*destPixmapPtr>
     * to <srcPixmap>.
     */
    static void setPixmapAt (QPixmap *destPixmapPtr, const QRect &destRect,
                             const QPixmap &srcPixmap);

    /*
     * Sets the pixel and mask data at the rectangle in <*destPixmapPtr>,
     * with the top-left <destAt> and dimensions <srcPixmap.rect()>,
     * to <srcPixmap>.
     */
    static void setPixmapAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                             const QPixmap &srcPixmap);
    static void setPixmapAt (QPixmap *destPixmapPtr, int destX, int destY,
                             const QPixmap &srcPixmap);

    /*
     * Draws <srcPixmap> on top of <*destPixmapPtr> at <destAt>.
     * The mask of <*destPixmapPtr> is adjusted so that all opaque
     * pixels in <srcPixmap> will be opaque in <*destPixmapPtr>.
     */
    static void paintPixmapAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                               const QPixmap &srcPixmap);
    static void paintPixmapAt (QPixmap *destPixmapPtr, int destX, int destY,
                               const QPixmap &srcPixmap);

    /*
     * Returns the colour of the pixel at <at> in <pm>.
     * If the pixel is transparent, a value is returned such that
     * kpTool::isColorTransparent(<return_value>) will return true.
     */
    static kpColor getColorAtPixel (const QPixmap &pm, const QPoint &at);
    static kpColor getColorAtPixel (const QPixmap &pm, int x, int y);

    /*
     * Returns the color of the pixel at <at> in <img>.
     * If the pixel is transparent, a value is returned such that
     * kpTool::isColorTransparent(<return_value>) will return true.
     */
    static kpColor getColorAtPixel (const QImage &img, const QPoint &at);
    static kpColor getColorAtPixel (const QImage &img, int x, int y);


    /*
     * Mask Operations
     */


    /*
     * Removes <*destPixmapPtr>'s Alpha Channel and attempts to convert it
     * to a mask.  KolourPaint - and QPixmap to a great extent - does not
     * support Alpha Channels - only masks.  Call this whenever you get
     * a pixmap from a foreign source; else all KolourPaint code will
     * exhibit "undefined behaviour".
     */
    static void ensureNoAlphaChannel (QPixmap *destPixmapPtr);

    /*
     * Returns <pm>'s mask or a fully opaque mask (with <pm>'s dimensions)
     * if <pm> does not have a mask.
     */
    static QBitmap getNonNullMask (const QPixmap &pm);

    /*
     * Returns the mask data found at the <rect> in <pm>.
     */
    static QBitmap getNonNullMaskAt (const QPixmap &pm, const QRect &rect);

    /*
     * Sets the mask of <*destPixmapPtr> at the rectangle, with the
     * top-left <destAt> and dimensions <srcMaskBitmap.rect()>,
     * to <srcMaskBitmap>.
     */
    static void setMaskAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                           const QBitmap &srcMaskBitmap);
    static void setMaskAt (QPixmap *destPixmapPtr, int destX, int destY,
                           const QBitmap &srcMaskBitmap);

    /*
     * Ensures that <*destPixmapPtr> is transparent at <rect>.
     */
    static void ensureTransparentAt (QPixmap *destPixmapPtr, const QRect &destRect);

    /*
     * Sets the mask of <*destPixmapPtr> at the rectangle, with the
     * top-left <destAt> and dimensions <srcMaskBitmap.rect()>,
     * to transparent where <brushBitmap> is opaque.
     *
     * <brushPixmap> must be a QPixmap of depth 1 (or a QBitmap).
     */
    static void paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, const QPoint &destAt,
                                               const QPixmap &brushBitmap);
    static void paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, int destX, int destY,
                                               const QPixmap &brushBitmap);

    /*
     * Ensures that <*destPixmapPtr> is opaque at <rect>.
     */
    static void ensureOpaqueAt (QPixmap *destPixmapPtr, const QRect &destRect);

    /*
     * Ensures that <srcPixmap>'s opaque pixels will be opaque if
     * painted onto <*destPixmapPtr> at <destAt>.
     */
    static void ensureOpaqueAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                                const QPixmap &srcPixmap);
    static void ensureOpaqueAt (QPixmap *destPixmapPtr, int destX, int destY,
                                const QPixmap &srcPixmap);


    /*
     * Effects
     */


    /*
     * Inverts the colours of each pixel in the given image.
     * These functions differ from QImage::invertPixels() in the following ways:
     *
     * 1. for 8-bit images, it inverts the colours of the Colour Table
     *    (this means that you would get visually similar results to inversion
     *     at higher bit depths - rather than a "random-looking" inversion
     *     depending on the contents of the Colour Table)
     * 2. never inverts the Alpha Buffer
     */
    static void invertColors (QPixmap *destPixmapPtr);
    static QPixmap invertColors (const QPixmap &pm);
    static void invertColors (QImage *destImagePtr);
    static QImage invertColors (const QImage &img);

    /*
     * Converts the image to grayscale.
     */
    static void convertToGrayscale (QPixmap *destPixmapPtr);
    static QPixmap convertToGrayscale (const QPixmap &pm);
    static void convertToGrayscale (QImage *destImagePtr);
    static QImage convertToGrayscale (const QImage &img);

    /*
     * Converts the image to monochrome.
     */
    static void convertToBlackAndWhite (QPixmap *destPixmapPtr);
    static QPixmap convertToBlackAndWhite (const QPixmap &pm);

    /*
     * Fills an image in the given color.
     */
    static void fill (QPixmap *destPixmapPtr, const kpColor &color);
    static QPixmap fill (const QPixmap &pm, const kpColor &color);

    /*
     * Resizes an image to the given width and height,
     * filling any new areas with <backgroundColor> if <fillNewAreas> is set.
     */
    static void resize (QPixmap *destPixmapPtr, int w, int h,
                        const kpColor &backgroundColor, bool fillNewAreas = true);
    static QPixmap resize (const QPixmap &pm, int w, int h,
                           const kpColor &backgroundColor, bool fillNewAreas = true);

    /*
     * Scales an image to the given width and height.
     */
    static void scale (QPixmap *destPixmapPtr, int w, int h);
    static QPixmap scale (const QPixmap &pm, int w, int h);

    /*
     * Skews an image.
     *
     * <hangle>             horizontal angle clockwise (-90 < x < 90)
     * <vangle>             vertical angle clockwise (-90 < x < 90)
     * <backgroundColor>    color to fill new areas with
     * <targetWidth>        if > 0, the desired width of the resultant pixmap
     * <targetHeight>       if > 0, the desired height of the resultant pixmap
     *
     * Using <targetWidth> & <targetHeight> to generate preview pixmaps is
     * significantly more efficient than skewing and then scaling yourself.
     */
    static QWMatrix skewMatrix (int width, int height, double hangle, double vangle);
    static QWMatrix skewMatrix (const QPixmap &pixmap, double hangle, double vangle);

    static void skew (QPixmap *destPixmapPtr, double hangle, double vangle,
                      const kpColor &backgroundColor,
                      int targetWidth = -1, int targetHeight = -1);
    static QPixmap skew (const QPixmap &pm, double hangle, double vangle,
                         const kpColor &backgroundColor,
                         int targetWidth = -1, int targetHeight = -1);

    /*
     * Rotates an image.
     *
     * <angle>              clockwise angle to rotate by
     * <backgroundColor>    color to fill new areas with
     * <targetWidth>        if > 0, the desired width of the resultant pixmap
     * <targetHeight>       if > 0, the desired height of the resultant pixmap
     *
     * Using <targetWidth> & <targetHeight> to generate preview pixmaps is
     * significantly more efficient than rotating and then scaling yourself.
     */
    static QWMatrix rotateMatrix (int width, int height, double angle);
    static QWMatrix rotateMatrix (const QPixmap &pixmap, double angle);

    static bool isLosslessRotation (double angle);

    static void rotate (QPixmap *destPixmapPtr, double angle,
                        const kpColor &backgroundColor,
                        int targetWidth = -1, int targetHeight = -1);
    static QPixmap rotate (const QPixmap &pm, double angle,
                           const kpColor &backgroundColor,
                           int targetWidth = -1, int targetHeight = -1);

    /*
     * Flips an image in the given directions.
     */
    static QWMatrix flipMatrix (int width, int height, bool horz, bool vert);
    static QWMatrix flipMatrix (const QPixmap &pixmap, bool horz, bool vert);

    static void flip (QPixmap *destPixmapPtr, bool horz, bool vert);
    static QPixmap flip (const QPixmap &pm, bool horz, bool vert);
    static void flip (QImage *destImagePtr, bool horz, bool vert);
    static QImage flip (const QImage &img, bool horz, bool vert);
};


#endif  // __kppixmapfx_h__
