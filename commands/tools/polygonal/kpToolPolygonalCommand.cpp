
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


#define DEBUG_KP_TOOL_POLYGON 0


#include "kpToolPolygonalCommand.h"

#include "document/kpDocument.h"
#include "kpDefs.h"
#include "imagelib/kpImage.h"


struct kpToolPolygonalCommandPrivate
{
    kpToolPolygonalBase::DrawShapeFunc drawShapeFunc{};

    QPolygon points;
    QRect boundingRect;

    kpColor fcolor;
    int penWidth{};
    kpColor bcolor;

    kpImage oldImage;
};

kpToolPolygonalCommand::kpToolPolygonalCommand (const QString &name,
        kpToolPolygonalBase::DrawShapeFunc drawShapeFunc,
        const QPolygon &points,
        const QRect &boundingRect,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        kpCommandEnvironment *environ)

    : kpNamedCommand (name, environ),
      d (new kpToolPolygonalCommandPrivate ())
{
    d->drawShapeFunc = drawShapeFunc;

    d->points = points;
    d->boundingRect = boundingRect;

    d->fcolor = fcolor;
    d->penWidth = penWidth;
    d->bcolor = bcolor;
}

kpToolPolygonalCommand::~kpToolPolygonalCommand ()
{
    delete d;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolPolygonalCommand::size () const
{
    return PolygonSize (d->points) +
           ImageSize (d->oldImage);
}

// public virtual [base kpCommand]
void kpToolPolygonalCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    // Store Undo info.
    Q_ASSERT (d->oldImage.isNull ());
    d->oldImage = doc->getImageAt (d->boundingRect);

    // Invoke shape drawing function passed in ctor.
    kpImage image = d->oldImage;

    QPolygon pointsTranslated = d->points;
    pointsTranslated.translate (-d->boundingRect.x (), -d->boundingRect.y ());

    (*d->drawShapeFunc) (&image,
        pointsTranslated,
        d->fcolor, d->penWidth,
        d->bcolor,
        true/*final shape*/);

    doc->setImageAt (image, d->boundingRect.topLeft ());
}

// public virtual [base kpCommand]
void kpToolPolygonalCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    Q_ASSERT (!d->oldImage.isNull ());
    doc->setImageAt (d->oldImage, d->boundingRect.topLeft ());

    d->oldImage = kpImage ();
}

