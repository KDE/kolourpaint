
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


#ifndef kpEllipticalImageSelection_H
#define kpEllipticalImageSelection_H


#include "layers/selections/image/kpAbstractImageSelection.h"


class kpEllipticalImageSelection : public kpAbstractImageSelection
{
Q_OBJECT

public:
    kpEllipticalImageSelection (const kpImageSelectionTransparency &transparency =
        kpImageSelectionTransparency ());

    kpEllipticalImageSelection (const QRect &rect,
        const kpImage &baseImage = kpImage (),
        const kpImageSelectionTransparency &transparency =
            kpImageSelectionTransparency ());

    kpEllipticalImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency =
            kpImageSelectionTransparency ());

    kpEllipticalImageSelection (const kpEllipticalImageSelection &rhs);

    kpEllipticalImageSelection &operator= (const kpEllipticalImageSelection &rhs);

    kpEllipticalImageSelection *clone () const override;

    ~kpEllipticalImageSelection () override;


//
// Marshalling
//

public:
    static const int SerialID = 1;
    int serialID () const override;


//
// General Queries
//

public:
    bool isRectangular () const override;


//
// Position & Dimensions
//

public:
   QPolygon calculatePoints () const override;


//
// Shape Mask
//

public:
    QRegion shapeRegion () const override;


//
// Point Testing
//

public:
    bool contains (const QPoint &point) const override;


//
// Rendering
//

public:
    void paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const override;


private:
    struct kpEllipticalImageSelectionPrivate * const d;
};



#endif  // kpEllipticalImageSelection_H
