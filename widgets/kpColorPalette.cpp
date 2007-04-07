
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

#include <QBoxLayout>

#include <kpColorCells.h>
#include <kpTransparentColorCell.h>


kpColorPalette::kpColorPalette (QWidget *parent,
                                Qt::Orientation o)
    : QWidget (parent),
      m_boxLayout (0)
{
#if DEBUG_KP_COLOR_PALETTE
    kDebug () << "kpColorPalette::kpColorPalette()" << endl;
#endif

    m_transparentColorCell = new kpTransparentColorCell (this);
    m_transparentColorCell->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect (m_transparentColorCell, SIGNAL (foregroundColorChanged (const kpColor &)),
             this, SIGNAL (foregroundColorChanged (const kpColor &)));
    connect (m_transparentColorCell, SIGNAL (backgroundColorChanged (const kpColor &)),
             this, SIGNAL (backgroundColorChanged (const kpColor &)));

    m_colorCells = new kpColorCells (this);
    connect (m_colorCells, SIGNAL (foregroundColorChanged (const kpColor &)),
             this, SIGNAL (foregroundColorChanged (const kpColor &)));
    connect (m_colorCells, SIGNAL (backgroundColorChanged (const kpColor &)),
             this, SIGNAL (backgroundColorChanged (const kpColor &)));

    setOrientation (o);
}

kpColorPalette::~kpColorPalette ()
{
}

// public
Qt::Orientation kpColorPalette::orientation () const
{
    return m_orientation;
}

void kpColorPalette::setOrientation (Qt::Orientation o)
{
    m_colorCells->setOrientation (o);

    delete m_boxLayout;

    if (o == Qt::Horizontal)
    {
        m_boxLayout = new QBoxLayout (QBoxLayout::LeftToRight, this );
        m_boxLayout->addWidget (m_transparentColorCell, 0/*stretch*/, Qt::AlignVCenter);
        m_boxLayout->addWidget (m_colorCells);
    }
    else
    {
        m_boxLayout = new QBoxLayout (QBoxLayout::TopToBottom, this);
        m_boxLayout->addWidget (m_transparentColorCell, 0/*stretch*/, Qt::AlignHCenter);
        m_boxLayout->addWidget (m_colorCells);
    }
    m_boxLayout->setSpacing( 5 );

    m_orientation = o;
}


#include <kpColorPalette.moc>
