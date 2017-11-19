
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


#ifndef kpAbstractImageSelection_H
#define kpAbstractImageSelection_H


#include "kpImageSelectionTransparency.h"
#include "layers/selections/kpAbstractSelection.h"
#include "imagelib/kpImage.h"


//
// An abstract selection with optional image content and background
// subtraction.  If there is image content, it is known as a "floating
// selection" that hovers above the document.  Otherwise, it is an
// image selection "border", that highlights pixels of the document, and
// may later be upgraded to a floating selection by giving it an image
// consisting of those pixels.
//
// The images passed to this class (known as "base images") should have all
// pixels, outside of the border, set to transparent.  However, nothing
// enforces this.  Pixels on, or inside, the border might be opaque or
// transparent, depending on the content.
//
// The "transparent image" is the base image with background subtraction
// (kpImageSelectionTransparency) applied.  This is automatically computed.
//
// The boundingRect() is the size of the border.  The base image must be of
// exactly the same size, except that the base image is allowed to be null
// (for a selection that only consists of a border).
//
// Instead of copying selections' images to the clipboard, we copy
// selections, to preserve the border across KolourPaint instances.
// Background subtraction is not copied to the clipboard so that the base
// image is affected by the background subtraction of the destination
// KolourPaint window.
//
class kpAbstractImageSelection : public kpAbstractSelection
{
Q_OBJECT

//
// Initialization
//

protected:
    // (Call these in subclass constructors)
    kpAbstractImageSelection (const kpImageSelectionTransparency &transparency =
        kpImageSelectionTransparency ());

    kpAbstractImageSelection (const QRect &rect,
        const kpImage &baseImage = kpImage (),
        const kpImageSelectionTransparency &transparency =
            kpImageSelectionTransparency ());

    kpAbstractImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency =
            kpImageSelectionTransparency ());

    // (Call this in subclass implementations of operator=)
    kpAbstractImageSelection &operator= (const kpAbstractImageSelection &rhs);

public:
    // (Covariant return-type specialization of superclass pure virtual method)
    kpAbstractImageSelection *clone () const override = 0;

    ~kpAbstractImageSelection () override;


//
// Marshalling
//

public:
    // You must override this if you have extra serializable fields.
    // Remember to call this base implementation before your code.
    bool readFromStream (QDataStream &stream) override;

    // You must override this if you have extra serializable fields.
    // Remember to call this base implementation before your code.
    void writeToStream (QDataStream &stream) const override;


//
// General Queries
//

public:
    QString name () const override;

    // You must override this, if you have extra fields that take a
    // non-constant amount of space, and add the size returned by this
    // implementation.
    kpCommandSize::SizeType size () const override;

    // Same as virtual size() (it even calls it) but subtracts the size of the
    // baseImage().
    //
    // kpCommand's store the kpImage's they are working on.  These images may
    // be from documents or selections.  In the case of a selection, the
    // selection's baseImage() is identical to that image, but takes no extra
    // space due to kpImage's copy-on-write.  This method fixes that
    // double-counting of baseImage()'s size.
    //
    // The size of the internal transparency() mask is still included
    // (see recalculateTransparencyMask()).
    //
    // sync: kpImage copy-on-write behavior
    //
    // TODO: Check all size() implementations are correct since we've
    //       started removing the old kpSelection::setPixmap(QPixmap())
    //       (now kpAbstractImageSelection::setBaseImage(kpImage()) or
    //        kpAbstractImageSelection::deleteContent()) space saving hack.
    kpCommandSize::SizeType sizeWithoutImage () const;


//
// Dimensions
//

public:
    int minimumWidth () const override;
    int minimumHeight () const override;


//
// Shape Mask
//
// These methods do not access any class instance fields.
//

public:
    // Returns the mask corresponding to the shape of the selection.
    //
    // If <nullForRectangular> is set, the method _may_ return a null
    // bitmap if the selection is rectangular.
    //
    // This base implementation calls calculatePoints() and ignores
    // <nullForRectangular>.
    //
    // You should override this if you can implement it more efficiently or
    // if you can honor <nullForRectangular>.
    //
    // Note: This must be consistent with the outputs of calculatePoints() and
    //       shapeRegion().
    //
    // TODO: Try to get rid of this method since it's slow.
    virtual QBitmap shapeBitmap (bool nullForRectangular = false) const;

    // Returns the region corresponding to the shape of the selection
    // e.g. elliptical region for an elliptical selection.
    //
    // Very slow.
    //
    // Note: This must be consistent with the outputs of calculatePoints() and
    //       shapeRegion().
    //
    // OPT: QRegion is probably incredibly slow - cache
    virtual QRegion shapeRegion () const = 0;

    // Returns the given <image> with the pixels outside of the selection's
    // shape set to transparent.
    //
    // Very slow.
    //
    // ASSUMPTION: The image has the same dimensions as the selection.
    kpImage givenImageMaskedByShape (const kpImage &image) const;


//
// Content - Base Image
//

public:
    // Returns whether there's a non-null base image.
    bool hasContent () const override;

    void deleteContent () override;

public:
    kpImage baseImage () const;
    void setBaseImage (const kpImage &baseImage);


//
// Background Subtraction
//

public:
    kpImageSelectionTransparency transparency () const;

    // Returns whether or not the selection changed due to setting the
    // transparency info.  If <checkTransparentPixmapChanged> is set,
    // it will try harder to return false (although the check is
    // expensive).
    bool setTransparency (const kpImageSelectionTransparency &transparency,
                          bool checkTransparentPixmapChanged = false);

private:
    // Updates the selection transparency (a.k.a. background subtraction) mask
    // so that transparentImage() will work.
    //
    // Called when the base image or selection transparency changes.
    void recalculateTransparencyMaskCache ();

public:
    // Returns baseImage() after applying kpImageSelectionTransparency
    kpImage transparentImage () const;


//
// Mutation - Effects
//

public:
    // Overwrites the base image with the selection's shape (e.g. ellipse)
    // filled in with <color>.  See shapeBitmap().
    void fill (const kpColor &color);

    virtual void flip (bool horiz, bool vert);


//
// Rendering
//

public:
    // (using transparent image)
    void paint (QImage *destPixmap, const QRect &docRect) const override;

    // (using base image)
    void paintWithBaseImage (QImage *destPixmap, const QRect &docRect) const;


private:
    struct kpAbstractImageSelectionPrivate * const d;
};


#endif  // kpAbstractImageSelection_H
