
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


#define DEBUG_KP_TOOL_RECTANGULAR_COMMAND 0


#include "kpToolRectangularCommand.h"

#include "imagelib/kpColor.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "layers/tempImage/kpTempImage.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetFillStyle.h"
#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "kpLogCategories.h"


struct kpToolRectangularCommandPrivate
{
    kpToolRectangularBase::DrawShapeFunc drawShapeFunc{};

    QRect rect;

    kpColor fcolor;
    int penWidth{};
    kpColor bcolor;

    kpImage oldImage;
};

kpToolRectangularCommand::kpToolRectangularCommand (const QString &name,
        kpToolRectangularBase::DrawShapeFunc drawShapeFunc,
        const QRect &rect,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        kpCommandEnvironment *environ)

    : kpNamedCommand (name, environ),
      d (new kpToolRectangularCommandPrivate ())
{
    d->drawShapeFunc = drawShapeFunc;

    d->rect = rect;

    d->fcolor = fcolor;
    d->penWidth = penWidth;
    d->bcolor = bcolor;
}

kpToolRectangularCommand::~kpToolRectangularCommand ()
{
    delete d;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolRectangularCommand::size () const
{
    return ImageSize (d->oldImage);
}


// public virtual [base kpCommand]
void kpToolRectangularCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    // Store Undo info.
    // OPT: For a pure rectangle, can do better if there is no bcolor, by only
    //      saving 4 pixmaps corresponding to the pixels dirtied by the 4 edges.
    Q_ASSERT (d->oldImage.isNull ());
    d->oldImage = doc->getImageAt (d->rect);

    // Invoke shape drawing function passed in ctor.
    kpImage image = d->oldImage;
    (*d->drawShapeFunc) (&image,
        0, 0, d->rect.width (), d->rect.height (),
        d->fcolor, d->penWidth,
        d->bcolor);

    doc->setImageAt (image, d->rect.topLeft ());
}

// public virtual [base kpCommand]
void kpToolRectangularCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    Q_ASSERT (!d->oldImage.isNull ());
    doc->setImageAt (d->oldImage, d->rect.topLeft ());

    d->oldImage = kpImage ();
}

