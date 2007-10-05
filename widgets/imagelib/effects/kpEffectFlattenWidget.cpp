
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


#include <kpEffectFlattenWidget.h>

#include <qcheckbox.h>
#include <qlayout.h>

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kvbox.h>

#include <kpDefs.h>
#include <kpEffectFlatten.h>
#include <kpEffectFlattenCommand.h>


// public static
QColor kpEffectFlattenWidget::s_lastColor1;
QColor kpEffectFlattenWidget::s_lastColor2;

kpEffectFlattenWidget::kpEffectFlattenWidget (bool actOnSelection,
                                              QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    if (!s_lastColor1.isValid () || !s_lastColor2.isValid ())
    {
        KConfigGroup cfgGroupSaver (KGlobal::config (), kpSettingsGroupFlattenEffect);

        s_lastColor1 = cfgGroupSaver.readEntry (kpSettingFlattenEffectColor1, QColor ());
        if (!s_lastColor1.isValid ())
            s_lastColor1 = Qt::red;

        s_lastColor2 = cfgGroupSaver.readEntry (kpSettingFlattenEffectColor2, QColor ());
        if (!s_lastColor2.isValid ())
            s_lastColor2 = Qt::blue;
    }


    m_enableCheckBox = new QCheckBox (i18n ("E&nable"), this);

    KVBox *colorButtonContainer = new KVBox (this);
    colorButtonContainer->setMargin (KDialog::marginHint () / 2);
    colorButtonContainer->setSpacing (spacingHint ());
    m_color1Button = new KColorButton (s_lastColor1, colorButtonContainer);
    m_color2Button = new KColorButton (s_lastColor2, colorButtonContainer);


    m_color1Button->setEnabled (false);
    m_color2Button->setEnabled (false);


    QVBoxLayout *lay = new QVBoxLayout (this);
    lay->setSpacing(spacingHint ());
    lay->setMargin(marginHint ());
    lay->addWidget (m_enableCheckBox);
    lay->addWidget (colorButtonContainer);


    connect (m_enableCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotEnableChanged (bool)));

    connect (m_color1Button, SIGNAL (changed (const QColor &)),
             this, SIGNAL (settingsChanged ()));
    connect (m_color2Button, SIGNAL (changed (const QColor &)),
             this, SIGNAL (settingsChanged ()));
}

kpEffectFlattenWidget::~kpEffectFlattenWidget ()
{
    s_lastColor1 = color1 ();
    s_lastColor2 = color2 ();


    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupFlattenEffect);

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
    kDebug () << "kpEffectFlattenWidget::applyEffect() nop="
               << isNoOp () << endl;
#endif

    if (isNoOp ())
        return image;

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
    kDebug () << "kpEffectFlattenWidget::slotEnableChanged(" << enable
               << ") enableButton=" << m_enableCheckBox->isChecked ()
               << endl;
#endif

    m_color1Button->setEnabled (enable);
    m_color2Button->setEnabled (enable);

    emit settingsChanged ();
}


#include <kpEffectFlattenWidget.moc>
