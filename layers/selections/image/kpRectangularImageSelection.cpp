
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


#include "layers/selections/image/kpRectangularImageSelection.h"

#include <QBitmap>
#include <QRegion>


struct kpRectangularImageSelectionPrivate
{
};


kpRectangularImageSelection::kpRectangularImageSelection (
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (transparency),
      d (new kpRectangularImageSelectionPrivate ())
{
}

kpRectangularImageSelection::kpRectangularImageSelection (const QRect &rect,
        const kpImage &baseImage,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (rect, baseImage, transparency),
      d (new kpRectangularImageSelectionPrivate ())
{
}

kpRectangularImageSelection::kpRectangularImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (rect, transparency),
      d (new kpRectangularImageSelectionPrivate ())
{
}

kpRectangularImageSelection::kpRectangularImageSelection (const kpRectangularImageSelection &rhs)
    : kpAbstractImageSelection (),
      d (new kpRectangularImageSelectionPrivate ())
{
    *this = rhs;
}

kpRectangularImageSelection &kpRectangularImageSelection::operator= (
        const kpRectangularImageSelection &rhs)
{
    kpAbstractImageSelection::operator= (rhs);

    return *this;
}

kpRectangularImageSelection *kpRectangularImageSelection::clone () const
{
    kpRectangularImageSelection *sel = new kpRectangularImageSelection ();
    *sel = *this;
    return sel;
}

kpRectangularImageSelection::~kpRectangularImageSelection ()
{
    delete d;
}


// public virtual [kpAbstractSelection]
int kpRectangularImageSelection::serialID () const
{
    return SerialID;
}


// public virtual [kpAbstractSelection]
bool kpRectangularImageSelection::isRectangular () const
{
    return true;
}


// public virtual [kpAbstractSelection]
QPolygon kpRectangularImageSelection::calculatePoints () const
{
    return kpAbstractImageSelection::CalculatePointsForRectangle (boundingRect ());
}


// public virtual [base kpAbstractImageSelection]
QBitmap kpRectangularImageSelection::shapeBitmap (bool nullForRectangular) const
{
    Q_ASSERT (boundingRect ().isValid ());

    if (nullForRectangular) {
        return {};
    }

    QBitmap maskBitmap (width (), height ());
    maskBitmap.fill (Qt::color1/*opaque*/);
    return maskBitmap;
}

// public virtual [kpAbstractImageSelection]
QRegion kpRectangularImageSelection::shapeRegion () const
{
    return QRegion (boundingRect (), QRegion::Rectangle);
}


// public virtual [kpAbstractSelection]
bool kpRectangularImageSelection::contains (const QPoint &point) const
{
    return boundingRect ().contains (point);
}


// public virtual [kpAbstractSelection]
void kpRectangularImageSelection::paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const
{
    paintRectangularBorder (destPixmap, docRect, selectionFinished);
}


