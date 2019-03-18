
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


#define DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM 0


#include "kpColorSimilarityToolBarItem.h"

#include <QTimer>
#include <QPixmap>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "imagelib/kpColor.h"
#include "dialogs/kpColorSimilarityDialog.h"
#include "kpColorSimilarityCubeRenderer.h"
#include "kpDefs.h"

//---------------------------------------------------------------------

kpColorSimilarityToolBarItem::kpColorSimilarityToolBarItem (QWidget *parent)
    : QToolButton (parent),
      kpColorSimilarityHolder (),

      m_oldColorSimilarity (0),
      m_processedColorSimilarity (kpColor::Exact),
      m_flashTimer (new QTimer (this)),
      m_flashHighlight (0),
      m_suppressingFlashCounter (0)
{
    setAutoRaise (true);
    setFixedSize (52, 52);

    setWhatsThis (WhatsThisWithClickInstructions ());

    connect (this, &kpColorSimilarityToolBarItem::clicked,
             this, &kpColorSimilarityToolBarItem::openDialog);

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);
    setColorSimilarityInternal (cfg.readEntry (kpSettingColorSimilarity, 0.0),
        false/*don't write config*/);

    m_flashTimer->setInterval (100/*ms*/);
    connect (m_flashTimer, &QTimer::timeout,
             this, &kpColorSimilarityToolBarItem::slotFlashTimerTimeout);
}

//---------------------------------------------------------------------

// public
int kpColorSimilarityToolBarItem::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}

//---------------------------------------------------------------------

// private
void kpColorSimilarityToolBarItem::setColorSimilarityInternal (double similarity,
        bool writeConfig)
{
#if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
    qCDebug(kpLogWidgets) << "kpColorSimilarityToolBarItem::setColorSimilarityInternal("
              << "similarity=" << similarity << ",writeConfig=" << writeConfig
              << ")";
#endif

    m_oldColorSimilarity = colorSimilarity ();

    kpColorSimilarityHolder::setColorSimilarity (similarity);
    m_processedColorSimilarity = kpColor::processSimilarity (colorSimilarity ());

    updateIcon ();
    updateToolTip ();

    if (writeConfig)
    {
        KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);
        cfg.writeEntry (kpSettingColorSimilarity, colorSimilarity ());
        cfg.sync ();
    }

    emit colorSimilarityChanged (colorSimilarity (), m_processedColorSimilarity);
}

//---------------------------------------------------------------------

// public virtual [base kopColorSimilarityHolder]
void kpColorSimilarityToolBarItem::setColorSimilarity (double similarity)
{
    // (this calls the base setColorSimilarity() as required by base)
    setColorSimilarityInternal (similarity, true/*write config*/);
}

//---------------------------------------------------------------------

// public
double kpColorSimilarityToolBarItem::oldColorSimilarity () const
{
    return m_oldColorSimilarity;
}

//---------------------------------------------------------------------

// public
void kpColorSimilarityToolBarItem::openDialog ()
{
    kpColorSimilarityDialog dialog (this);
    dialog.setColorSimilarity (colorSimilarity ());
    if (dialog.exec ())
    {
        setColorSimilarity (dialog.colorSimilarity ());
    }
}

//---------------------------------------------------------------------

// private slot:
void kpColorSimilarityToolBarItem::slotFlashTimerTimeout ()
{
#if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
    qCDebug(kpLogWidgets) << "kpColorSimilarityToolBarItem::slotFlashTimerTimeout()"
              << " highlight=" << m_flashHighlight << endl;
#endif
    int newHigh = m_flashHighlight - 20;
    if (newHigh < 0) {
        newHigh = 0;
    }

    m_flashHighlight = newHigh;

    updateIcon ();

    if (newHigh == 0) {
        m_flashTimer->stop ();
    }
}

//---------------------------------------------------------------------

// public
void kpColorSimilarityToolBarItem::flash ()
{
#if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
    qCDebug(kpLogWidgets) << "kpColorSimilarityToolBarItem::flash()";
#endif
    if (isSuppressingFlash ()) {
        return;
    }

    if (m_flashHighlight == 255)
    {
    #if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
        qCDebug(kpLogWidgets) << "\tNOP";
    #endif
    }
    else
    {
        m_flashHighlight = 255;

        updateIcon ();
    }

    m_flashTimer->start ();
}

//---------------------------------------------------------------------

// public
bool kpColorSimilarityToolBarItem::isSuppressingFlash () const
{
    return (m_suppressingFlashCounter > 0);
}

//---------------------------------------------------------------------

// public
void kpColorSimilarityToolBarItem::suppressFlash ()
{
    m_suppressingFlashCounter++;
}

//---------------------------------------------------------------------

// public
void kpColorSimilarityToolBarItem::unsupressFlash ()
{
    m_suppressingFlashCounter--;
    Q_ASSERT (m_suppressingFlashCounter >= 0);
}

//---------------------------------------------------------------------

// private
void kpColorSimilarityToolBarItem::updateToolTip ()
{
#if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
    qCDebug(kpLogWidgets) << "kpColorSimilarityToolBarItem::updateToolTip()";
#endif

    if (colorSimilarity () > 0)
    {
        setToolTip (
            i18n ("<p>Color Similarity: %1%</p>"
                  "<p align=\"center\">Click to configure.</p>",
                qRound (colorSimilarity () * 100)));
    }
    else
    {
        setToolTip (
            i18n ("<p>Color Similarity: Exact Match</p>"
                  "<p align=\"center\">Click to configure.</p>"));
    }
}

//---------------------------------------------------------------------

// private
// LOOPT: This gets called twice on KolourPaint startup by:
//
//            1. setColorSimilarityInternal() called by the ctor
//            2. resizeEvent() when it's first shown()
//
//        We could get rid of the first and save a few milliseconds.
void kpColorSimilarityToolBarItem::updateIcon ()
{
    const int side = width () * 6 / 8;
#if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
    qCDebug(kpLogWidgets) << "kpColorSimilarityToolBarItem::updateIcon() width=" << width ()
              << " side=" << side;
#endif

    QPixmap icon(side, side);
    icon.fill(Qt::transparent);

    kpColorSimilarityCubeRenderer::Paint (&icon,
        0/*x*/, 0/*y*/, side,
        colorSimilarity (), m_flashHighlight);

    setIconSize(QSize(side, side));
    setIcon(icon);
}

//---------------------------------------------------------------------

// private virtual [base QWidget]
void kpColorSimilarityToolBarItem::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM
    qCDebug(kpLogWidgets) << "kpColorSimilarityToolBarItem::resizeEvent() size=" << size ()
              << " oldSize=" << e->oldSize ();
#endif
    QToolButton::resizeEvent (e);

    updateIcon ();
}

//---------------------------------------------------------------------

