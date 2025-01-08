
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2006 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectToneEnhanceWidget_H
#define kpEffectToneEnhanceWidget_H

#include "kpEffectWidgetBase.h"
#include "kpNumInput.h"

class kpDoubleNumInput;

class kpEffectToneEnhanceWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectToneEnhanceWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectToneEnhanceWidget() override;

    QString caption() const override;

private:
    double amount() const;
    double granularity() const;

public:
    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected:
    kpDoubleNumInput *m_granularityInput;
    kpDoubleNumInput *m_amountInput;
};

#endif // kpEffectToneEnhanceWidget_H
