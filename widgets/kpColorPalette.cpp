
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


#include <kpColorPalette.h>

#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>

#include <KColorDialog>
#include <KLocale>

#include <kpColorCells.h>
#include <kpTransparentColorCell.h>


struct kpColorPalettePrivate
{
    Qt::Orientation orientation;

    QGridLayout *gridLayout;

    kpTransparentColorCell *transparentColorCell;

    QScrollArea *colorCellsScroll;
    kpColorCells *colorCells;

    QPushButton *colorCellsExpandButton, *colorCellsShrinkButton;
};

kpColorPalette::kpColorPalette (QWidget *parent, Qt::Orientation o)
    : QWidget (parent),
      d (new kpColorPalettePrivate ())
{
#if DEBUG_KP_COLOR_PALETTE
    kDebug () << "kpColorPalette::kpColorPalette()";
#endif

    d->gridLayout = 0;

    d->transparentColorCell = new kpTransparentColorCell (this);
    connect (d->transparentColorCell, SIGNAL (foregroundColorChanged (const kpColor &)),
             this, SIGNAL (foregroundColorChanged (const kpColor &)));
    connect (d->transparentColorCell, SIGNAL (backgroundColorChanged (const kpColor &)),
             this, SIGNAL (backgroundColorChanged (const kpColor &)));

    d->colorCellsScroll = new QScrollArea (this);
    d->colorCellsScroll->setSizePolicy (QSizePolicy::Minimum/*horizontal*/,
        QSizePolicy::Minimum/*vertical*/);

    d->colorCells = new kpColorCells (d->colorCellsScroll);
    connect (d->colorCells, SIGNAL (foregroundColorChanged (const kpColor &)),
             this, SIGNAL (foregroundColorChanged (const kpColor &)));
    connect (d->colorCells, SIGNAL (backgroundColorChanged (const kpColor &)),
             this, SIGNAL (backgroundColorChanged (const kpColor &)));

    d->colorCellsScroll->setWidget (d->colorCells);
    d->colorCellsScroll->setAlignment (Qt::AlignHCenter);

    // TODO: Use QScrollbar's arrows.

    d->colorCellsExpandButton = new QPushButton (i18nc ("up arrow", "^^^"), this);
    d->colorCellsExpandButton->setFixedHeight (15);
    connect (d->colorCellsExpandButton, SIGNAL (clicked ()),
             SLOT (expandColorCells ()));

    d->colorCellsShrinkButton = new QPushButton (i18nc ("down arrow", "vvv"), this);
    d->colorCellsShrinkButton->setFixedHeight (15);
    connect (d->colorCellsShrinkButton, SIGNAL (clicked ()),
             SLOT (shrinkColorCells ()));

    setOrientation (o);
}

kpColorPalette::~kpColorPalette ()
{
    delete d;
}

// public
Qt::Orientation kpColorPalette::orientation () const
{
    return d->orientation;
}

void kpColorPalette::setOrientation (Qt::Orientation o)
{
    d->colorCells->setOrientation (o);

    delete d->gridLayout;

    if (o == Qt::Horizontal)
    {
        d->gridLayout = new QGridLayout (this);
        d->gridLayout->addWidget (d->transparentColorCell,
            0/*row*/, 0/*col*/, 2/*row span*/, 1/*col span*/,
            Qt::AlignVCenter);
        d->gridLayout->setColumnStretch (0/*column*/, 0);

        d->gridLayout->addWidget (d->colorCellsScroll,
            0/*row*/, 1/*col*/, 1/*row span*/, 2/*col span*/);

        // Even though the buttons feel like they should be above the color
        // cells: Since the Color Tool Bar is normally at the
        // bottom of the window, the vertical resizes don't move the buttons
        // if they are placed below the color cells.  This allows you to click
        // the expand or shrink buttons several times in succession, without
        // repositioning the mouse.
        //
        // TODO; Once we really start supporting moving and undocking the Color
        //       Tool Bar again, we should move these buttons depending on
        //       the position of the toolbar.
        d->gridLayout->addWidget (d->colorCellsExpandButton,
            1/*row*/, 1/*col*/, 1/*row span*/, 1/*col span*/);
        d->gridLayout->addWidget (d->colorCellsShrinkButton,
            1/*row*/, 2/*col*/, 1/*row span*/, 1/*col span*/);
    }
    else
    {
        Q_ASSERT (!"unimplemented");
    }

    d->gridLayout->setSpacing (5);

    d->orientation = o;
}


// public
kpColorCells *kpColorPalette::colorCells () const
{
    return d->colorCells;
}


// private slot
void kpColorPalette::expandColorCells ()
{
    const int newWidth = d->colorCellsScroll->width ();
    const int newHeight = d->colorCellsScroll->height () + 26;

    d->colorCellsScroll->setMinimumSize (newWidth, newHeight);
}

// private slot
void kpColorPalette::shrinkColorCells ()
{
    const int newWidth = d->colorCellsScroll->width ();
    const int newHeight = d->colorCellsScroll->height () - 26;

    d->colorCellsScroll->setMinimumSize (newWidth, newHeight);
}


#include <kpColorPalette.moc>
