
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


#include "layers/selections/image/kpAbstractImageSelection.h"

#include <QBitmap>
#include <QPainter>

#include "kpLogCategories.h"

//---------------------------------------------------------------------

// Returns whether <sel> can be set to have <baseImage>.
// In other words, this is the precondition for <sel>.setBaseImage(<baseImage).
//
// This checks that they have compatible relative dimensions.
static bool CanSetBaseImageTo (kpAbstractImageSelection *sel, const kpImage &baseImage)
{
    if (baseImage.isNull ())
    {
        // Always allowed to wipe out selection content, changing it into a
        // border.
        return true;
    }

    return (baseImage.width () == sel->width () &&
            baseImage.height () == sel->height ());
}

//---------------------------------------------------------------------

struct kpAbstractImageSelectionPrivate
{
    kpImage baseImage;

    kpImageSelectionTransparency transparency;

    // The mask for the image, after selection transparency (a.k.a. background
    // subtraction) is applied.
    QBitmap transparencyMaskCache;  // OPT: calculate lazily i.e. on-demand only
};

//---------------------------------------------------------------------

// protected
kpAbstractImageSelection::kpAbstractImageSelection (
        const kpImageSelectionTransparency &transparency)
    : kpAbstractSelection (),
      d (new kpAbstractImageSelectionPrivate ())
{
    setTransparency (transparency);
}

//---------------------------------------------------------------------

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

//---------------------------------------------------------------------

// protected
kpAbstractImageSelection::kpAbstractImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractSelection (rect),
      d (new kpAbstractImageSelectionPrivate ())
{
    setTransparency (transparency);
}

//---------------------------------------------------------------------

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

//---------------------------------------------------------------------

// protected
kpAbstractImageSelection::~kpAbstractImageSelection ()
{
    delete d;
}

//---------------------------------------------------------------------

// public virtual [base kpAbstractSelection]
bool kpAbstractImageSelection::readFromStream (QDataStream &stream)
{
    if (!kpAbstractSelection::readFromStream (stream )) {
        return false;
    }

    QImage qimage;
    stream >> qimage;
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "\timage: w=" << qimage.width () << " h=" << qimage.height ()
               << " depth=" << qimage.depth ();
#endif

    if (!qimage.isNull ())
    {
        // Image size does not match the selection's dimensions?
        // This call only accesses our superclass' fields, which have already
        // been read in.
        if (!::CanSetBaseImageTo (this, qimage))
        {
            return false;
        }

        d->baseImage = qimage;
    }
    // (was just a selection border in the clipboard, even though KolourPaint's
    //  GUI doesn't allow you to copy such a thing into the clipboard)
    else {
        d->baseImage = kpImage ();
    }

    // TODO: Reset transparency mask?
    // TODO: Concrete subclass need to emit changed()?
    //       [we can't since changed() must be called after all reading
    //        is complete and subclasses always call this method
    //        _before_ their reading logic]
    return true;
}

//---------------------------------------------------------------------

// public virtual [base kpAbstractSelection]
void kpAbstractImageSelection::writeToStream (QDataStream &stream) const
{
    kpAbstractSelection::writeToStream (stream);

    if (!d->baseImage.isNull ())
    {
        const QImage image = d->baseImage;
    #if DEBUG_KP_SELECTION && 1
        qCDebug(kpLogLayers) << "\twrote image rect=" << image.rect ();
    #endif
        stream << image;
    }
    else
    {
    #if DEBUG_KP_SELECTION && 1
        qCDebug(kpLogLayers) << "\twrote no image because no pixmap";
    #endif
        stream << QImage ();
    }
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
QString kpAbstractImageSelection::name () const
{
    return i18n ("Selection");
}

//---------------------------------------------------------------------

// public virtual [base kpAbstractSelection]
kpCommandSize::SizeType kpAbstractImageSelection::size () const
{
    return kpAbstractSelection::size () +
        kpCommandSize::ImageSize (d->baseImage) +
        (d->transparencyMaskCache.width() * d->transparencyMaskCache.height()) / 8;
}

//---------------------------------------------------------------------

// public
kpCommandSize::SizeType kpAbstractImageSelection::sizeWithoutImage () const
{
    return (size () - kpCommandSize::ImageSize (d->baseImage));
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
int kpAbstractImageSelection::minimumWidth () const
{
    return 1;
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
int kpAbstractImageSelection::minimumHeight () const
{
    return 1;
}

//---------------------------------------------------------------------
// public virtual
QBitmap kpAbstractImageSelection::shapeBitmap (bool nullForRectangular) const
{
    (void) nullForRectangular;

    Q_ASSERT (boundingRect ().isValid ());

    QBitmap maskBitmap (width (), height ());
    maskBitmap.fill (Qt::color0/*transparent*/);

    {
        QPainter painter(&maskBitmap);

        painter.setPen (Qt::color1/*opaque*/);
        painter.setBrush (Qt::color1/*opaque*/);

        QPolygon points = calculatePoints ();
        points.translate (-x (), -y ());

        // Unlike QPainter::drawRect(), this draws the points literally
        // without being 1 pixel wider and higher.  This requires a QPen
        // or it will draw 1 pixel narrower and shorter.
        painter.drawPolygon (points, Qt::OddEvenFill);
    }

    return maskBitmap;
}

//---------------------------------------------------------------------

// public
kpImage kpAbstractImageSelection::givenImageMaskedByShape (const kpImage &image) const
{
#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "kpAbstractImageSelection::givenImageMaskedByShape() boundingRect="
              << boundingRect () << endl;
#endif
    Q_ASSERT (image.width () == width () && image.height () == height ());

    if (isRectangular ()) {
        return image;
    }

    const QRegion mRegion = shapeRegion ().translated (-topLeft ());

#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "\tshapeRegion=" << shapeRegion ()
              << " [rect=" << shapeRegion ().boundingRect () << "]"
              << " calculatePoints=" << calculatePoints ()
              << " [rect=" << calculatePoints ().boundingRect () << "]"
              << endl;
#endif

    kpImage retImage(width (), height (), QImage::Format_ARGB32_Premultiplied);
    retImage.fill(0);  // transparent

    QPainter painter(&retImage);
    painter.setClipRegion(mRegion);
    painter.drawImage(0, 0, image);
    painter.end();

    return retImage;
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
bool kpAbstractImageSelection::hasContent () const
{
    return !d->baseImage.isNull ();
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
void kpAbstractImageSelection::deleteContent ()
{
    if (!hasContent ()) {
        return;
    }

    setBaseImage (kpImage ());
}

//---------------------------------------------------------------------


// public
kpImage kpAbstractImageSelection::baseImage () const
{
    return d->baseImage;
}

//---------------------------------------------------------------------

// public
void kpAbstractImageSelection::setBaseImage (const kpImage &baseImage)
{
    Q_ASSERT (::CanSetBaseImageTo (this, baseImage));

    // qt doc: the image format must be set to Format_ARGB32Premultiplied or Format_ARGB32
    // for the composition modes to have any effect
    d->baseImage = baseImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    recalculateTransparencyMaskCache ();

    emit changed (boundingRect ());
}

//---------------------------------------------------------------------

// public
kpImageSelectionTransparency kpAbstractImageSelection::transparency () const
{
    return d->transparency;
}

//---------------------------------------------------------------------

// public
bool kpAbstractImageSelection::setTransparency (
        const kpImageSelectionTransparency &transparency,
        bool checkTransparentPixmapChanged)
{
    if (d->transparency == transparency) {
        return false;
    }

    d->transparency = transparency;

    bool haveChanged = true;

    QBitmap oldTransparencyMaskCache = d->transparencyMaskCache;
    recalculateTransparencyMaskCache ();

    if ( oldTransparencyMaskCache.size() == d->transparencyMaskCache.size() )
    {
        if (d->transparencyMaskCache.isNull ())
        {
        #if DEBUG_KP_SELECTION
            qCDebug(kpLogLayers) << "\tboth old and new pixmaps are null - nothing changed";
        #endif
            haveChanged = false;
        }
        else if (checkTransparentPixmapChanged)
        {
            QImage oldTransparencyMaskImage = oldTransparencyMaskCache.toImage();
            QImage newTransparencyMaskImage = d->transparencyMaskCache.toImage();

            bool changed = false;
            for (int y = 0; y < oldTransparencyMaskImage.height () && !changed; y++)
            {
                for (int x = 0; x < oldTransparencyMaskImage.width () && !changed; x++)
                {
                    if (kpPixmapFX::getColorAtPixel (oldTransparencyMaskImage, x, y) !=
                        kpPixmapFX::getColorAtPixel (newTransparencyMaskImage, x, y))
                    {
                    #if DEBUG_KP_SELECTION
                        qCDebug(kpLogLayers) << "\tdiffer at " << QPoint (x, y)
                                   << " old=" << kpPixmapFX::getColorAtPixel (oldTransparencyMaskImage, x, y).toQRgb ()
                                   << " new=" << kpPixmapFX::getColorAtPixel (newTransparencyMaskImage, x, y).toQRgb ()
                                   << endl;
                    #endif
                        changed = true;
                        break;
                    }
                }
            }

            if (!changed) {
                haveChanged = false;
            }
        }
    }


    if (haveChanged) {
        emit changed (boundingRect ());
    }

    return haveChanged;
}

//---------------------------------------------------------------------

// private
void kpAbstractImageSelection::recalculateTransparencyMaskCache ()
{
#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "kpAbstractImageSelection::recalculateTransparencyMaskCache()";
#endif

    if (d->baseImage.isNull ())
    {
    #if DEBUG_KP_SELECTION
        qCDebug(kpLogLayers) << "\tno image - no need for transparency mask";
    #endif
        d->transparencyMaskCache = QBitmap ();
        return;
    }

    if (d->transparency.isOpaque ())
    {
    #if DEBUG_KP_SELECTION
        qCDebug(kpLogLayers) << "\topaque - no need for transparency mask";
    #endif
        d->transparencyMaskCache = QBitmap ();
        return;
    }

    d->transparencyMaskCache = QBitmap(d->baseImage.size());

    QPainter transparencyMaskPainter (&d->transparencyMaskCache);

    bool hasTransparent = false;
    for (int y = 0; y < d->baseImage.height (); y++)
    {
        for (int x = 0; x < d->baseImage.width (); x++)
        {
            const kpColor pixelCol = kpPixmapFX::getColorAtPixel (d->baseImage, x, y);
            if (pixelCol == kpColor::Transparent ||
                pixelCol.isSimilarTo (d->transparency.transparentColor (),
                                      d->transparency.processedColorSimilarity ()))
            {
                transparencyMaskPainter.setPen (Qt::color1/*transparent*/);
                hasTransparent = true;
            }
            else
            {
                transparencyMaskPainter.setPen (Qt::color0/*opaque*/);
            }

            transparencyMaskPainter.drawPoint (x, y);
        }
    }

    transparencyMaskPainter.end ();

    if (!hasTransparent)
    {
    #if DEBUG_KP_SELECTION
        qCDebug(kpLogLayers) << "\tcolour useless - completely opaque";
    #endif
        d->transparencyMaskCache = QBitmap ();
        return;
    }
}

//---------------------------------------------------------------------

// public
kpImage kpAbstractImageSelection::transparentImage () const
{
    kpImage image = baseImage ();

    if (!d->transparencyMaskCache.isNull ())
    {
      QPainter painter(&image);
      painter.setCompositionMode(QPainter::CompositionMode_Clear);
      painter.drawPixmap(0, 0, d->transparencyMaskCache);
    }

    return image;
}

//---------------------------------------------------------------------

// public
void kpAbstractImageSelection::fill (const kpColor &color)
{
    QImage newImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
    newImage.fill(color.toQRgb());

    // LOTODO: Maybe disable Image/Clear menu item if transparent color
    if ( !color.isTransparent() )
    {
      QPainter painter(&newImage);
      painter.setCompositionMode(QPainter::CompositionMode_Clear);
      painter.drawPixmap(0, 0, shapeBitmap());
    }

    setBaseImage (newImage);
}

//---------------------------------------------------------------------

// public virtual
void kpAbstractImageSelection::flip (bool horiz, bool vert)
{
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpAbstractImageSelection::flip(horiz=" << horiz
               << ",vert=" << vert << ")";
#endif

    if (!d->baseImage.isNull ())
    {
    #if DEBUG_KP_SELECTION && 1
        qCDebug(kpLogLayers) << "\thave pixmap - flipping that";
    #endif
        d->baseImage = d->baseImage.mirrored(horiz, vert);
    }

    if (!d->transparencyMaskCache.isNull ())
    {
    #if DEBUG_KP_SELECTION && 1
        qCDebug(kpLogLayers) << "\thave transparency mask - flipping that";
    #endif
        QImage image = d->transparencyMaskCache.toImage().mirrored(horiz, vert);
        d->transparencyMaskCache = QBitmap::fromImage(image);
    }

    emit changed (boundingRect ());
}

//---------------------------------------------------------------------

static void Paint (const kpAbstractImageSelection *sel, const kpImage &srcImage,
                   QImage *destImage, const QRect &docRect)
{
    if (!srcImage.isNull ())
    {
        kpPixmapFX::paintPixmapAt (destImage,
                                   sel->topLeft () - docRect.topLeft (),
                                   srcImage);
    }
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
void kpAbstractImageSelection::paint (QImage *destImage,
        const QRect &docRect) const
{
    ::Paint (this, transparentImage (), destImage, docRect);
}

//---------------------------------------------------------------------

// public
void kpAbstractImageSelection::paintWithBaseImage (QImage *destImage,
        const QRect &docRect) const
{
    ::Paint (this, baseImage (), destImage, docRect);
}

//---------------------------------------------------------------------


