
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
class QMatrix;
class QPainter;
class QPen;
class QPixmap;
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
// WARNING: The given QPixmap's can have masks but not alpha channels.
//          You must maintain this invariant (i.e. use the
//          KP_PFX_CHECK_NO_ALPHA_CHANNEL assertion).
//
class kpPixmapFX
{
public:
    // Call this before KApplication has been constructed.
    static void initPre ();

    // Call this as early as possible -- after KApplication is constructed
    // and before any kpPixmapFX methods have been called.
    //
    // This may display dialogs to the user.
    static void initPost ();


//
// Screen Depth
//

public:
    // Called by initPre() - do not call directly.
    static void initScreenDepthPre ();

    // If the screen mode is paletted, it brings up a dialog warning the
    // user that KolourPaint has not been tested under such an environment
    // (paletted QImage's, usually created from paletted QPixmap's, need
    //  completely different code paths to truecolor QImage's; see also
    //  the warning in convertToQImage()).
    //
    // Called by initPost() - do not call directly.
    static void initScreenDepthPost ();


    // Returns whether the screen mode is paletted, rather than truecolor.
    // If this returns true, all QPixmap's (and therefore, kpImage's) are
    // paletted.  Otherwise, they are truecolor.
    static bool screenIsPaletted ();

    // Returns a translated message stating that the (image) effect does
    // not support paletted screen modes.
    //
    // This method is included here so that the similar messages in this
    // method and initScreenDepthPost() can be synchronized.
    //
    // ASSUMPTION: screenIsPaletted()
    static QString effectDoesNotSupportPalettedScreenMessage ();


//
// QPixmap/QImage Conversion Functions
//

public:

    //
    // Converts <pixmap> to a QImage and returns it.
    //
    // WARNING: On an 8-bit screen:
    //
    //              QPixmap result = convertToPixmap (convertToQImage (pixmap));
    //
    //          <result> is slightly differently colored to <pixmap>.
    //
    //          KolourPaint needs to convert to QImage occasionally as
    //          QImage allows KolourPaint to read pixels and because the QImage
    //          paint engine gives reliable results and pixel-identical results on
    //          all platforms.  The QPixmap paint engine has no such guarantee
    //          and even depends on the quality of the video driver.
    //
    //          As a result, KolourPaint should not be used on an 8-bit screen.
    //
    //          This bug will be fixed when KolourPaint gets a proper image library,
    //          where QPixmap -> QImage -> QPixmap transitions will be not be needed.
    static QImage convertToQImage (const QPixmap &pixmap);

    //
    // Dialog info for warning about data loss with convertToPixmap().
    //
    // Currently (2007-08-20), instances are only created in:
    //
    // 1. kpDocument::getPixmapFromFile()
    // 2. kpMainWindow::pasteWarnAboutLossInfo ()
    //
    // KDE3: Backport message changes (which do justice to the seriousness of
    //       the issues) to KDE 3.
    //
    struct WarnAboutLossInfo
    {
        // <moreColorsThanDisplayAndHasAlphaChannelMessage>:
        //
        //     ki18n ("<qt><p>The (image \"example.jpg\"|image to be pasted)"
        //            " may have more colors than the current screen mode can support."
        //            " In order to display it, some color information may be removed.</p>"
        //
        //            "<p><b>If you save this image, any color loss will become"
        //            " permanent.</b></p>"
        //
        //            "<p>To avoid this issue, increase your screen depth to at"
        //            " least %1bpp and then restart KolourPaint.</p>"
        //
        //            "<hr/>"
        //
        //            "<p>It also"
        //
        //            " contains translucency which is not fully"
        //            " supported. The translucency data will be"
        //            " approximated with a 1-bit transparency mask.</p>"
        //
        //            "<p><b>If you save this image, this loss of translucency will"
        //            " become permanent.</b></p></qt>")
        //
        // <moreColorsThanDisplayMessage>:
        //     ki18n ("<qt><p>The (image \"example.jpg\"|image to be pasted)"
        //            " may have more colors than the current screen mode can support."
        //            " In order to display it, some color information may be removed.</p>"
        //
        //            "<p><b>If you save this image, any color loss will become"
        //            " permanent.</b></p>"
        //
        //            "<p>To avoid this issue, increase your screen depth to at"
        //            " least %1bpp and then restart KolourPaint.</p></qt>")
        //
        // <hasAlphaChannelMessage>:
        //     i18n ("<qt><p>The (image \"example.jpg\"|image to be pasted)"
        //           " contains translucency which is not fully"
        //           " supported. The translucency data will be"
        //           " approximated with a 1-bit transparency mask.</p>"
        //
        //           "<p><b>If you save this image, this loss of translucency will"
        //           " become permanent.</b></p></qt>")
        //
        // <dontAskAgainPrefix>:
        //
        //     Don'tAskAgain ID for dialog.
        //
        // <parent>:
        //
        //     Dialog parent
        //
        WarnAboutLossInfo (
                const KLocalizedString &moreColorsThanDisplayAndHasAlphaChannelMessage,
                const KLocalizedString &moreColorsThanDisplayMessage,
                const QString &hasAlphaChannelMessage,
                const QString &dontAskAgainPrefix,
                QWidget *parent)
            :
                m_moreColorsThanDisplayAndHasAlphaChannelMessage (
                    moreColorsThanDisplayAndHasAlphaChannelMessage),
                m_moreColorsThanDisplayMessage (
                    moreColorsThanDisplayMessage),
                m_hasAlphaChannelMessage (
                    hasAlphaChannelMessage),
                m_dontAskAgainPrefix (
                    dontAskAgainPrefix),
                m_parent (parent),
                m_isValid (true)
        {
        }

        WarnAboutLossInfo ()
            : m_parent (0),
              m_isValid (false)
        {
        }

        ~WarnAboutLossInfo ()
        {
        }


        bool isValid () const { return m_isValid; }


        KLocalizedString m_moreColorsThanDisplayAndHasAlphaChannelMessage;
        KLocalizedString m_moreColorsThanDisplayMessage;
        QString m_hasAlphaChannelMessage;
        QString m_dontAskAgainPrefix;
        QWidget *m_parent;
        bool m_isValid;
    };

    //
    // Converts <image> to a QPixmap of the current display's depth and
    // returns it.
    //
    // If the flag <pretty> is set, it will dither the image making the
    // returned pixmap look better but if the image has few colours
    // (less than the screen can handle), this will be at the expense of
    // exactness of conversion.
    //
    // This will automatically call ensureNoAlphaChannel().
    //
    // Never use a foreign QPixmap that is offered to you - always get the
    // foreign QImage and use this function to convert it to a sane QPixmap.
    //
    // <wali>, if specified, describes parameters for the dialog that comes
    // up warning the user of data loss if the <image> contains translucency
    // and/or more colors than the current display.
    //
    static QPixmap convertToPixmap (const QImage &image, bool pretty = false,
                                    const WarnAboutLossInfo &wali = WarnAboutLossInfo ());

    // Same as convertToPixmap() but tries as hard as possible to make the
    // pixmap look like the original <image> - when in doubt, reads the
    // config to see whether or not to dither (default: on).
    //
    // If you know for sure that <image> can be displayed losslessly on
    // the screen, you should call convertToPixmap() with <pretty> = false
    // instead.  If you know for sure that <image> cannot be displayed
    // losslessly, then call convertToPixmap() with <pretty> = true.
    //
    static QPixmap convertToPixmapAsLosslessAsPossible (const QImage &image,
        const WarnAboutLossInfo &wali = WarnAboutLossInfo ());


    // Sets the RGB values of the pixels where <pixmap> is transparent to
    // <transparentColor>.  This has visually no effect on the <pixmap>
    // unless the mask is lost.
    //
    // TODO: As of Qt4, this does not appear to work:
    //
    //       It certainly does not work if you convert the result to a QImage,
    //       without nuking the result's mask before the conversion.  The
    //       transparent pixels in the QImage seem to have black RGB values.
    //       It looks like nuking the mask gets around this but then you lose
    //       transparency (nevertheless, this solution is usually applicable
    //       if you later restore the transparency).
    //
    //       I'm not sure if it works if you _don't_ do a QImage conversion.
    static QPixmap pixmapWithDefinedTransparentPixels (const QPixmap &pixmap,
        const QColor &transparentColor);



//
// Abstract Drawing
//

public:

    // Inside a <drawFunc> passed to kpPixmapFX::draw(), pass the <color>
    // you intend to draw in and <drawingOnRGBLayer> (passed to your <drawFunc>),
    // and this will spit back the appropriate QColor depending on whether you
    // are drawing on the RGB layer or a mask.
    static QColor draw_ToQColor (const kpColor &color, bool drawingOnRGBLayer);

    // Exercises the drawing pattern on QPixmap's - draws on, separately, the:
    //
    // 1. RGB layer (if there is an opaque colour involved in the drawing i.e.
    //               <anyColorOpaque>)
    // 2. Mask layer (if there is a transparency involved i.e.
    //                <anyColorTransparent> or the <image> has a mask to start
    //                with)
    //
    // Each time, it opens up a QPainter and calls <drawFunc> with:
    //
    // 1. A pointer to this QPainter
    // 2. A boolean that is true if we are currently drawing on the RGB layer
    //    and false if we are drawing on the Mask layer.  Often passed straight
    //    to draw_ToQColor() to convert from kpColor to QColor.
    // 3. A pointer to the provided <data>
    //
    // Use of this function permits drawing on pixmaps without breaking our
    // invariant of no alpha channels (KP_PFX_CHECK_NO_ALPHA_CHANNEL).
    //
    // WARNING: The current implementation does not permit <drawFunc> to access
    //          <image> (as it clears <image>'s mask before drawFunc() (but
    //          does restore the mask later)).
    //
    // ASSUMPTIONS:
    //
    // 1. <image> already satisifes the no-alpha-channel invariant
    //    (any newly-constructed QPixmap will, even though the RGB channel
    //     contains randomness).
    // 2. <image> is not of depth 1.
    //
    static void draw (QPixmap *image,
        void (*drawFunc) (QPainter * /*p*/,
            bool /*drawingOnRGBLayer*/,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data);

    // Same as above except that <drawFunc> is called a maximum of once
    // for the RGB and mask layers simultaneously.  Regarding the arguments
    // this function passes to <drawFunc>, <rgbPainter> may be 0 and
    // <maskPainter> may be 0 - but not both.
    //
    // <drawFunc> must return the dirty rectangle (return the full image
    // rectangle if in doubt).  This is then returned by draw() itself.
    static QRect draw (QPixmap *image,
        QRect (*drawFunc) (QPainter * /*rgbPainter*/, QPainter * /*maskPainter*/,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data);


//
// Get/Set Parts of Pixmap
//

public:

    //
    // Returns the pixel and mask data found at the <rect> in <pm>.
    //
    static QPixmap getPixmapAt (const QPixmap &pm, const QRect &rect);

    //
    // Sets the pixel and mask data at <destRect> in <*destPixmapPtr>
    // to <srcPixmap>.  Neither <destRect>'s width nor height are allowed
    // to be bigger than <srcPixmap>'s (you can't copy more than you have).
    // On the other hand, you can copy less than the size of <srcPixmap>
    // - no scaling is done.
    //
    static void setPixmapAt (QPixmap *destPixmapPtr, const QRect &destRect,
                             const QPixmap &srcPixmap);

    //
    // Sets the pixel and mask data at the rectangle in <*destPixmapPtr>,
    // with the top-left <destAt> and dimensions <srcPixmap.rect()>,
    // to <srcPixmap>.
    //
    static void setPixmapAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                             const QPixmap &srcPixmap);
    static void setPixmapAt (QPixmap *destPixmapPtr, int destX, int destY,
                             const QPixmap &srcPixmap);

    //
    // Draws <srcPixmap> on top of <*destPixmapPtr> at <destAt>.
    // The mask of <*destPixmapPtr> is adjusted so that all opaque
    // pixels in <srcPixmap> will be opaque in <*destPixmapPtr>.
    //
    static void paintPixmapAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                               const QPixmap &srcPixmap);
    static void paintPixmapAt (QPixmap *destPixmapPtr, int destX, int destY,
                               const QPixmap &srcPixmap);

    //
    // Returns the colour of the pixel at <at> in <pm>.
    // If the pixel is transparent, a value is returned such that
    // kpTool::isColorTransparent(<return_value>) will return true.
    //
    static kpColor getColorAtPixel (const QPixmap &pm, const QPoint &at);
    static kpColor getColorAtPixel (const QPixmap &pm, int x, int y);

    //
    // Returns the color of the pixel at <at> in <img>.
    // If the pixel is transparent, a value is returned such that
    // kpTool::isColorTransparent(<return_value>) will return true.
    //
    static kpColor getColorAtPixel (const QImage &img, const QPoint &at);
    static kpColor getColorAtPixel (const QImage &img, int x, int y);


//
// Mask Operations
//

public:

    // For X11: QPixmap, with XRENDER under a screen depth of 32bpp
    // (subtlely different to 24bpp), always has an alpha channel, even
    // if the channel is empty.  This violates the KolourPaint invariant
    // so we fix that by disabling XRENDER, killing the alpha channel.
    // The other approach would be to change screen depth to 24bpp but
    // such changes are not possible under X11 (XRANDR limitation).
    //
    // This is nasty since it reduces the quality of the whole GUI and
    // prevents the text in text boxes from antialiasing.
    //
    // The proper fix -- as opposed to disabling XRENDER --  would be to either:
    //
    // a) Rewrite the kpImage class so that it's not tied to the screen
    //    depth (long term goal that would take months to do).
    //
    // OR
    //
    // b) Add extra code paths to support images with alpha channels
    //    (would probably require a few days of work).
    //
    // Called by initPre() - do not call directly.
    static void initMaskOpsPre ();

    // Asserts invariants.
    //
    // Called by initPost() - do not call directly.
    static void initMaskOpsPost ();

    // You should use this instead of !QPixmap::mask().isNull(), as this
    // is faster, and instead of QPixmap::hasAlpha(), for encapsulation
    // purposes.
    static bool hasMask (const QPixmap &pixmap);

    // You should use this instead of QPixmap::hasAlphaChannel(),
    // for encapsulation purposes.
    static bool hasAlphaChannel (const QPixmap &pixmap);

    // Controls whether the KP_PFX_CHECK_NO_ALPHA_CHANNEL assert, if it
    // trips, should print out an error message or crash.
    //
    // Called by KP_PFX_CHECK_NO_ALPHA_CHANNEL() - do not call directly.
    // LOREFACTOR: Compile out in release mode.
    static bool checkNoAlphaChannelInternal (const QPixmap &pixmap);

    // Simply removing instances of this invariant check, to avoid
    // assertion failure, is ABSOLUTELY WRONG and is merely pushing
    // bugs elsewhere.
    //
    // See ensureNoAlphaChannel().
    #define KP_PFX_CHECK_NO_ALPHA_CHANNEL(pixmap)  \
        Q_ASSERT (kpPixmapFX::checkNoAlphaChannelInternal (pixmap))


    //
    // Removes <*destPixmapPtr>'s Alpha Channel and attempts to convert it
    // to a mask.  KolourPaint does not support Alpha Channels - only masks.
    //
    // Note that this method is slow so you should avoid calling it, if
    // possible.
    //
    // Generally, the only time you need to call it is when you get a QImage
    // from a foreign source (e.g. from a file or clipboard, not originally
    // from a QPixmap) and need to convert it to a QPixmap.  You don't need
    // to call it anywhere else since internally, all KolourPaint code is
    // supposed to maintain the QPixmap-has-no-alpha-channel invariant.
    // However, convertToPixmap() and convertToPixmapAsLosslessAsPossible()
    // call this method for you anyway.
    //
    // It is invalid to use a QPainter directly on a QPixmap, which
    // usually introduces an alpha channel, and then to call this method to
    // fix it up.  While this works on XRENDER, the same code path will fail
    // without XRENDER: QPainter does not draw to the QPixmap::mask() when
    // drawing to a QPixmap, if XRENDER is disabled.  The correct way to
    // manipulate QPixmap's is to use this class' draw*() methods or draw(),
    // which do not introduce an alpha channel and work regardless of whether
    // XRENDER is enabled.
    //
    // The no-alpha-channel invariant is checked by the
    // KP_PFX_CHECK_NO_ALPHA_CHANNEL assertion.  You must maintain this
    // invariant for all pixmaps that you manipulate using kpPixmapFX methods
    // or else:
    //
    // 1. For XRENDER screens, KolourPaint will slow down to a crawl
    //    (QPixmap::mask(), used extensively by kpPixmapFX, is very slow if
    //     an alpha channel exists) and there may be some painting artifacts.
    //
    // 2. For non-XRENDER screens, the invariant cannot be broken since
    //    alpha channels are not supported.  However, the code path, that
    //    would have broken the invariant had XRENDER been enabled, would
    //    likely fail to draw to the pixmap mask with XRENDER disabled (see
    //    above).
    //
    static void ensureNoAlphaChannel (QPixmap *destPixmapPtr);

    //
    // Returns <pm>'s mask or a fully opaque mask (with <pm>'s dimensions)
    // if <pm> does not have a mask.
    //
    static QBitmap getNonNullMask (const QPixmap &pm);

    //
    // Ensures that <*destPixmapPtr> is transparent at <rect>.
    //
    static void ensureTransparentAt (QPixmap *destPixmapPtr, const QRect &destRect);

    //
    // Sets the mask of <*destPixmapPtr> at the rectangle, with the
    // top-left <destAt> and dimensions <srcMaskBitmap.rect()>,
    // to transparent where <brushBitmap> is opaque.
    //
    // <brushPixmap> must be a QPixmap of depth 1 (or a QBitmap).
    //
    // TODO: we must drop this since 1. It is very slow 2. We later want to support alpha so this method makes little sense.
    static KDE_DEPRECATED void paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, const QPoint &destAt,
                                               const QPixmap &brushBitmap);
    static KDE_DEPRECATED void paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, int destX, int destY,
                                               const QPixmap &brushBitmap);

    //
    // Ensures that <*destPixmapPtr> is opaque at <rect>.
    //
    static void ensureOpaqueAt (QPixmap *destPixmapPtr, const QRect &destRect);


//
// Effects
//

public:

    //
    // Fills an image in the given color.
    //
    static void fill (QPixmap *destPixmapPtr, const kpColor &color);
    static QPixmap fill (const QPixmap &pm, const kpColor &color);


//
// Transforms
//

public:

    //
    // Resizes an image to the given width and height,
    // filling any new areas with <backgroundColor>.
    //
    static void resize (QPixmap *destPixmapPtr, int w, int h,
                        const kpColor &backgroundColor);
    static QPixmap resize (const QPixmap &pm, int w, int h,
                           const kpColor &backgroundColor);

    //
    // Scales an image to the given width and height.
    // If <pretty> is true, a smooth scale will be used.
    //
    static void scale (QPixmap *destPixmapPtr, int w, int h, bool pretty = false);
    static QPixmap scale (const QPixmap &pm, int w, int h, bool pretty = false);


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
    static QMatrix skewMatrix (const QPixmap &pixmap, double hangle, double vangle);

    static void skew (QPixmap *destPixmapPtr, double hangle, double vangle,
                      const kpColor &backgroundColor,
                      int targetWidth = -1, int targetHeight = -1);
    static QPixmap skew (const QPixmap &pm, double hangle, double vangle,
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
    static QMatrix rotateMatrix (const QPixmap &pixmap, double angle);

    static bool isLosslessRotation (double angle);

    static void rotate (QPixmap *destPixmapPtr, double angle,
                        const kpColor &backgroundColor,
                        int targetWidth = -1, int targetHeight = -1);
    static QPixmap rotate (const QPixmap &pm, double angle,
                           const kpColor &backgroundColor,
                           int targetWidth = -1, int targetHeight = -1);


    //
    // Flips an image in the given directions.
    //
    static QMatrix flipMatrix (int width, int height, bool horz, bool vert);
    static QMatrix flipMatrix (const QPixmap &pixmap, bool horz, bool vert);

    // TODO: this kind of overloading is error prone
    //       e.g. QPixmap pixmap;
    //            kpPixmapFX::flip (pixmap, false, true);
    //       looks like it will flip vertically but does absolutely nothing!
    //       (should be &pixmap)
    static void flip (QPixmap *destPixmapPtr, bool horz, bool vert);
    static QPixmap flip (const QPixmap &pm, bool horz, bool vert);
    static void flip (QImage *destImagePtr, bool horz, bool vert);
    static QImage flip (const QImage &img, bool horz, bool vert);


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
    static void drawPolyline (QPixmap *image,
        const QPolygon &points,
        const kpColor &color, int penWidth,
        const kpColor &stippleColor = kpColor::Invalid);
    static void drawLine (QPixmap *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth,
        const kpColor &stippleColor = kpColor::Invalid);
    // <isFinal> = shape completed else drawing but haven't finalised.
    // If not <isFinal>, the edge that would form the closure, if the
    // shape were finalised now, is highlighted specially.  Unfortunately,
    // the argument is currently ignored.
    //
    // Odd-even fill.
    static void drawPolygon (QPixmap *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor = kpColor::Invalid,
        bool isFinal = true,
        const kpColor &fStippleColor = kpColor::Invalid);
    // Cubic Beizer.
    static void drawCurve (QPixmap *image,
        const QPoint &startPoint,
        const QPoint &controlPointP, const QPoint &controlPointQ,
        const QPoint &endPoint,
        const kpColor &color, int penWidth);

    static void fillRect (QPixmap *image,
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
    static void drawRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::Invalid,
        const kpColor &fStippleColor = kpColor::Invalid);
    static void drawRoundedRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::Invalid,
        const kpColor &fStippleColor = kpColor::Invalid);
    static void drawEllipse (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::Invalid,
        const kpColor &fStippleColor = kpColor::Invalid);


//
// Drawing Using Raster Operations
//
//
// 1. Alpha Channel Invariant
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// The widgetDraw*() methods do not deal with pixmap data, so they safely
// safely disregard our pixmap invariant of not introducing an alpha
// channel (KP_PFX_CHECK_NO_ALPHA_CHANNEL).  This permits far
// more straightforward implementations.
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
    static void drawStippledXORPolygon (QPixmap *image,
        const QPolygon &points,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2,
        bool isFinal = true);

    // Same as drawRect() but the border consists of stippled lines of
    // <fcolor1> and <fcolor2>, XOR'ed with the existing contents of the
    // pixmap.  Pen width is set to 1.
    //
    // (used for rectangular selection borders)
    static void drawStippledXORRect (QPixmap *image,
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
    static void fillXORRect (QPixmap *image,
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
    static void drawNOTRect (QPixmap *image,
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
