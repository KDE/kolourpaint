
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


#define DEBUG_KP_COLOR_TOOL_BAR 0


#include <kpColorToolBar.h>

#include <qbitmap.h>
#include <qboxlayout.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>

#include <qwidget.h>

#include <kapplication.h>
#include <kcolordialog.h>
#include <k3colordrag.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kpColorPalette.h>
#include <kpColorSimilarityToolBarItem.h>
#include <kpDefs.h>
#include <kpDualColorButton.h>
#include <kpPixmapFX.h>
#include <kpTool.h>
#include <kpView.h>


kpColorToolBar::kpColorToolBar (const QString &label, QWidget *parent)
    : KToolBar (parent)
{
    setWindowTitle (label);


    QWidget *base = new QWidget (this);
    m_boxLayout = new QBoxLayout (QBoxLayout::LeftToRight, base);
    m_boxLayout->setMargin (5);
    m_boxLayout->setSpacing (10 * 4);

    m_dualColorButton = new kpDualColorButton (base);
    m_dualColorButton->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect (m_dualColorButton, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)),
             this, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)));
    connect (m_dualColorButton, SIGNAL (foregroundColorChanged (const kpColor &)),
             this, SIGNAL (foregroundColorChanged (const kpColor &)));
    connect (m_dualColorButton, SIGNAL (backgroundColorChanged (const kpColor &)),
             this, SIGNAL (backgroundColorChanged (const kpColor &)));
    m_boxLayout->addWidget (m_dualColorButton, 0/*stretch*/);

    m_colorPalette = new kpColorPalette (base);
    connect (m_colorPalette, SIGNAL (foregroundColorChanged (const kpColor &)),
             m_dualColorButton, SLOT (setForegroundColor (const kpColor &)));
    connect (m_colorPalette, SIGNAL (backgroundColorChanged (const kpColor &)),
             m_dualColorButton, SLOT (setBackgroundColor (const kpColor &)));
    m_boxLayout->addWidget (m_colorPalette, 0/*stretch*/);

    m_colorSimilarityToolBarItem = new kpColorSimilarityToolBarItem (base);
    m_colorSimilarityToolBarItem->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect (m_colorSimilarityToolBarItem, SIGNAL (colorSimilarityChanged (double, int)),
             this, SIGNAL (colorSimilarityChanged (double, int)));
    m_boxLayout->addWidget (m_colorSimilarityToolBarItem, 0/*stretch*/);

    // HACK: couldn't get QSpacerItem to work
    QWidget *fakeSpacer = new QWidget (base);
    m_boxLayout->addWidget (fakeSpacer, 1/*stretch*/);

    m_lastDockedOrientationSet = false;
    setOrientation (orientation ());

    addWidget (base);
}

// TODO: setOrientation() is not virtual in Qt4.
//       So don't we need the kptooltoolbar.cpp hacks too?  Maybe not
//       since the default orientation is horizontal.
//
//       In any case, we need to rewrite kpColorToolBar to be based on
//       QDockWidget.

// virtual
void kpColorToolBar::setOrientation (Qt::Orientation o)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "kpColorToolBar::setOrientation("
               << (o == Qt::Vertical ? "vertical" : "horizontal")
               << ") called!" << endl;
#endif

    // (QDockWindow::undock() calls us)
    bool isOutsideDock = false; //(place () == Q3DockWindow::OutsideDock);

    if (!m_lastDockedOrientationSet || !isOutsideDock)
    {
        m_lastDockedOrientation = o;
        m_lastDockedOrientationSet = true;
    }

    if (isOutsideDock)
    {
        //kDebug () << "\toutside dock, forcing orientation to last";
        o = m_lastDockedOrientation;
    }

    if (o == Qt::Horizontal)
    {
        m_boxLayout->setDirection (QBoxLayout::LeftToRight);
    }
    else
    {
        m_boxLayout->setDirection (QBoxLayout::TopToBottom);
    }

    m_colorPalette->setOrientation (o);

    KToolBar::setOrientation (o);
}

kpColorToolBar::~kpColorToolBar ()
{
}


// public
kpColorCells *kpColorToolBar::colorCells () const
{
    return m_colorPalette->colorCells ();
}


kpColor kpColorToolBar::color (int which) const
{
    Q_ASSERT (which == 0 || which == 1);

    return m_dualColorButton->color (which);
}

void kpColorToolBar::setColor (int which, const kpColor &color)
{
    Q_ASSERT (which == 0 || which == 1);

    m_dualColorButton->setColor (which, color);
}

kpColor kpColorToolBar::foregroundColor () const
{
    return m_dualColorButton->foregroundColor ();
}

void kpColorToolBar::setForegroundColor (const kpColor &color)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "kpColorToolBar::setForegroundColor("
              << (int *) color.toQRgb () << ")" << endl;
#endif
    m_dualColorButton->setForegroundColor (color);
}

kpColor kpColorToolBar::backgroundColor () const
{
    return m_dualColorButton->backgroundColor ();
}

void kpColorToolBar::setBackgroundColor (const kpColor &color)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "kpColorToolBar::setBackgroundColor("
              << (int *) color.toQRgb () << ")" << endl;
#endif
    m_dualColorButton->setBackgroundColor (color);
}


kpColor kpColorToolBar::oldForegroundColor () const
{
    return m_dualColorButton->oldForegroundColor ();
}

kpColor kpColorToolBar::oldBackgroundColor () const
{
    return m_dualColorButton->oldBackgroundColor ();
}

double kpColorToolBar::oldColorSimilarity () const
{
    return m_colorSimilarityToolBarItem->oldColorSimilarity ();
}


double kpColorToolBar::colorSimilarity () const
{
    return m_colorSimilarityToolBarItem->colorSimilarity ();
}

void kpColorToolBar::setColorSimilarity (double similarity)
{
    m_colorSimilarityToolBarItem->setColorSimilarity (similarity);
}

int kpColorToolBar::processedColorSimilarity () const
{
    return m_colorSimilarityToolBarItem->processedColorSimilarity ();
}


void kpColorToolBar::openColorSimilarityDialog ()
{
    m_colorSimilarityToolBarItem->openDialog ();
}

void kpColorToolBar::flashColorSimilarityToolBarItem ()
{
    m_colorSimilarityToolBarItem->flash ();
}


#include <kpColorToolBar.moc>
