
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


#ifndef kpRectangularImageSelection_H
#define kpRectangularImageSelection_H


#include <kpAbstractImageSelection.h>


class kpRectangularImageSelection : public kpAbstractImageSelection
{
Q_OBJECT

public:
    kpRectangularImageSelection (const kpImageSelectionTransparency &transparency =
        kpImageSelectionTransparency ());

    kpRectangularImageSelection (const QRect &rect,
        const kpImage &baseImage = kpImage (),
        const kpImageSelectionTransparency &transparency =
            kpImageSelectionTransparency ());

    kpRectangularImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency =
            kpImageSelectionTransparency ());

    kpRectangularImageSelection (const kpRectangularImageSelection &rhs);

    kpRectangularImageSelection &operator= (const kpRectangularImageSelection &rhs);

    virtual kpRectangularImageSelection *clone () const;

    virtual ~kpRectangularImageSelection ();


//
// Marshalling
//

public:
    static const int SerialID = 0;
    virtual int serialID () const;


//
// General Queries
//

public:
    virtual bool isRectangular () const;


//
// Position & Dimensions
//

public:
    virtual QPolygon calculatePoints () const;


//
// Shape Mask
//

public:
    virtual QBitmap shapeBitmap (bool nullForRectangular = false) const;

    virtual QRegion shapeRegion () const;


//
// Point Testing
//

public:
    virtual bool contains (const QPoint &point) const;


//
// Rendering
//

public:
    virtual void paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const;


private:
    struct kpRectangularImageSelectionPrivate * const d;
};


#endif  // kpRectangularImageSelection_H
