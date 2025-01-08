
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectBlurSharpenWidget_H
#define kpEffectBlurSharpenWidget_H

#include "imagelib/kpColor.h"

#include "imagelib/effects/kpEffectBlurSharpen.h"
#include "kpEffectWidgetBase.h"

class QLabel;

class kpIntNumInput;

class kpEffectBlurSharpenWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectBlurSharpenWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectBlurSharpenWidget() override;

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected Q_SLOTS:
    void slotUpdateTypeLabel();

protected:
    kpEffectBlurSharpen::Type type() const;
    int strength() const;

    kpIntNumInput *m_amountInput;
    QLabel *m_typeLabel;
};

#endif // kpEffectBlurSharpenWidget_H
