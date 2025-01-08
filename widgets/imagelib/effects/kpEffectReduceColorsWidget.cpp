
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_REDUCE_COLORS 0

#include "kpEffectReduceColorsWidget.h"

#include "commands/imagelib/effects/kpEffectReduceColorsCommand.h"
#include "imagelib/effects/kpEffectReduceColors.h"
#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QButtonGroup>
#include <QCheckBox>
#include <QImage>
#include <QRadioButton>
#include <QVBoxLayout>

kpEffectReduceColorsWidget::kpEffectReduceColorsWidget(bool actOnSelection, QWidget *parent)
    : kpEffectWidgetBase(actOnSelection, parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    m_blackAndWhiteRadioButton = new QRadioButton(i18n("&Monochrome"), this);

    m_blackAndWhiteDitheredRadioButton = new QRadioButton(i18n("Mo&nochrome (dithered)"), this);

    m_8BitRadioButton = new QRadioButton(i18n("256 co&lor"), this);

    m_8BitDitheredRadioButton = new QRadioButton(i18n("256 colo&r (dithered)"), this);

    m_24BitRadioButton = new QRadioButton(i18n("24-&bit color"), this);

    // LOCOMPAT: don't think this is needed
    auto *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_blackAndWhiteRadioButton);
    buttonGroup->addButton(m_blackAndWhiteDitheredRadioButton);
    buttonGroup->addButton(m_8BitRadioButton);
    buttonGroup->addButton(m_8BitDitheredRadioButton);
    buttonGroup->addButton(m_24BitRadioButton);

    m_defaultRadioButton = m_24BitRadioButton;
    m_defaultRadioButton->setChecked(true);

    lay->addWidget(m_blackAndWhiteRadioButton);
    lay->addWidget(m_blackAndWhiteDitheredRadioButton);
    lay->addWidget(m_8BitRadioButton);
    lay->addWidget(m_8BitDitheredRadioButton);
    lay->addWidget(m_24BitRadioButton);

    connect(m_blackAndWhiteRadioButton, &QRadioButton::toggled, this, &kpEffectReduceColorsWidget::settingsChanged);

    connect(m_blackAndWhiteDitheredRadioButton, &QRadioButton::toggled, this, &kpEffectReduceColorsWidget::settingsChanged);

    connect(m_8BitRadioButton, &QRadioButton::toggled, this, &kpEffectReduceColorsWidget::settingsChanged);

    connect(m_8BitDitheredRadioButton, &QRadioButton::toggled, this, &kpEffectReduceColorsWidget::settingsChanged);

    connect(m_24BitRadioButton, &QRadioButton::toggled, this, &kpEffectReduceColorsWidget::settingsChanged);
}

//---------------------------------------------------------------------

// public
int kpEffectReduceColorsWidget::depth() const
{
    // These values (1, 8, 32) are QImage's supported depths.
    // TODO: Qt-4.7.1: 1, 8, 16, 24 and 32
    if (m_blackAndWhiteRadioButton->isChecked() || m_blackAndWhiteDitheredRadioButton->isChecked()) {
        return 1;
    }

    if (m_8BitRadioButton->isChecked() || m_8BitDitheredRadioButton->isChecked()) {
        return 8;
    }

    if (m_24BitRadioButton->isChecked()) {
        return 32;
    }

    return 0;
}

//---------------------------------------------------------------------

// public
bool kpEffectReduceColorsWidget::dither() const
{
    return (m_blackAndWhiteDitheredRadioButton->isChecked() || m_8BitDitheredRadioButton->isChecked());
}

//---------------------------------------------------------------------
//
// kpEffectReduceColorsWidget implements kpEffectWidgetBase interface
//

// public virtual [base kpEffectWidgetBase]
QString kpEffectReduceColorsWidget::caption() const
{
    return i18n("Reduce To");
}

//---------------------------------------------------------------------

// public virtual [base kpEffectWidgetBase]
bool kpEffectReduceColorsWidget::isNoOp() const
{
    return (!m_defaultRadioButton || m_defaultRadioButton->isChecked());
}

//---------------------------------------------------------------------

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectReduceColorsWidget::applyEffect(const kpImage &image)
{
    return kpEffectReduceColors::applyEffect(image, depth(), dither());
}

//---------------------------------------------------------------------

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectReduceColorsWidget::createCommand(kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectReduceColorsCommand(depth(), dither(), m_actOnSelection, cmdEnviron);
}

//---------------------------------------------------------------------

#include "moc_kpEffectReduceColorsWidget.cpp"
