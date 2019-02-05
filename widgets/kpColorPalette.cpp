
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


#define DEBUG_KP_COLOR_PALETTE 0


#include "kpColorPalette.h"

#include "kpColorCells.h"
#include "kpTransparentColorCell.h"

#include <QBoxLayout>

//---------------------------------------------------------------------

struct kpColorPalettePrivate
{
    Qt::Orientation orientation;

    QBoxLayout *boxLayout;

    kpTransparentColorCell *transparentColorCell;

    kpColorCells *colorCells;
};

//---------------------------------------------------------------------

kpColorPalette::kpColorPalette (QWidget *parent, Qt::Orientation o)
    : QWidget (parent),
      d (new kpColorPalettePrivate ())
{
    d->boxLayout = nullptr;

    d->transparentColorCell = new kpTransparentColorCell (this);
    connect (d->transparentColorCell, &kpTransparentColorCell::foregroundColorChanged,
             this, &kpColorPalette::foregroundColorChanged);

    connect (d->transparentColorCell, &kpTransparentColorCell::backgroundColorChanged,
             this, &kpColorPalette::backgroundColorChanged);

    d->colorCells = new kpColorCells (this);
    connect (d->colorCells, &kpColorCells::foregroundColorChanged,
             this, &kpColorPalette::foregroundColorChanged);

    connect (d->colorCells, &kpColorCells::backgroundColorChanged,
             this, &kpColorPalette::backgroundColorChanged);

    setOrientation (o);
}

//---------------------------------------------------------------------

kpColorPalette::~kpColorPalette ()
{
    delete d;
}

//---------------------------------------------------------------------

// public
Qt::Orientation kpColorPalette::orientation () const
{
    return d->orientation;
}

//---------------------------------------------------------------------

void kpColorPalette::setOrientation (Qt::Orientation o)
{
    d->colorCells->setOrientation (o);

    delete d->boxLayout;

    if (o == Qt::Horizontal)
    {
        d->boxLayout = new QBoxLayout (QBoxLayout::LeftToRight, this);
        d->boxLayout->addWidget (d->transparentColorCell, 0/*stretch*/, Qt::AlignTop);
        d->boxLayout->addWidget (d->colorCells);
    }
    else
    {
        d->boxLayout = new QBoxLayout (QBoxLayout::TopToBottom, this);
        d->boxLayout->addWidget (d->transparentColorCell, 0/*stretch*/, Qt::AlignTop);
        d->boxLayout->addWidget (d->colorCells);
    }
    d->boxLayout->setSpacing (5);

    d->orientation = o;
}

//---------------------------------------------------------------------

// public
kpColorCells *kpColorPalette::colorCells () const
{
    return d->colorCells;
}

//---------------------------------------------------------------------

