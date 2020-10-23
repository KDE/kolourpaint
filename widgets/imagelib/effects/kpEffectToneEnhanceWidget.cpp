
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2006 Mike Gashler <gashlerm@yahoo.com>
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


#include "kpEffectToneEnhanceWidget.h"

#include "imagelib/effects/kpEffectToneEnhance.h"
#include "commands/imagelib/effects/kpEffectToneEnhanceCommand.h"
#include "pixmapfx/kpPixmapFX.h"

#include <QGridLayout>
#include <QLabel>

#include "kpLogCategories.h"
#include <KLocalizedString>

kpEffectToneEnhanceWidget::kpEffectToneEnhanceWidget (bool actOnSelection,
                                                      QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent),
      m_granularityInput (nullptr),
      m_amountInput (nullptr)

{
    auto *lay = new QGridLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);

    // See kpEffectToneEnhance::applyEffect().

    auto *granularityLabel = new QLabel (i18n ("&Granularity:"), this);

    auto *amountLabel = new QLabel (i18n ("&Amount:"), this);

    m_granularityInput = new kpDoubleNumInput (this);
    m_granularityInput->setRange (0, 1, 0.1/*step*/);

    m_amountInput = new kpDoubleNumInput (this);
    m_amountInput->setRange (0, 1, 0.1/*step*/);

    granularityLabel->setBuddy (m_granularityInput);
    amountLabel->setBuddy (m_amountInput);


    lay->addWidget (granularityLabel, 0, 0);
    lay->addWidget (m_granularityInput, 0, 1);

    lay->addWidget (amountLabel, 1, 0);
    lay->addWidget (m_amountInput, 1, 1);

    lay->setColumnStretch (1, 1);


    connect (m_granularityInput, &kpDoubleNumInput::valueChanged,
             this, &kpEffectToneEnhanceWidget::settingsChangedDelayed);

    connect (m_amountInput, &kpDoubleNumInput::valueChanged,
             this, &kpEffectToneEnhanceWidget::settingsChangedDelayed);

}

kpEffectToneEnhanceWidget::~kpEffectToneEnhanceWidget () = default;


// public virtual [base kpEffectWidgetBase]
QString kpEffectToneEnhanceWidget::caption () const
{
    // TODO: Why doesn't this have a caption?  Ditto for the other effects.
    return {};
}


// private
double kpEffectToneEnhanceWidget::amount () const
{
    return m_amountInput ? m_amountInput->value () : 0;
}

// private
double kpEffectToneEnhanceWidget::granularity () const
{
    return m_granularityInput ? m_granularityInput->value () : 0;
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectToneEnhanceWidget::isNoOp () const
{
    // If the "amount" is 0, nothing happens regardless of the granularity.
    // Note that if "granularity" is 0 but "amount" > 0, the effect _is_ active.
    // Therefore, "granularity" should have no involvement in this check.
    if (amount () == 0) {
        return true;
    }

    return false;
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectToneEnhanceWidget::applyEffect (const kpImage &image)
{
    return kpEffectToneEnhance::applyEffect (image,
        granularity (), amount ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectToneEnhanceWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectToneEnhanceCommand (granularity (), amount (),
                                           m_actOnSelection,
                                           cmdEnviron);
}


