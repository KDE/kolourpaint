
/*
   Copyright (c) 2007 Mike Gashler <gashlerm@yahoo.com>
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


#include "kpEffectHSVWidget.h"

#include <QGridLayout>
#include <QLabel>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "imagelib/effects/kpEffectHSV.h"
#include "commands/imagelib/effects/kpEffectHSVCommand.h"


kpEffectHSVWidget::kpEffectHSVWidget (bool actOnSelection, QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    auto *lay = new QGridLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *hueLabel = new QLabel (i18n ("&Hue:"), this);
    auto *saturationLabel = new QLabel (i18n ("&Saturation:"), this);
    auto *valueLabel = new QLabel (i18nc ("The V of HSV", "&Value:"), this);

    m_hueInput = new kpDoubleNumInput (this);
    m_hueInput->setRange (-180, 180, 15/*step*/);

    m_saturationInput = new kpDoubleNumInput (this);
    m_saturationInput->setRange (-1, 1, 0.1/*step*/);

    m_valueInput = new kpDoubleNumInput (this);
    m_valueInput->setRange (-1, 1, 0.1/*step*/);

    hueLabel->setBuddy (m_hueInput);
    saturationLabel->setBuddy (m_saturationInput);
    valueLabel->setBuddy (m_valueInput);

    lay->addWidget (hueLabel, 0, 0);
    lay->addWidget (m_hueInput, 0, 1);

    lay->addWidget (saturationLabel, 1, 0);
    lay->addWidget (m_saturationInput, 1, 1);

    lay->addWidget (valueLabel, 2, 0);
    lay->addWidget (m_valueInput, 2, 1);

    lay->setColumnStretch (1, 1);


    connect (m_hueInput, &kpDoubleNumInput::valueChanged,
             this, &kpEffectHSVWidget::settingsChangedDelayed);

    connect (m_saturationInput, &kpDoubleNumInput::valueChanged,
             this, &kpEffectHSVWidget::settingsChangedDelayed);

    connect (m_valueInput, &kpDoubleNumInput::valueChanged,
             this, &kpEffectHSVWidget::settingsChangedDelayed);
}

kpEffectHSVWidget::~kpEffectHSVWidget () = default;


// public virtual [base kpEffectWidgetBase]
QString kpEffectHSVWidget::caption () const
{
    // TODO: Why doesn't this have a caption?  Ditto for the other effects.
    return {};
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectHSVWidget::isNoOp () const
{
    return m_hueInput->value () == 0 && m_saturationInput->value () == 0 && m_valueInput->value () == 0;
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectHSVWidget::applyEffect (const kpImage &image)
{
    return kpEffectHSV::applyEffect (image,
        m_hueInput->value (), m_saturationInput->value (), m_valueInput->value ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectHSVWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectHSVCommand (
        m_hueInput->value (), m_saturationInput->value (), m_valueInput->value (),
        m_actOnSelection,
        cmdEnviron);
}


