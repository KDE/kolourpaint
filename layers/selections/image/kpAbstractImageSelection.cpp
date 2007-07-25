
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


#define DEBUG_KP_SELECTION 0


#include <kpAbstractImageSelection.h>

#include <QBitmap>
#include <QPainter>

#include <KDebug>


struct kpAbstractImageSelectionPrivate
{
    kpImage baseImage;

    kpImageSelectionTransparency transparency;
    // The mask for the image, after selection transparency (a.k.a. background
    // subtraction) is applied.
    QBitmap transparencyMaskCache;  // OPT: calculate lazily i.e. on-demand only
};


// protected
kpAbstractImageSelection::kpAbstractImageSelection (
        const kpImageSelectionTransparency &transparency)
    : kpAbstractSelection (),
      d (new kpAbstractImageSelectionPrivate ())
{
    setTransparency (transparency);
}

// protected
kpAbstractImageSelection::kpAbstractImageSelection (const QRect &rect,
        const kpImage &baseImage,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractSelection (rect),
      d (new kpAbstractImageSelectionPrivate ())
{
    // This also checks that <rect> and <baseImage> have compatible
    // relative dimensions.
    setBaseImage (baseImage);

    setTransparency (transparency);
}

// protected
kpAbstractImageSelection::kpAbstractImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractSelection (rect),
      d (new kpAbstractImageSelectionPrivate ())
{
    setTransparency (transparency);
}

// protected
kpAbstractImageSelection &kpAbstractImageSelection::operator= (
        const kpAbstractImageSelection &rhs)
{
    kpAbstractSelection::operator= (rhs);

    d->baseImage = rhs.d->baseImage;

    d->transparency = rhs.d->transparency;
    d->transparencyMaskCache = rhs.d->transparencyMaskCache;

    return *this;
}

// protected
kpAbstractImageSelection::~kpAbstractImageSelection ()
{
    delete d;
}


// public virtual [base kpAbstractSelection]
bool kpAbstractImageSelection::readFromStream (QDataStream &stream,
        const kpPixmapFX::WarnAboutLossInfo &wali)
{
    if (!kpAbstractSelection::readFromStream (stream, wali))
        return false;

    QImage image;
    stream >> image;
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\timage: w=" << image.width () << " h=" << image.height ()
               << " depth=" << image.depth () << endl;
#endif

    if (!image.isNull ())
    {
        d->baseImage = kpPixmapFX::convertToPixmap (image, false/*no dither*/, wali);
    }
    else
        d->baseImage = kpImage ();

    // TODO: Reset transparency mask?
    // TODO: Concrete subclass need to emit changed()?
    //       [we can't since changed() must be called after all reading
    //        is complete and subclasses always call this method
    //        _before_ their reading logic]
    return true;
}

// public virtual [base kpAbstractSelection]
void kpAbstractImageSelection::writeToStream (QDataStream &stream) const
{
    kpAbstractSelection::writeToStream (stream);

    if (!d->baseImage.isNull ())
    {
        const QImage image = kpPixmapFX::convertToImage (d->baseImage);
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\twrote image rect=" << image.rect () << endl;
    #endif
        stream << image;
    }
    else
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\twrote no image because no pixmap" << endl;
    #endif
        stream << QImage ();
    }
}


// public virtual [kpAbstractSelection]
QString kpAbstractImageSelection::name () const
{
    return i18n ("Selection");
}

// public virtual [base kpAbstractSelection]
kpCommandSize::SizeType kpAbstractImageSelection::size () const
{
    return kpAbstractSelection::size () +
        (kpCommandSize::ImageSize (d->baseImage) +
         kpCommandSize::PixmapSize (d->transparencyMaskCache));
}

// public
kpCommandSize::SizeType kpAbstractImageSelection::sizeWithoutImage () const
{
    return (size () - kpCommandSize::ImageSize (d->baseImage));
}

// public virtual [kpAbstractSelection]
bool kpAbstractImageSelection::hasContent () const
{
    return (!d->baseImage.isNull ());
}


// public virtual [kpAbstractSelection]
int kpAbstractImageSelection::minimumWidth () const
{
    return 1;
}

// public virtual [kpAbstractSelection]
int kpAbstractImageSelection::minimumHeight () const
{
    return 1;
}


// public virtual
// TODO: You could probably compute this by using a fill with
//       QPainter::setClipRegion(shapeRegion()).
//
//       This would eliminate possible inconsistency between shapeRegion() and us.
//
//       However, kpEllipticalImageSelection currently implements shapeRegion()
//       in terms of shapeBitmap() so such a change is not yet possible.
QBitmap kpAbstractImageSelection::shapeBitmap (bool nullForRectangular) const
{
    (void) nullForRectangular;

    Q_ASSERT (boundingRect ().isValid ());

    QBitmap maskBitmap (width (), height ());
    maskBitmap.fill (Qt::color0/*transparent*/);

    QPainter painter;
    {
        painter.begin (&maskBitmap);

        // (Doesn't seem to be needed but do it just in case)
        painter.setBackgroundMode (Qt::OpaqueMode);

        painter.setPen (Qt::color1/*opaque*/);
        painter.setBrush (Qt::color1/*opaque*/);

        QPolygon points = calculatePoints ();
        points.translate (-x (), -y ());

        // Unlike QPainter::drawRect(), this draws the points literally
        // without being 1 pixel wider and higher.  This requires a QPen
        // or it will draw 1 pixel narrower and shorter.
        painter.drawPolygon (points, Qt::OddEvenFill);

        painter.end ();
    }

    return maskBitmap;
}


// public
kpImage kpAbstractImageSelection::givenImageMaskedByShape (const kpImage &image) const
{
#if DEBUG_KP_SELECTION
    kDebug () << "kpAbstractImageSelection::givenImageMaskedByShape() boundingRect="
              << boundingRect () << endl;
#endif
    Q_ASSERT (image.width () == width () && image.height () == height ());

    if (isRectangular ())
        return image;


    const QRegion mRegion = shapeRegion ().translated (-topLeft ());

#if DEBUG_KP_SELECTION
    kDebug () << "\tshapeRegion=" << shapeRegion ()
              << " [rect=" << shapeRegion ().boundingRect () << "]"
              << " calculatePoints=" << calculatePoints ()
              << " [rect=" << calculatePoints ().boundingRect () << "]"
              << endl;
#endif

    kpImage retImage (width (), height ());
    kpPixmapFX::ensureTransparentAt (&retImage, retImage.rect ());

    // OPT: Hopelessly inefficent due to function call overhead.
    //      kpPixmapFX should have a function that does this.
    //      Or use QPainter::setClipRegion()?  Or use QPainter::setClipPath()?
    foreach (QRect r, mRegion.rects ())
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\tcopy rect=" << r << endl;
    #endif
        // OPT: Hopelessly inefficient.  If kpPixmapFX::setPixmapAt() was
        //      more flexible, we wouldn't need to call getPixmapAt().
        const kpImage srcPixmap = kpPixmapFX::getPixmapAt (image, r);

        kpPixmapFX::setPixmapAt (&retImage, r.topLeft (), srcPixmap);
    }

    return retImage;
}


// public
kpImage kpAbstractImageSelection::baseImage () const
{
    return d->baseImage;
}

// public
void kpAbstractImageSelection::setBaseImage (const QPixmap &baseImage)
{
    d->baseImage = baseImage;

    if (!d->baseImage.isNull ())
    {
        Q_ASSERT (d->baseImage.width () == width () &&
                  d->baseImage.height () == height ());
    }

    recalculateTransparencyMaskCache ();

    emit changed (boundingRect ());
}


// public
kpImageSelectionTransparency kpAbstractImageSelection::transparency () const
{
    return d->transparency;
}

// public
bool kpAbstractImageSelection::setTransparency (
        const kpImageSelectionTransparency &transparency,
        bool checkTransparentPixmapChanged)
{
    if (d->transparency == transparency)
        return false;

    d->transparency = transparency;

    bool haveChanged = true;

    QBitmap oldTransparencyMaskCache = d->transparencyMaskCache;
    recalculateTransparencyMaskCache ();


    if (oldTransparencyMaskCache.width () == d->transparencyMaskCache.width () &&
        oldTransparencyMaskCache.height () == d->transparencyMaskCache.height ())
    {
        if (d->transparencyMaskCache.isNull ())
        {
        #if DEBUG_KP_SELECTION
            kDebug () << "\tboth old and new pixmaps are null - nothing changed" << endl;
        #endif
            haveChanged = false;
        }
        else if (checkTransparentPixmapChanged)
        {
            QImage oldTransparencyMaskImage = kpPixmapFX::convertToImage (oldTransparencyMaskCache);
            QImage newTransparencyMaskImage = kpPixmapFX::convertToImage (d->transparencyMaskCache);

            bool changed = false;
            for (int y = 0; y < oldTransparencyMaskImage.height () && !changed; y++)
            {
                for (int x = 0; x < oldTransparencyMaskImage.width () && !changed; x++)
                {
                    if (kpPixmapFX::getColorAtPixel (oldTransparencyMaskImage, x, y) !=
                        kpPixmapFX::getColorAtPixel (newTransparencyMaskImage, x, y))
                    {
                    #if DEBUG_KP_SELECTION
                        kDebug () << "\tdiffer at " << QPoint (x, y)
                                   << " old=" << (int *) kpPixmapFX::getColorAtPixel (oldTransparencyMaskImage, x, y).toQRgb ()
                                   << " new=" << (int *) kpPixmapFX::getColorAtPixel (newTransparencyMaskImage, x, y).toQRgb ()
                                   << endl;
                    #endif
                        changed = true;
                        break;
                    }
                }
            }

            if (!changed)
                haveChanged = false;
        }
    }


    if (haveChanged)
        emit changed (boundingRect ());

    return haveChanged;
}


// private
void kpAbstractImageSelection::recalculateTransparencyMaskCache ()
{
#if DEBUG_KP_SELECTION
    kDebug () << "kpAbstractImageSelection::recalculateTransparencyMaskCache()" << endl;
#endif

    if (d->baseImage.isNull ())
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\tno image - no need for transparency mask" << endl;
    #endif
        d->transparencyMaskCache = QBitmap ();
        return;
    }

    if (d->transparency.isOpaque ())
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\topaque - no need for transparency mask" << endl;
    #endif
        d->transparencyMaskCache = QBitmap ();
        return;
    }

    d->transparencyMaskCache = QBitmap (d->baseImage.width (), d->baseImage.height ());

    QImage image = kpPixmapFX::convertToImage (d->baseImage);
    QPainter transparencyMaskPainter (&d->transparencyMaskCache);

    bool hasTransparent = false;
    for (int y = 0; y < d->baseImage.height (); y++)
    {
        for (int x = 0; x < d->baseImage.width (); x++)
        {
            const kpColor pixelCol = kpPixmapFX::getColorAtPixel (image, x, y);
            if (pixelCol == kpColor::Transparent ||
                pixelCol.isSimilarTo (d->transparency.transparentColor (),
                                      d->transparency.processedColorSimilarity ()))
            {
                transparencyMaskPainter.setPen (Qt::color0/*transparent*/);
                hasTransparent = true;
            }
            else
            {
                transparencyMaskPainter.setPen (Qt::color1/*opaque*/);
            }

            transparencyMaskPainter.drawPoint (x, y);
        }
    }

    transparencyMaskPainter.end ();

    if (!hasTransparent)
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\tcolour useless - completely opaque" << endl;
    #endif
        d->transparencyMaskCache = QBitmap ();
        return;
    }
}


// public
kpImage kpAbstractImageSelection::transparentImage () const
{
    kpImage image = baseImage ();

    if (!d->transparencyMaskCache.isNull ())
    {
        image.setMask (d->transparencyMaskCache);
    }

    return image;
}


// public
void kpAbstractImageSelection::fill (const kpColor &color)
{
    kpImage newImage (width (), height ());
    kpPixmapFX::fill (&newImage, color);

    // LOTODO: Maybe disable Image/Clear menu item if transparent color
    if (color.isOpaque ())
        newImage.setMask (shapeBitmap ());

    setBaseImage (newImage);
}

// public virtual
void kpAbstractImageSelection::flip (bool horiz, bool vert)
{
#if DEBUG_KP_SELECTION && 1
    kDebug () << "kpAbstractImageSelection::flip(horiz=" << horiz
               << ",vert=" << vert << ")" << endl;
#endif

    if (!d->baseImage.isNull ())
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\thave pixmap - flipping that" << endl;
    #endif
        kpPixmapFX::flip (&d->baseImage, horiz, vert);
    }

    if (!d->transparencyMaskCache.isNull ())
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\thave transparency mask - flipping that" << endl;
    #endif
        kpPixmapFX::flip (&d->transparencyMaskCache, horiz, vert);
    }


    emit changed (boundingRect ());
}


static void Paint (const kpAbstractImageSelection *sel, const kpImage &srcImage,
        QPixmap *destPixmap, const QRect &docRect)
{
    if (!srcImage.isNull ())
    {
        kpPixmapFX::paintPixmapAt (destPixmap,
                                   sel->topLeft () - docRect.topLeft (),
                                   srcImage);
    }
}

// public virtual [kpAbstractSelection]
void kpAbstractImageSelection::paint (QPixmap *destPixmap,
        const QRect &docRect) const
{
    ::Paint (this, transparentImage (), destPixmap, docRect);
}

// public
void kpAbstractImageSelection::paintWithBaseImage (QPixmap *destPixmap,
        const QRect &docRect) const
{
    ::Paint (this, baseImage (), destPixmap, docRect);
}


#include <kpAbstractImageSelection.moc>
