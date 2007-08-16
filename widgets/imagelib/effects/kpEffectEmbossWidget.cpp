
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


#define DEBUG_KP_EFFECT_EMBOSS 0


#include <kpEffectEmbossWidget.h>

#include <qcheckbox.h>
#include <qgridlayout.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpEffectEmboss.h>
#include <kpEffectEmbossCommand.h>


kpEffectEmbossWidget::kpEffectEmbossWidget (bool actOnSelection,
                                            QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    QGridLayout *lay = new QGridLayout (this);
    lay->setSpacing (spacingHint ());
    lay->setMargin (marginHint ());


#if 0
    QLabel *amountLabel = new QLabel (i18n ("&Amount:"), this);
    m_amountInput = new KIntNumInput (this);
    m_amountInput->setRange (kpEffectEmboss::MinStrength,
        kpEffectEmboss::MaxStrength, 1/*step*/, true/*slider*/);
    m_amountInput->setSpecialValueText (i18n ("None"));


    amountLabel->setBuddy (m_amountInput);


    lay->addWidget (amountLabel, 0, 0);
    lay->addWidget (m_amountInput, 0, 1);

    lay->setColumnStretch (1, 1);


    connect (m_amountInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));
#endif

    m_enableCheckBox = new QCheckBox (i18n ("E&nable"), this);


    lay->addWidget (m_enableCheckBox, 0, 0, 1, 2, Qt::AlignCenter);


    // (settingsChangedDelayed() instead of settingsChanged() so that the
    //  user can quickly press OK to apply effect to document directly and
    //  not have to wait for the also slow preview)
    connect (m_enableCheckBox, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChangedDelayed ()));
}

kpEffectEmbossWidget::~kpEffectEmbossWidget ()
{
}


// public virtual [base kpEffectWidgetBase]
QString kpEffectEmbossWidget::caption () const
{
    return QString();
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectEmbossWidget::isNoOp () const
{
    //return (m_amountInput->value () == 0);
    return !m_enableCheckBox->isChecked ();
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectEmbossWidget::applyEffect (const kpImage &image)
{
    if (isNoOp ())
        return image;

    return kpEffectEmboss::applyEffect (image, strength ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectEmbossWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectEmbossCommand (strength (),
                                      m_actOnSelection,
                                      cmdEnviron);
}

// protected
int kpEffectEmbossWidget::strength () const
{
    return kpEffectEmboss::MaxStrength;
}


#include <kpEffectEmbossWidget.moc>
