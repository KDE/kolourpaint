
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_EMBOSS 0

#include "kpEffectEmbossWidget.h"

#include <QCheckBox>
#include <QGridLayout>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "commands/imagelib/effects/kpEffectEmbossCommand.h"
#include "imagelib/effects/kpEffectEmboss.h"

kpEffectEmbossWidget::kpEffectEmbossWidget(bool actOnSelection, QWidget *parent)
    : kpEffectWidgetBase(actOnSelection, parent)
{
    auto *lay = new QGridLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    m_enableCheckBox = new QCheckBox(i18n("E&nable"), this);

    lay->addWidget(m_enableCheckBox, 0, 0, 1, 2, Qt::AlignCenter);

    // (settingsChangedDelayed() instead of settingsChanged() so that the
    //  user can quickly press OK to apply effect to document directly and
    //  not have to wait for the also slow preview)
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &kpEffectEmbossWidget::settingsChangedDelayed);
}

kpEffectEmbossWidget::~kpEffectEmbossWidget() = default;

// public virtual [base kpEffectWidgetBase]
QString kpEffectEmbossWidget::caption() const
{
    return {};
}

// public virtual [base kpEffectWidgetBase]
bool kpEffectEmbossWidget::isNoOp() const
{
    // return (m_amountInput->value () == 0);
    return !m_enableCheckBox->isChecked();
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectEmbossWidget::applyEffect(const kpImage &image)
{
    if (isNoOp()) {
        return image;
    }

    return kpEffectEmboss::applyEffect(image, strength());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectEmbossWidget::createCommand(kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectEmbossCommand(strength(), m_actOnSelection, cmdEnviron);
}

// protected
int kpEffectEmbossWidget::strength() const
{
    return kpEffectEmboss::MaxStrength;
}

#include "moc_kpEffectEmbossWidget.cpp"
