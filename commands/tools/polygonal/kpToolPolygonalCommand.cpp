
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_POLYGON 0

#include "kpToolPolygonalCommand.h"

#include "document/kpDocument.h"
#include "imagelib/kpImage.h"
#include "kpDefs.h"

struct kpToolPolygonalCommandPrivate {
    kpToolPolygonalBase::DrawShapeFunc drawShapeFunc{};

    QPolygon points;
    QRect boundingRect;

    kpColor fcolor;
    int penWidth{};
    kpColor bcolor;

    kpImage oldImage;
};

kpToolPolygonalCommand::kpToolPolygonalCommand(const QString &name,
                                               kpToolPolygonalBase::DrawShapeFunc drawShapeFunc,
                                               const QPolygon &points,
                                               const QRect &boundingRect,
                                               const kpColor &fcolor,
                                               int penWidth,
                                               const kpColor &bcolor,
                                               kpCommandEnvironment *environ)

    : kpNamedCommand(name, environ)
    , d(new kpToolPolygonalCommandPrivate())
{
    d->drawShapeFunc = drawShapeFunc;

    d->points = points;
    d->boundingRect = boundingRect;

    d->fcolor = fcolor;
    d->penWidth = penWidth;
    d->bcolor = bcolor;
}

kpToolPolygonalCommand::~kpToolPolygonalCommand()
{
    delete d;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolPolygonalCommand::size() const
{
    return PolygonSize(d->points) + ImageSize(d->oldImage);
}

// public virtual [base kpCommand]
void kpToolPolygonalCommand::execute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    // Store Undo info.
    Q_ASSERT(d->oldImage.isNull());
    d->oldImage = doc->getImageAt(d->boundingRect);

    // Invoke shape drawing function passed in ctor.
    kpImage image = d->oldImage;

    QPolygon pointsTranslated = d->points;
    pointsTranslated.translate(-d->boundingRect.x(), -d->boundingRect.y());

    (*d->drawShapeFunc)(&image, pointsTranslated, d->fcolor, d->penWidth, d->bcolor, true /*final shape*/);

    doc->setImageAt(image, d->boundingRect.topLeft());
}

// public virtual [base kpCommand]
void kpToolPolygonalCommand::unexecute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    Q_ASSERT(!d->oldImage.isNull());
    doc->setImageAt(d->oldImage, d->boundingRect.topLeft());

    d->oldImage = kpImage();
}
