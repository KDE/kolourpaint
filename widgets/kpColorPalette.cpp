
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_COLOR_PALETTE 0

#include "kpColorPalette.h"

#include "kpColorCells.h"
#include "kpTransparentColorCell.h"

#include <QBoxLayout>

//---------------------------------------------------------------------

struct kpColorPalettePrivate {
    Qt::Orientation orientation;

    QBoxLayout *boxLayout;

    kpTransparentColorCell *transparentColorCell;

    kpColorCells *colorCells;
};

//---------------------------------------------------------------------

kpColorPalette::kpColorPalette(QWidget *parent, Qt::Orientation o)
    : QWidget(parent)
    , d(new kpColorPalettePrivate())
{
    d->boxLayout = nullptr;

    d->transparentColorCell = new kpTransparentColorCell(this);
    connect(d->transparentColorCell, &kpTransparentColorCell::foregroundColorChanged, this, &kpColorPalette::foregroundColorChanged);

    connect(d->transparentColorCell, &kpTransparentColorCell::backgroundColorChanged, this, &kpColorPalette::backgroundColorChanged);

    d->colorCells = new kpColorCells(this);
    connect(d->colorCells, &kpColorCells::foregroundColorChanged, this, &kpColorPalette::foregroundColorChanged);

    connect(d->colorCells, &kpColorCells::backgroundColorChanged, this, &kpColorPalette::backgroundColorChanged);

    setOrientation(o);
}

//---------------------------------------------------------------------

kpColorPalette::~kpColorPalette()
{
    delete d;
}

//---------------------------------------------------------------------

// public
Qt::Orientation kpColorPalette::orientation() const
{
    return d->orientation;
}

//---------------------------------------------------------------------

void kpColorPalette::setOrientation(Qt::Orientation o)
{
    d->colorCells->setOrientation(o);

    delete d->boxLayout;

    if (o == Qt::Horizontal) {
        d->boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
        d->boxLayout->addWidget(d->transparentColorCell, 0 /*stretch*/, Qt::AlignTop);
        d->boxLayout->addWidget(d->colorCells);
    } else {
        d->boxLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
        d->boxLayout->addWidget(d->transparentColorCell, 0 /*stretch*/, Qt::AlignTop);
        d->boxLayout->addWidget(d->colorCells);
    }
    d->boxLayout->setSpacing(5);

    d->orientation = o;
}

//---------------------------------------------------------------------

// public
kpColorCells *kpColorPalette::colorCells() const
{
    return d->colorCells;
}

//---------------------------------------------------------------------

#include "moc_kpColorPalette.cpp"
