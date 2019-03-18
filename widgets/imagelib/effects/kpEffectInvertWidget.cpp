
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


#define DEBUG_KP_EFFECT_INVERT 0


#include "kpEffectInvertWidget.h"

#include "imagelib/effects/kpEffectInvert.h"
#include "commands/imagelib/effects/kpEffectInvertCommand.h"
#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QCheckBox>
#include <QImage>
#include <QLayout>
#include <QPixmap>


kpEffectInvertWidget::kpEffectInvertWidget (bool actOnSelection,
                                            QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    auto *topLevelLay = new QVBoxLayout (this);
    topLevelLay->setContentsMargins(0, 0, 0, 0);

    auto *centerWidget = new QWidget (this);
    topLevelLay->addWidget (centerWidget, 0/*stretch*/, Qt::AlignCenter);

    auto *centerWidgetLay = new QVBoxLayout (centerWidget );
    centerWidgetLay->setContentsMargins(0, 0, 0, 0);

    m_redCheckBox = new QCheckBox (i18n ("&Red"), centerWidget);
    m_greenCheckBox = new QCheckBox (i18n ("&Green"), centerWidget);
    m_blueCheckBox = new QCheckBox (i18n ("&Blue"), centerWidget);

    auto *spaceWidget = new QWidget (centerWidget);
    spaceWidget->setFixedSize (1, fontMetrics ().height () / 4);

    m_allCheckBox = new QCheckBox (i18n ("&All"), centerWidget);


    m_redCheckBox->setChecked (false);
    m_greenCheckBox->setChecked (false);
    m_blueCheckBox->setChecked (false);

    m_allCheckBox->setChecked (false);


    centerWidgetLay->addWidget (m_redCheckBox);
    centerWidgetLay->addWidget (m_greenCheckBox);
    centerWidgetLay->addWidget (m_blueCheckBox);

    centerWidgetLay->addWidget (spaceWidget);

    centerWidgetLay->addWidget (m_allCheckBox);


    m_inSignalHandler = false;
    connect (m_redCheckBox, &QCheckBox::toggled,
             this, &kpEffectInvertWidget::slotRGBCheckBoxToggled);

    connect (m_greenCheckBox, &QCheckBox::toggled,
             this, &kpEffectInvertWidget::slotRGBCheckBoxToggled);

    connect (m_blueCheckBox, &QCheckBox::toggled,
             this, &kpEffectInvertWidget::slotRGBCheckBoxToggled);

    connect (m_allCheckBox, &QCheckBox::toggled,
             this, &kpEffectInvertWidget::slotAllCheckBoxToggled);
}

kpEffectInvertWidget::~kpEffectInvertWidget () = default;


// public
int kpEffectInvertWidget::channels () const
{
#if DEBUG_KP_EFFECT_INVERT
    qCDebug(kpLogWidgets) << "kpEffectInvertWidget::channels()"
               << " isChecked: r=" << m_redCheckBox->isChecked ()
               << " g=" << m_greenCheckBox->isChecked ()
               << " b=" << m_blueCheckBox->isChecked ()
               << endl;
#endif

    int channels = 0;


    if (m_redCheckBox->isChecked ()) {
        channels |= kpEffectInvert::Red;
    }

    if (m_greenCheckBox->isChecked ()) {
        channels |= kpEffectInvert::Green;
    }

    if (m_blueCheckBox->isChecked ()) {
        channels |= kpEffectInvert::Blue;
    }


#if DEBUG_KP_EFFECT_INVERT
    qCDebug(kpLogWidgets) << "\treturning channels=" << (int *) channels;
#endif
    return channels;
}


//
// kpEffectInvertWidget implements kpEffectWidgetBase interface
//

// public virtual [base kpEffectWidgetBase]
QString kpEffectInvertWidget::caption () const
{
    return i18n ("Channels");
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectInvertWidget::isNoOp () const
{
    return (channels () == kpEffectInvert::None);
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectInvertWidget::applyEffect (const kpImage &image)
{
    return kpEffectInvert::applyEffect (image, channels ());
}


// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectInvertWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectInvertCommand (channels (),
                                      m_actOnSelection,
                                      cmdEnviron);
}


// protected slots
void kpEffectInvertWidget::slotRGBCheckBoxToggled ()
{
    if (m_inSignalHandler) {
        return;
    }

    m_inSignalHandler = true;

    //blockSignals (true);
    m_allCheckBox->setChecked (m_redCheckBox->isChecked () &&
                               m_blueCheckBox->isChecked () &&
                               m_greenCheckBox->isChecked ());
    //blockSignals (false);

    emit settingsChanged ();

    m_inSignalHandler = false;
}

// protected slot
void kpEffectInvertWidget::slotAllCheckBoxToggled ()
{
    if (m_inSignalHandler) {
        return;
    }

    m_inSignalHandler = true;

    //blockSignals (true);
    m_redCheckBox->setChecked (m_allCheckBox->isChecked ());
    m_greenCheckBox->setChecked (m_allCheckBox->isChecked ());
    m_blueCheckBox->setChecked (m_allCheckBox->isChecked ());
    //blockSignals (false);

    emit settingsChanged ();

    m_inSignalHandler = false;
}



