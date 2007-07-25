
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

// LOTODO: Remove unused headers

#include <kpEffectToneEnhanceWidget.h>

#include <qimage.h>
#include <qrect.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpEffectToneEnhance.h>
#include <kpEffectToneEnhanceCommand.h>
#include <kpPixmapFX.h>

#include <stdlib.h>


kpEffectToneEnhanceWidget::kpEffectToneEnhanceWidget (bool actOnSelection,
                                                      QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    QGridLayout *lay = new QGridLayout (this);
    lay->setSpacing (spacingHint ());
    lay->setMargin (marginHint ());

    QLabel *granularityLabel = new QLabel (i18n ("&Granularity:"), this);
    QLabel *amountLabel = new QLabel (i18n ("&Amount:"), this);
    m_granularityInput = new KDoubleNumInput (this);
    m_granularityInput->setRange (0, 1, .1/*step*/, true/*slider*/);
    m_amountInput = new KDoubleNumInput (this);
    m_amountInput->setRange (0, 1, .1/*step*/, true/*slider*/);

    granularityLabel->setBuddy (m_granularityInput);
    amountLabel->setBuddy (m_amountInput);


    lay->addWidget (granularityLabel, 0, 0);
    lay->addWidget (m_granularityInput, 0, 1);
    lay->addWidget (amountLabel, 1, 0);
    lay->addWidget (m_amountInput, 1, 1);

    lay->setColumnStretch (1, 1);


    connect (m_granularityInput, SIGNAL (valueChanged (double)),
             this, SIGNAL (settingsChangedDelayed ()));

    connect (m_amountInput, SIGNAL (valueChanged (double)),
             this, SIGNAL (settingsChangedDelayed ()));
}

kpEffectToneEnhanceWidget::~kpEffectToneEnhanceWidget ()
{
}


// public virtual [base kpEffectWidgetBase]
QString kpEffectToneEnhanceWidget::caption () const
{
    // TODO: Why doesn't this have a caption?  Ditto for the other effects.
    return QString::null;
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectToneEnhanceWidget::isNoOp () const
{
    if (m_amountInput->value () == 0)
        return true;
    else
        return false;
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectToneEnhanceWidget::applyEffect (const kpImage &image)
{
    return kpEffectToneEnhance::applyEffect (image,
        m_granularityInput->value (), m_amountInput->value ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectToneEnhanceWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectToneEnhanceCommand (m_granularityInput->value (), m_amountInput->value (),
                                           m_actOnSelection,
                                           cmdEnviron);
}


#include <kpEffectToneEnhanceWidget.moc>