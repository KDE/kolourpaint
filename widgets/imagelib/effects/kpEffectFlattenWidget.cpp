
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


#define DEBUG_KP_EFFECT_FLATTEN 0


#include "kpEffectFlattenWidget.h"

#include "kpDefs.h"
#include "imagelib/effects/kpEffectFlatten.h"
#include "commands/imagelib/effects/kpEffectFlattenCommand.h"
#include "kpLogCategories.h"

#include <KColorButton>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QCheckBox>
#include <QLayout>


// public static
QColor kpEffectFlattenWidget::s_lastColor1;
QColor kpEffectFlattenWidget::s_lastColor2;

kpEffectFlattenWidget::kpEffectFlattenWidget (bool actOnSelection,
                                              QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    if (!s_lastColor1.isValid () || !s_lastColor2.isValid ())
    {
        KConfigGroup cfgGroupSaver (KSharedConfig::openConfig (), kpSettingsGroupFlattenEffect);

        s_lastColor1 = cfgGroupSaver.readEntry (kpSettingFlattenEffectColor1, QColor ());
        if (!s_lastColor1.isValid ()) {
            s_lastColor1 = Qt::red;
        }

        s_lastColor2 = cfgGroupSaver.readEntry (kpSettingFlattenEffectColor2, QColor ());
        if (!s_lastColor2.isValid ()) {
            s_lastColor2 = Qt::blue;
        }
    }


    m_enableCheckBox = new QCheckBox (i18n ("E&nable"), this);

    m_color1Button = new KColorButton (s_lastColor1, this);
    m_color2Button = new KColorButton (s_lastColor2, this);


    m_color1Button->setEnabled (false);
    m_color2Button->setEnabled (false);

    auto *lay = new QVBoxLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget (m_enableCheckBox);
    lay->addWidget (m_color1Button);
    lay->addWidget (m_color2Button);

    connect (m_enableCheckBox, &QCheckBox::toggled,
             this, &kpEffectFlattenWidget::slotEnableChanged);

    connect (m_color1Button, &KColorButton::changed,
             this, &kpEffectFlattenWidget::settingsChanged);

    connect (m_color2Button, &KColorButton::changed,
             this, &kpEffectFlattenWidget::settingsChanged);
}

kpEffectFlattenWidget::~kpEffectFlattenWidget ()
{
    s_lastColor1 = color1 ();
    s_lastColor2 = color2 ();


    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupFlattenEffect);

    cfg.writeEntry (kpSettingFlattenEffectColor1, s_lastColor1);
    cfg.writeEntry (kpSettingFlattenEffectColor2, s_lastColor2);
    cfg.sync ();
}


// public
QColor kpEffectFlattenWidget::color1 () const
{
    return m_color1Button->color ();
}

// public
QColor kpEffectFlattenWidget::color2 () const
{
    return m_color2Button->color ();
}


//
// kpEffectFlattenWidget implements kpEffectWidgetBase interface
//

// public virtual [base kpEffectWidgetBase]
QString kpEffectFlattenWidget::caption () const
{
    return i18n ("Colors");
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectFlattenWidget::isNoOp () const
{
    return !m_enableCheckBox->isChecked ();
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectFlattenWidget::applyEffect (const kpImage &image)
{
#if DEBUG_KP_EFFECT_FLATTEN
    qCDebug(kpLogWidgets) << "kpEffectFlattenWidget::applyEffect() nop="
               << isNoOp () << endl;
#endif

    if (isNoOp ()) {
        return image;
    }

    return kpEffectFlatten::applyEffect (image, color1 (), color2 ());
}


// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectFlattenWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectFlattenCommand (color1 (), color2 (),
                                       m_actOnSelection,
                                       cmdEnviron);
}


// protected slot:
void kpEffectFlattenWidget::slotEnableChanged (bool enable)
{
#if DEBUG_KP_EFFECT_FLATTEN
    qCDebug(kpLogWidgets) << "kpEffectFlattenWidget::slotEnableChanged(" << enable
               << ") enableButton=" << m_enableCheckBox->isChecked ()
               << endl;
#endif

    m_color1Button->setEnabled (enable);
    m_color2Button->setEnabled (enable);

    emit settingsChanged ();
}


