
/*
   SPDX-FileCopyrightText: 2007 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectHSVWidget_H
#define kpEffectHSVWidget_H

#include "kpEffectWidgetBase.h"
#include "kpNumInput.h"

class kpDoubleNumInput;

class kpEffectHSVWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectHSVWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectHSVWidget() override;

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected:
    kpDoubleNumInput *m_hueInput;
    kpDoubleNumInput *m_saturationInput;
    kpDoubleNumInput *m_valueInput;
};

#endif // kpEffectHSVWidget_H
