
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_RECTANGULAR_COMMAND 0

#include "kpToolRectangularCommand.h"

#include "document/kpDocument.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpPainter.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "layers/tempImage/kpTempImage.h"
#include "pixmapfx/kpPixmapFX.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetFillStyle.h"
#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"

struct kpToolRectangularCommandPrivate {
    kpToolRectangularBase::DrawShapeFunc drawShapeFunc{};

    QRect rect;

    kpColor fcolor;
    int penWidth{};
    kpColor bcolor;

    kpImage oldImage;
};

kpToolRectangularCommand::kpToolRectangularCommand(const QString &name,
                                                   kpToolRectangularBase::DrawShapeFunc drawShapeFunc,
                                                   const QRect &rect,
                                                   const kpColor &fcolor,
                                                   int penWidth,
                                                   const kpColor &bcolor,
                                                   kpCommandEnvironment *environ)

    : kpNamedCommand(name, environ)
    , d(new kpToolRectangularCommandPrivate())
{
    d->drawShapeFunc = drawShapeFunc;

    d->rect = rect;

    d->fcolor = fcolor;
    d->penWidth = penWidth;
    d->bcolor = bcolor;
}

kpToolRectangularCommand::~kpToolRectangularCommand()
{
    delete d;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolRectangularCommand::size() const
{
    return ImageSize(d->oldImage);
}

// public virtual [base kpCommand]
void kpToolRectangularCommand::execute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    // Store Undo info.
    // OPT: For a pure rectangle, can do better if there is no bcolor, by only
    //      saving 4 pixmaps corresponding to the pixels dirtied by the 4 edges.
    Q_ASSERT(d->oldImage.isNull());
    d->oldImage = doc->getImageAt(d->rect);

    // Invoke shape drawing function passed in ctor.
    kpImage image = d->oldImage;
    (*d->drawShapeFunc)(&image, 0, 0, d->rect.width(), d->rect.height(), d->fcolor, d->penWidth, d->bcolor);

    doc->setImageAt(image, d->rect.topLeft());
}

// public virtual [base kpCommand]
void kpToolRectangularCommand::unexecute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    Q_ASSERT(!d->oldImage.isNull());
    doc->setImageAt(d->oldImage, d->rect.topLeft());

    d->oldImage = kpImage();
}
