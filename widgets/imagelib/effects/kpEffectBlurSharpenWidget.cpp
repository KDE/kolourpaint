
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


#define DEBUG_KP_EFFECT_BLUR_SHARPEN 0


#include "kpEffectBlurSharpenWidget.h"

#include "commands/imagelib/effects/kpEffectBlurSharpenCommand.h"

#include "kpLogCategories.h"
#include <KLocalizedString>
#include "kpNumInput.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>


kpEffectBlurSharpenWidget::kpEffectBlurSharpenWidget (bool actOnSelection,
                                                      QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    auto *lay = new QGridLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *amountLabel = new QLabel (i18n ("&Amount:"), this);
    m_amountInput = new kpIntNumInput (this);
    m_amountInput->setRange (-kpEffectBlurSharpen::MaxStrength/*- for blur*/,
			     +kpEffectBlurSharpen::MaxStrength/*+ for sharpen*/);

    m_typeLabel = new QLabel (this);

    // Make sure <m_typeLabel> doesn't expand when the effect type changes,
    // as otherwise, that would cause the preview pixmap label in the
    // "More Effects" dialog (which our widget is inside) to contract,
    // which would look a bit weird.
    //
    // We do this by setting the label to every possible string it could
    // contain and fixing its height to the maximum seen size hint height.

    auto h = m_typeLabel->sizeHint ().height ();
#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    qCDebug(kpLogWidgets) << "initial size hint height=" << h;
#endif

    m_typeLabel->setText (
        kpEffectBlurSharpenCommand::nameForType (kpEffectBlurSharpen::Blur));
    h = qMax (h, m_typeLabel->sizeHint ().height ());

    m_typeLabel->setText (
        kpEffectBlurSharpenCommand::nameForType (kpEffectBlurSharpen::Sharpen));
    h = qMax (h, m_typeLabel->sizeHint ().height ());

    // Set this text last as the label's text needs to reflect the default
    // effect of "None".
    m_typeLabel->setText (
        kpEffectBlurSharpenCommand::nameForType (kpEffectBlurSharpen::None));
    h = qMax (h, m_typeLabel->sizeHint ().height ());

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    qCDebug(kpLogWidgets) << "maximum size hint height" << h;
#endif
    m_typeLabel->setFixedHeight (h);
    m_typeLabel->setAlignment (Qt::AlignCenter);


    amountLabel->setBuddy (m_amountInput);


    lay->addWidget (amountLabel, 0, 0);
    lay->addWidget (m_amountInput, 0, 1);

    lay->addWidget (m_typeLabel, 1, 0, 1, 2, Qt::AlignCenter);

    lay->setColumnStretch (1, 1);


    connect (m_amountInput, &kpIntNumInput::valueChanged,
             this, &kpEffectBlurSharpenWidget::settingsChangedDelayed);

    connect (m_amountInput, &kpIntNumInput::valueChanged,
             this, &kpEffectBlurSharpenWidget::slotUpdateTypeLabel);
}

kpEffectBlurSharpenWidget::~kpEffectBlurSharpenWidget () = default;


// public virtual [base kpEffectWidgetBase]
QString kpEffectBlurSharpenWidget::caption () const
{
    return {};
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectBlurSharpenWidget::isNoOp () const
{
    return (type () == kpEffectBlurSharpen::None);
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectBlurSharpenWidget::applyEffect (const kpImage &image)
{
    return kpEffectBlurSharpen::applyEffect (image,
        type (), strength ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectBlurSharpenWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectBlurSharpenCommand (type (), strength (),
                                           m_actOnSelection,
                                           cmdEnviron);
}


// protected slot
void kpEffectBlurSharpenWidget::slotUpdateTypeLabel ()
{
    QString text = kpEffectBlurSharpenCommand::nameForType (type ());

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    qCDebug(kpLogWidgets) << "kpEffectBlurSharpenWidget::slotUpdateTypeLabel() text="
               << text;
#endif
    const int h = m_typeLabel->height ();
    m_typeLabel->setText (text);
    if (m_typeLabel->height () != h)
    {
        qCCritical(kpLogWidgets) << "Label changed height despite the hack in ctor:"
                  << "was=" << h
                  << "now=" << m_typeLabel->height ();
    }
}


// protected
kpEffectBlurSharpen::Type kpEffectBlurSharpenWidget::type () const
{
    if (m_amountInput->value () == 0) {
        return kpEffectBlurSharpen::None;
    }

    if (m_amountInput->value () < 0) {
        return kpEffectBlurSharpen::Blur;
    }

    return kpEffectBlurSharpen::Sharpen;
}

// protected
int kpEffectBlurSharpenWidget::strength () const
{
    return qAbs (m_amountInput->value ());
}


