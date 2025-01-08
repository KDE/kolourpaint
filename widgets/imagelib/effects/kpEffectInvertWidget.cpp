
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_INVERT 0

#include "kpEffectInvertWidget.h"

#include "commands/imagelib/effects/kpEffectInvertCommand.h"
#include "imagelib/effects/kpEffectInvert.h"
#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QCheckBox>
#include <QVBoxLayout>

kpEffectInvertWidget::kpEffectInvertWidget(bool actOnSelection, QWidget *parent)
    : kpEffectWidgetBase(actOnSelection, parent)
{
    auto *topLevelLay = new QVBoxLayout(this);
    topLevelLay->setContentsMargins(0, 0, 0, 0);

    auto *centerWidget = new QWidget(this);
    topLevelLay->addWidget(centerWidget, 0 /*stretch*/, Qt::AlignCenter);

    auto *centerWidgetLay = new QVBoxLayout(centerWidget);
    centerWidgetLay->setContentsMargins(0, 0, 0, 0);

    m_redCheckBox = new QCheckBox(i18n("&Red"), centerWidget);
    m_greenCheckBox = new QCheckBox(i18n("&Green"), centerWidget);
    m_blueCheckBox = new QCheckBox(i18n("&Blue"), centerWidget);

    auto *spaceWidget = new QWidget(centerWidget);
    spaceWidget->setFixedSize(1, fontMetrics().height() / 4);

    m_allCheckBox = new QCheckBox(i18n("&All"), centerWidget);

    m_redCheckBox->setChecked(false);
    m_greenCheckBox->setChecked(false);
    m_blueCheckBox->setChecked(false);

    m_allCheckBox->setChecked(false);

    centerWidgetLay->addWidget(m_redCheckBox);
    centerWidgetLay->addWidget(m_greenCheckBox);
    centerWidgetLay->addWidget(m_blueCheckBox);

    centerWidgetLay->addWidget(spaceWidget);

    centerWidgetLay->addWidget(m_allCheckBox);

    m_inSignalHandler = false;
    connect(m_redCheckBox, &QCheckBox::toggled, this, &kpEffectInvertWidget::slotRGBCheckBoxToggled);

    connect(m_greenCheckBox, &QCheckBox::toggled, this, &kpEffectInvertWidget::slotRGBCheckBoxToggled);

    connect(m_blueCheckBox, &QCheckBox::toggled, this, &kpEffectInvertWidget::slotRGBCheckBoxToggled);

    connect(m_allCheckBox, &QCheckBox::toggled, this, &kpEffectInvertWidget::slotAllCheckBoxToggled);
}

kpEffectInvertWidget::~kpEffectInvertWidget() = default;

// public
int kpEffectInvertWidget::channels() const
{
#if DEBUG_KP_EFFECT_INVERT
    qCDebug(kpLogWidgets) << "kpEffectInvertWidget::channels()"
                          << " isChecked: r=" << m_redCheckBox->isChecked() << " g=" << m_greenCheckBox->isChecked() << " b=" << m_blueCheckBox->isChecked()
                          << endl;
#endif

    int channels = 0;

    if (m_redCheckBox->isChecked()) {
        channels |= kpEffectInvert::Red;
    }

    if (m_greenCheckBox->isChecked()) {
        channels |= kpEffectInvert::Green;
    }

    if (m_blueCheckBox->isChecked()) {
        channels |= kpEffectInvert::Blue;
    }

#if DEBUG_KP_EFFECT_INVERT
    qCDebug(kpLogWidgets) << "\treturning channels=" << (int *)channels;
#endif
    return channels;
}

//
// kpEffectInvertWidget implements kpEffectWidgetBase interface
//

// public virtual [base kpEffectWidgetBase]
QString kpEffectInvertWidget::caption() const
{
    return i18n("Channels");
}

// public virtual [base kpEffectWidgetBase]
bool kpEffectInvertWidget::isNoOp() const
{
    return (channels() == kpEffectInvert::None);
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectInvertWidget::applyEffect(const kpImage &image)
{
    return kpEffectInvert::applyEffect(image, channels());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectInvertWidget::createCommand(kpCommandEnvironment *cmdEnviron) const
{
    return new kpEffectInvertCommand(channels(), m_actOnSelection, cmdEnviron);
}

// protected slots
void kpEffectInvertWidget::slotRGBCheckBoxToggled()
{
    if (m_inSignalHandler) {
        return;
    }

    m_inSignalHandler = true;

    // blockSignals (true);
    m_allCheckBox->setChecked(m_redCheckBox->isChecked() && m_blueCheckBox->isChecked() && m_greenCheckBox->isChecked());
    // blockSignals (false);

    Q_EMIT settingsChanged();

    m_inSignalHandler = false;
}

// protected slot
void kpEffectInvertWidget::slotAllCheckBoxToggled()
{
    if (m_inSignalHandler) {
        return;
    }

    m_inSignalHandler = true;

    // blockSignals (true);
    m_redCheckBox->setChecked(m_allCheckBox->isChecked());
    m_greenCheckBox->setChecked(m_allCheckBox->isChecked());
    m_blueCheckBox->setChecked(m_allCheckBox->isChecked());
    // blockSignals (false);

    Q_EMIT settingsChanged();

    m_inSignalHandler = false;
}

#include "moc_kpEffectInvertWidget.cpp"
