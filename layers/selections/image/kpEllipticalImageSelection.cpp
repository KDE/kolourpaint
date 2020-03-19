
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


#include "layers/selections/image/kpEllipticalImageSelection.h"

#include <QPainter>
#include <QRegion>
#include <QPainterPath>


struct kpEllipticalImageSelectionPrivate
{
};


kpEllipticalImageSelection::kpEllipticalImageSelection (
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (transparency),
      d (new kpEllipticalImageSelectionPrivate ())
{
}

kpEllipticalImageSelection::kpEllipticalImageSelection (const QRect &rect,
        const kpImage &baseImage,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (rect, baseImage, transparency),
      d (new kpEllipticalImageSelectionPrivate ())
{
}

kpEllipticalImageSelection::kpEllipticalImageSelection (const QRect &rect,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (rect, transparency),
      d (new kpEllipticalImageSelectionPrivate ())
{
}

kpEllipticalImageSelection::kpEllipticalImageSelection (const kpEllipticalImageSelection &rhs)
    : kpAbstractImageSelection (),
      d (new kpEllipticalImageSelectionPrivate ())
{
    *this = rhs;
}

kpEllipticalImageSelection &kpEllipticalImageSelection::operator= (
        const kpEllipticalImageSelection &rhs)
{
    kpAbstractImageSelection::operator= (rhs);

    return *this;
}

kpEllipticalImageSelection *kpEllipticalImageSelection::clone () const
{
    kpEllipticalImageSelection *sel = new kpEllipticalImageSelection ();
    *sel = *this;
    return sel;
}

kpEllipticalImageSelection::~kpEllipticalImageSelection ()
{
    delete d;
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
int kpEllipticalImageSelection::serialID () const
{
    return SerialID;
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
bool kpEllipticalImageSelection::isRectangular () const
{
    return false;
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
QPolygon kpEllipticalImageSelection::calculatePoints () const
{
    Q_ASSERT (boundingRect ().isValid ());

    if (width () == 1 && height () == 1)
    {
        QPolygon ret;
        ret.append (topLeft ());
        return ret;
    }

    QPainterPath path;
    if (width () == 1 || height () == 1)
    {
        path.moveTo (x (), y ());
        // This does not work when the width _and_ height are 1 since lineTo()
        // would not move at all.  This is why we have a separate case for that
        // at the top of the method.
        path.lineTo (x () + width () - 1, y () + height () - 1);
    }
    else
    {
        // The adjusting is to fight QPainterPath::addEllipse() making
        // the ellipse 1 pixel higher and wider than specified.
        path.addEllipse (boundingRect ().adjusted (0, 0, -1, -1));
    }

    const QList <QPolygonF> polygons = path.toSubpathPolygons ();
    Q_ASSERT (polygons.size () == 1);

    const QPolygonF& firstPolygonF = polygons.first ();
    return firstPolygonF.toPolygon ();
}

//---------------------------------------------------------------------


// protected virtual [kpAbstractImageSelection]
QRegion kpEllipticalImageSelection::shapeRegion () const
{
    QRegion reg(calculatePoints());
    return reg;
}

//---------------------------------------------------------------------


// public virtual [kpAbstractSelection]
bool kpEllipticalImageSelection::contains (const QPoint &point) const
{
    if (!boundingRect ().contains (point)) {
        return false;
    }

    return shapeRegion ().contains (point);
}

//---------------------------------------------------------------------


// public virtual [kpAbstractSelection]
void kpEllipticalImageSelection::paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const
{
    if ( !boundingRect().isValid() ) {
      return;
    }

    paintPolygonalBorder (calculatePoints (),
        destPixmap, docRect,
        selectionFinished);
}


