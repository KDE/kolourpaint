
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


#define DEBUG_KP_COLOR_TOOL_BAR 1


#include <kpColorToolBar.h>

#include <qbitmap.h>
#include <qboxlayout.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <QLabel>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <QPushButton>
#include <qsize.h>

#include <qwidget.h>

#include <kapplication.h>
#include <kcolordialog.h>
#include <k3colordrag.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kpColorCells.h>
#include <kpColorPalette.h>
#include <kpColorSimilarityToolBarItem.h>
#include <kpDefs.h>
#include <kpDualColorButton.h>
#include <kpPixmapFX.h>
#include <kpTool.h>
#include <kpView.h>


struct kpColorToolBarPrivate
{
    QPushButton *titlePushButton;
    QWidget *titleBarWidget;
};

kpColorToolBar::kpColorToolBar (const QString &label, QWidget *parent)
    : QDockWidget (parent),
      d (new kpColorToolBarPrivate ())
{
    setWindowTitle (label);

    d->titleBarWidget = new QWidget (this);
    d->titlePushButton = new QPushButton (i18n ("Reload Colors"), d->titleBarWidget);
    connect (d->titlePushButton, SIGNAL (clicked ()),
             SIGNAL (reloadColorsButtonClicked ()));

    const int h = d->titlePushButton->sizeHint ().height ();
#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "titlePushButton sizeHint=" << d->titlePushButton->sizeHint ();
#endif

    QHBoxLayout *titleBarLay = new QHBoxLayout (d->titleBarWidget);
    titleBarLay->addItem (
        new QSpacerItem (1, h, QSizePolicy::Expanding, QSizePolicy::Minimum));
    titleBarLay->addWidget (d->titlePushButton);
    titleBarLay->addItem (
        new QSpacerItem (1, h, QSizePolicy::Expanding, QSizePolicy::Minimum));

    d->titlePushButton->hide ();

    // Disable title when it's docked.
    // sync: updateTitleLabel()
    // TODO: This currently disables the title even when it's not docked.
    setTitleBarWidget (d->titleBarWidget);


    QWidget *base = new QWidget (this);
    m_boxLayout = new QBoxLayout (QBoxLayout::LeftToRight, base);
    m_boxLayout->setMargin (5);
    m_boxLayout->setSpacing (10 * 3);

    m_dualColorButton = new kpDualColorButton (base);
    connect (m_dualColorButton, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)),
             this, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)));
    connect (m_dualColorButton, SIGNAL (foregroundColorChanged (const kpColor &)),
             this, SIGNAL (foregroundColorChanged (const kpColor &)));
    connect (m_dualColorButton, SIGNAL (backgroundColorChanged (const kpColor &)),
             this, SIGNAL (backgroundColorChanged (const kpColor &)));
    m_boxLayout->addWidget (m_dualColorButton, 0/*stretch*/, Qt::AlignVCenter);

    m_colorPalette = new kpColorPalette (base);
    connect (m_colorPalette, SIGNAL (foregroundColorChanged (const kpColor &)),
             m_dualColorButton, SLOT (setForegroundColor (const kpColor &)));
    connect (m_colorPalette, SIGNAL (backgroundColorChanged (const kpColor &)),
             m_dualColorButton, SLOT (setBackgroundColor (const kpColor &)));
    
    connect (m_colorPalette, SIGNAL (colorCellsIsModifiedChanged (bool)),
             SLOT (updateTitleLabel ()));
    updateTitleLabel ();

    m_boxLayout->addWidget (m_colorPalette, 0/*stretch*/);

    m_colorSimilarityToolBarItem = new kpColorSimilarityToolBarItem (base);
    connect (m_colorSimilarityToolBarItem, SIGNAL (colorSimilarityChanged (double, int)),
             this, SIGNAL (colorSimilarityChanged (double, int)));
    m_boxLayout->addWidget (m_colorSimilarityToolBarItem, 0/*stretch*/);

    // Pad out all the horizontal space on the right of the Color Tool Bar so that
    // that the real Color Tool Bar widgets aren't placed in the center of the
    // Color Tool Bar.
    m_boxLayout->addItem (
        new QSpacerItem (1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

    adjustToOrientation (Qt::Horizontal);

    setWidget (base);
}

void kpColorToolBar::adjustToOrientation (Qt::Orientation o)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "kpColorToolBar::adjustToOrientation("
               << (o == Qt::Vertical ? "vertical" : "horizontal")
               << ") called!";
#endif

    Q_ASSERT (o == Qt::Horizontal);

    if (o == Qt::Horizontal)
    {
        m_boxLayout->setDirection (QBoxLayout::LeftToRight);
    }
    else
    {
        m_boxLayout->setDirection (QBoxLayout::TopToBottom);
    }

    m_colorPalette->setOrientation (o);
}

kpColorToolBar::~kpColorToolBar ()
{
    delete d;
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


static QString RemoveAmpersands (const QString &textIn)
{
    QString text = textIn;
    text.remove (QRegExp ("&"));
    return text;
}

// private slot
void kpColorToolBar::updateTitleLabel ()
{
#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "titlePushButton sizeHint=" << d->titlePushButton->sizeHint ();
#endif

    if (!m_colorPalette->colorCells ()->isModified ())
        d->titlePushButton->hide ();
        //d->titleLabel->clear ();
    else
    {
        d->titlePushButton->show ();


#if 0
        d->titleLabel->setText (
            ki18nc ("\"color palette [modified]\","
                        " like \"file.txt [modified]\" in a window caption",
                    "%1 [modified]")
                .subs (::RemoveAmpersands (windowTitle ()))
                .toString ());
#endif
    }

#if DEBUG_KP_COLOR_TOOL_BAR
    kDebug () << "titlePushButton sizeHint=" << d->titlePushButton->sizeHint ();
#endif
}


#include <kpColorToolBar.moc>
