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


#define DEBUG_KP_EFFECT_BALANCE 0


#include "kpEffectBalanceWidget.h"

#include "imagelib/effects/kpEffectBalance.h"
#include "commands/imagelib/effects/kpEffectBalanceCommand.h"
#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>
#include "kpNumInput.h"

#include <cmath>

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#if DEBUG_KP_EFFECT_BALANCE
    #include <qdatetime.h>
#endif


kpEffectBalanceWidget::kpEffectBalanceWidget (bool actOnSelection,
                                              QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    auto *lay = new QGridLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *brightnessLabel = new QLabel (i18n ("&Brightness:"), this);
    m_brightnessInput = new kpIntNumInput (0/*value*/, this);
    m_brightnessInput->setRange (-50, 50);
    auto *brightnessResetPushButton = new QPushButton (i18n ("Re&set"), this);

    auto *contrastLabel = new QLabel (i18n ("Co&ntrast:"), this);
    m_contrastInput = new kpIntNumInput (0/*value*/, this);
    m_contrastInput->setRange (-50, 50);
     auto *contrastResetPushButton = new QPushButton (i18n ("&Reset"), this);

    auto *gammaLabel = new QLabel (i18n ("&Gamma:"), this);
    m_gammaInput = new kpIntNumInput (0/*value*/, this);
    m_gammaInput->setRange (-50, 50);
    // TODO: This is what should be shown in the m_gammaInput spinbox
    m_gammaLabel = new QLabel (this);
    // TODO: This doesn't seem to be wide enough with some fonts so the
    //       whole layout moves when we drag the gamma slider.
    m_gammaLabel->setMinimumWidth(m_gammaLabel->fontMetrics().horizontalAdvance(QLatin1String(" 10.00 ")));
    m_gammaLabel->setAlignment (m_gammaLabel->alignment () | Qt::AlignRight);
    auto *gammaResetPushButton = new QPushButton (i18n ("Rese&t"), this);


    auto *spaceWidget = new QLabel (this);
    spaceWidget->setFixedSize (1, fontMetrics ().height () / 4);


    auto *channelLabel = new QLabel (i18n ("C&hannels:"), this);
    m_channelsComboBox = new QComboBox (this);
    m_channelsComboBox->addItem (i18n ("All"));
    m_channelsComboBox->addItem (i18n ("Red"));
    m_channelsComboBox->addItem (i18n ("Green"));
    m_channelsComboBox->addItem (i18n ("Blue"));


    auto *resetPushButton = new QPushButton (i18n ("Reset &All Values"), this);


    brightnessLabel->setBuddy (m_brightnessInput);
    contrastLabel->setBuddy (m_contrastInput);
    gammaLabel->setBuddy (m_gammaInput);

    channelLabel->setBuddy (m_channelsComboBox);


    lay->addWidget (brightnessLabel, 0, 0);
    lay->addWidget (m_brightnessInput, 0, 1, 1, 2);
    lay->addWidget (brightnessResetPushButton, 0, 4);

    lay->addWidget (contrastLabel, 1, 0);
    lay->addWidget (m_contrastInput, 1, 1, 1, 2);
    lay->addWidget (contrastResetPushButton, 1, 4);

    lay->addWidget (gammaLabel, 2, 0);
    lay->addWidget (m_gammaInput, 2, 1, 1, 2);
    lay->addWidget (m_gammaLabel, 2, 3);
    lay->addWidget (gammaResetPushButton, 2, 4);

    lay->addWidget (spaceWidget, 3, 0, 1, 5);
    lay->addWidget (resetPushButton, 4, 2, 1, 3, Qt::AlignRight);

    lay->addWidget (channelLabel, 4, 0);
    lay->addWidget (m_channelsComboBox, 4, 1, Qt::AlignLeft);
    //lay->addWidget (resetPushButton, 4, 2, Qt::AlignRight);

    lay->setColumnStretch (1, 1);


    // (no need for settingsChangedDelayed() since BCG effect is so fast :))
    connect (m_brightnessInput, &kpIntNumInput::valueChanged,
             this, &kpEffectBalanceWidget::settingsChangedNoWaitCursor);
    connect (m_contrastInput, &kpIntNumInput::valueChanged,
             this, &kpEffectBalanceWidget::settingsChangedNoWaitCursor);

    connect (m_gammaInput, &kpIntNumInput::valueChanged,
             this, &kpEffectBalanceWidget::recalculateGammaLabel);
    connect (m_gammaInput, &kpIntNumInput::valueChanged,
             this, &kpEffectBalanceWidget::settingsChangedNoWaitCursor);

    connect (m_channelsComboBox,
             static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
             this, &kpEffectBalanceWidget::settingsChanged);

    connect (brightnessResetPushButton, &QPushButton::clicked,
             this, &kpEffectBalanceWidget::resetBrightness);
    connect (contrastResetPushButton, &QPushButton::clicked,
             this, &kpEffectBalanceWidget::resetContrast);
    connect (gammaResetPushButton, &QPushButton::clicked,
             this, &kpEffectBalanceWidget::resetGamma);

    connect (resetPushButton, &QPushButton::clicked,
             this, &kpEffectBalanceWidget::resetAll);

    recalculateGammaLabel ();
}

kpEffectBalanceWidget::~kpEffectBalanceWidget () = default;


// public virtual [base kpEffectWidgetBase]
QString kpEffectBalanceWidget::caption () const
{
    return i18n ("Settings");
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectBalanceWidget::isNoOp () const
{
    return (brightness () == 0 && contrast () == 0 && gamma () == 0);
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectBalanceWidget::applyEffect (const kpImage &image)
{
    return kpEffectBalance::applyEffect (image,
        channels (), brightness (), contrast (), gamma ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectBalanceWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectBalanceCommand (channels (),
                                       brightness (), contrast (), gamma (),
                                       m_actOnSelection,
                                       cmdEnviron);
}


// protected
int kpEffectBalanceWidget::channels () const
{
    switch (m_channelsComboBox->currentIndex ())
    {
    default:
    case 0:
        return kpEffectBalance::RGB;

    case 1:
        return kpEffectBalance::Red;

    case 2:
        return kpEffectBalance::Green;

    case 3:
        return kpEffectBalance::Blue;
    }
}


// protected
int kpEffectBalanceWidget::brightness () const
{
    return m_brightnessInput->value ();
}

// protected
int kpEffectBalanceWidget::contrast () const
{
    return m_contrastInput->value ();
}

// protected
int kpEffectBalanceWidget::gamma () const
{
    return m_gammaInput->value ();
}


// protected slot
void kpEffectBalanceWidget::recalculateGammaLabel ()
{
    m_gammaLabel->setText (
        QLatin1String (" ") +
        QString::number (std::pow (10, gamma () / 50.0),
                         'f'/*[-]9.9*/,
                         2/*precision*/) +
        QLatin1String (" "));
    m_gammaLabel->repaint ();
}


// protected slot
void kpEffectBalanceWidget::resetBrightness ()
{
    if (brightness () == 0) {
        return;
    }

    bool sb = signalsBlocked ();

    if (!sb) {
        blockSignals (true);
    }
    m_brightnessInput->setValue (0);
    if (!sb) {
        blockSignals (false);
    }

    // Immediate update (if signals aren't blocked)
    emit settingsChanged ();
}

// protected slot
void kpEffectBalanceWidget::resetContrast ()
{
    if (contrast () == 0) {
        return;
    }

    bool sb = signalsBlocked ();

    if (!sb) {
        blockSignals (true);
    }
    m_contrastInput->setValue (0);
    if (!sb) {
        blockSignals (false);
    }

    // Immediate update (if signals aren't blocked)
    emit settingsChanged ();
}

// protected slot
void kpEffectBalanceWidget::resetGamma ()
{
    if (gamma () == 0) {
        return;
    }

    bool sb = signalsBlocked ();

    if (!sb) {
        blockSignals (true);
    }
    m_gammaInput->setValue (0);
    recalculateGammaLabel ();
    if (!sb) {
        blockSignals (false);
    }

    // Immediate update (if signals aren't blocked)
    emit settingsChanged ();
}


// protected slot
void kpEffectBalanceWidget::resetAll ()
{
    if (isNoOp ()) {
        return;
    }

    // Prevent multiple settingsChanged() which would normally result in
    // redundant, expensive preview repaints
    blockSignals (true);

    resetBrightness ();
    resetContrast ();
    resetGamma ();

    recalculateGammaLabel ();

    blockSignals (false);

    emit settingsChanged ();
}


