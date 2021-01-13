
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


#include "widgets/toolbars/kpColorToolBar.h"

#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QDragMoveEvent>

#include <KColorMimeData>
#include <KLocalizedString>
#include "kpLogCategories.h"

#include "widgets/kpColorCells.h"
#include "widgets/kpColorPalette.h"
#include "widgets/colorSimilarity/kpColorSimilarityToolBarItem.h"
#include "kpDefs.h"
#include "widgets/kpDualColorButton.h"
#include "lgpl/generic/kpUrlFormatter.h"

//---------------------------------------------------------------------

kpColorToolBar::kpColorToolBar (const QString &label, QWidget *parent)
    : QDockWidget (parent)
{
    setWindowTitle (label);

    // not closable, as it's not a KDE toolbar yet and can not be made shown easily again
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    setAcceptDrops (true);

    QWidget *base = new QWidget (this);
    m_boxLayout = new QBoxLayout (QBoxLayout::LeftToRight, base);
    m_boxLayout->setContentsMargins(5, 5, 5, 5);
    m_boxLayout->setSpacing (10 * 3);

    // This holds the current global foreground and background colors, for
    // tools.
    m_dualColorButton = new kpDualColorButton (base);
    connect (m_dualColorButton, &kpDualColorButton::colorsSwapped,
             this, &kpColorToolBar::colorsSwapped);

     connect (m_dualColorButton, &kpDualColorButton::foregroundColorChanged,
              this, &kpColorToolBar::foregroundColorChanged);

     connect (m_dualColorButton, &kpDualColorButton::backgroundColorChanged,
              this, &kpColorToolBar::backgroundColorChanged);

    m_boxLayout->addWidget (m_dualColorButton, 0/*stretch*/, Qt::AlignVCenter);

    m_colorPalette = new kpColorPalette (base);
    connect (m_colorPalette, &kpColorPalette::foregroundColorChanged,
             m_dualColorButton, &kpDualColorButton::setForegroundColor);

    connect (m_colorPalette, &kpColorPalette::backgroundColorChanged,
             m_dualColorButton, &kpDualColorButton::setBackgroundColor);

    connect (m_colorPalette->colorCells (), &kpColorCells::isModifiedChanged,
             this, &kpColorToolBar::updateNameOrUrlLabel);

    connect (m_colorPalette->colorCells (), &kpColorCells::urlChanged,
             this, &kpColorToolBar::updateNameOrUrlLabel);

    connect (m_colorPalette->colorCells (), &kpColorCells::nameChanged,
             this, &kpColorToolBar::updateNameOrUrlLabel);

    updateNameOrUrlLabel ();

    m_boxLayout->addWidget (m_colorPalette, 0/*stretch*/);

    m_colorSimilarityToolBarItem = new kpColorSimilarityToolBarItem (base);
    connect (m_colorSimilarityToolBarItem,
             &kpColorSimilarityToolBarItem::colorSimilarityChanged,
             this, &kpColorToolBar::colorSimilarityChanged);

    m_boxLayout->addWidget (m_colorSimilarityToolBarItem, 0/*stretch*/);

    // Pad out all the horizontal space on the right of the Color Tool Bar so that
    // that the real Color Tool Bar widgets aren't placed in the center of the
    // Color Tool Bar.
    m_boxLayout->addItem (
        new QSpacerItem (1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

    adjustToOrientation (Qt::Horizontal);

    setWidget (base);
}

//---------------------------------------------------------------------

void kpColorToolBar::adjustToOrientation (Qt::Orientation o)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpColorToolBar::adjustToOrientation("
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

//---------------------------------------------------------------------

// public
kpColorCells *kpColorToolBar::colorCells () const
{
    return m_colorPalette->colorCells ();
}

//---------------------------------------------------------------------

kpColor kpColorToolBar::color (int which) const
{
    Q_ASSERT (which == 0 || which == 1);

    return m_dualColorButton->color (which);
}

//---------------------------------------------------------------------

void kpColorToolBar::setColor (int which, const kpColor &color)
{
    Q_ASSERT (which == 0 || which == 1);

    m_dualColorButton->setColor (which, color);
}

//---------------------------------------------------------------------

kpColor kpColorToolBar::foregroundColor () const
{
    return m_dualColorButton->foregroundColor ();
}

//---------------------------------------------------------------------

void kpColorToolBar::setForegroundColor (const kpColor &color)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpColorToolBar::setForegroundColor("
              << (int *) color.toQRgb () << ")";
#endif
    m_dualColorButton->setForegroundColor (color);
}

//---------------------------------------------------------------------

kpColor kpColorToolBar::backgroundColor () const
{
    return m_dualColorButton->backgroundColor ();
}

//---------------------------------------------------------------------

void kpColorToolBar::setBackgroundColor (const kpColor &color)
{
#if DEBUG_KP_COLOR_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpColorToolBar::setBackgroundColor("
              << (int *) color.toQRgb () << ")";
#endif
    m_dualColorButton->setBackgroundColor (color);
}

//---------------------------------------------------------------------


kpColor kpColorToolBar::oldForegroundColor () const
{
    return m_dualColorButton->oldForegroundColor ();
}

//---------------------------------------------------------------------

kpColor kpColorToolBar::oldBackgroundColor () const
{
    return m_dualColorButton->oldBackgroundColor ();
}

//---------------------------------------------------------------------

double kpColorToolBar::oldColorSimilarity () const
{
    return m_colorSimilarityToolBarItem->oldColorSimilarity ();
}

//---------------------------------------------------------------------

double kpColorToolBar::colorSimilarity () const
{
    return m_colorSimilarityToolBarItem->colorSimilarity ();
}

//---------------------------------------------------------------------

void kpColorToolBar::setColorSimilarity (double similarity)
{
    m_colorSimilarityToolBarItem->setColorSimilarity (similarity);
}

//---------------------------------------------------------------------

int kpColorToolBar::processedColorSimilarity () const
{
    return m_colorSimilarityToolBarItem->processedColorSimilarity ();
}

//---------------------------------------------------------------------

void kpColorToolBar::openColorSimilarityDialog ()
{
    m_colorSimilarityToolBarItem->openDialog ();
}

//---------------------------------------------------------------------

void kpColorToolBar::flashColorSimilarityToolBarItem ()
{
    m_colorSimilarityToolBarItem->flash ();
}

//---------------------------------------------------------------------

// private slot
void kpColorToolBar::updateNameOrUrlLabel ()
{
    QString name;

    kpColorCells *colorCells = m_colorPalette->colorCells ();
    if (!colorCells->url ().isEmpty ()) {
        name = kpUrlFormatter::PrettyFilename (colorCells->url ());
    }
    else
    {
        if (!colorCells->name ().isEmpty ()) {
            name = colorCells->name ();
        }
        else {
            name = i18n ("KolourPaint Defaults");
        }
    }

    if (name.isEmpty ()) {
        name = i18n ("Untitled");
    }


    KLocalizedString labelStr;

    if (!m_colorPalette->colorCells ()->isModified ())
    {
        labelStr =
            ki18nc ("Colors: name_or_url_of_color_palette",
                    "Colors: %1")
                .subs (name);
    }
    else
    {
        labelStr =
            ki18nc ("Colors: name_or_url_of_color_palette [modified]",
                    "Colors: %1 [modified]")
                .subs (name);
    }

    // Kill 2 birds with 1 stone:
    //
    // 1. Hide the windowTitle() when it's docked.
    // 2. Add a label containing the name of the open color palette.
    //
    // TODO: This currently hides the windowTitle() even when it's not docked,
    //       because we've abused it to show the name of open color palette
    //       instead.
    setWindowTitle (labelStr.toString ());
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpColorToolBar::dragEnterEvent (QDragEnterEvent *e)
{
    // Grab the color drag for this widget, preventing it from being
    // handled by our parent, the main window.
    e->setAccepted (KColorMimeData::canDecode (e->mimeData ()));
#if DEBUG_KP_COLOR_TOOL_BAR
    qCDebug(kpLogWidgets) << "isAccepted=" << e->isAccepted ();
#endif
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpColorToolBar::dragMoveEvent (QDragMoveEvent *e)
{
    // Stop the grabbed drag from being dropped.
    e->setAccepted (!KColorMimeData::canDecode (e->mimeData ()));
#if DEBUG_KP_COLOR_TOOL_BAR
    qCDebug(kpLogWidgets) << "isAccepted=" << e->isAccepted ();
#endif
}

//---------------------------------------------------------------------

