
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


#define DEBUG_KP_EFFECT_REDUCE_COLORS 0


#include "kpEffectReduceColorsWidget.h"

#include "imagelib/effects/kpEffectReduceColors.h"
#include "commands/imagelib/effects/kpEffectReduceColorsCommand.h"
#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QBitmap>
#include <QButtonGroup>
#include <QCheckBox>
#include <QImage>
#include <QLayout>
#include <QPixmap>
#include <QRadioButton>


kpEffectReduceColorsWidget::kpEffectReduceColorsWidget (bool actOnSelection,
                                                        QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, parent)
{
    auto *lay = new QVBoxLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);

    m_blackAndWhiteRadioButton =
        new QRadioButton (i18n ("&Monochrome"), this);

    m_blackAndWhiteDitheredRadioButton =
        new QRadioButton (i18n ("Mo&nochrome (dithered)"), this);

    m_8BitRadioButton = new QRadioButton (i18n ("256 co&lor"), this);

    m_8BitDitheredRadioButton = new QRadioButton (i18n ("256 colo&r (dithered)"), this);

    m_24BitRadioButton = new QRadioButton (i18n ("24-&bit color"), this);


    // LOCOMPAT: don't think this is needed
    auto *buttonGroup = new QButtonGroup (this);
    buttonGroup->addButton (m_blackAndWhiteRadioButton);
    buttonGroup->addButton (m_blackAndWhiteDitheredRadioButton);
    buttonGroup->addButton (m_8BitRadioButton);
    buttonGroup->addButton (m_8BitDitheredRadioButton);
    buttonGroup->addButton (m_24BitRadioButton);

    m_defaultRadioButton = m_24BitRadioButton;
    m_defaultRadioButton->setChecked (true);

    lay->addWidget (m_blackAndWhiteRadioButton);
    lay->addWidget (m_blackAndWhiteDitheredRadioButton);
    lay->addWidget (m_8BitRadioButton);
    lay->addWidget (m_8BitDitheredRadioButton);
    lay->addWidget (m_24BitRadioButton);

    connect (m_blackAndWhiteRadioButton, &QRadioButton::toggled,
             this, &kpEffectReduceColorsWidget::settingsChanged);

    connect (m_blackAndWhiteDitheredRadioButton, &QRadioButton::toggled,
             this, &kpEffectReduceColorsWidget::settingsChanged);

    connect (m_8BitRadioButton, &QRadioButton::toggled,
             this, &kpEffectReduceColorsWidget::settingsChanged);

    connect (m_8BitDitheredRadioButton, &QRadioButton::toggled,
             this, &kpEffectReduceColorsWidget::settingsChanged);

    connect (m_24BitRadioButton, &QRadioButton::toggled,
             this, &kpEffectReduceColorsWidget::settingsChanged);
}

//---------------------------------------------------------------------

// public
int kpEffectReduceColorsWidget::depth () const
{
    // These values (1, 8, 32) are QImage's supported depths.
    // TODO: Qt-4.7.1: 1, 8, 16, 24 and 32
    if (m_blackAndWhiteRadioButton->isChecked () ||
        m_blackAndWhiteDitheredRadioButton->isChecked ())
    {
        return 1;
    }

    if (m_8BitRadioButton->isChecked () ||
             m_8BitDitheredRadioButton->isChecked ())
    {
        return 8;
    }

    if (m_24BitRadioButton->isChecked ())
    {
        return 32;
    }

    return 0;
}

//---------------------------------------------------------------------

// public
bool kpEffectReduceColorsWidget::dither () const
{
    return (m_blackAndWhiteDitheredRadioButton->isChecked () ||
            m_8BitDitheredRadioButton->isChecked ());
}

//---------------------------------------------------------------------
//
// kpEffectReduceColorsWidget implements kpEffectWidgetBase interface
//

// public virtual [base kpEffectWidgetBase]
QString kpEffectReduceColorsWidget::caption () const
{
    return i18n ("Reduce To");
}

//---------------------------------------------------------------------

// public virtual [base kpEffectWidgetBase]
bool kpEffectReduceColorsWidget::isNoOp () const
{
    return (!m_defaultRadioButton || m_defaultRadioButton->isChecked ());
}

//---------------------------------------------------------------------

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectReduceColorsWidget::applyEffect (const kpImage &image)
{
    return kpEffectReduceColors::applyEffect (image, depth (), dither ());
}

//---------------------------------------------------------------------

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectReduceColorsWidget::createCommand (
        kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectReduceColorsCommand (depth (), dither (),
                                            m_actOnSelection,
                                            cmdEnviron);
}

//---------------------------------------------------------------------

