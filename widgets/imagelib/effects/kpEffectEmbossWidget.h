
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectEmbossWidget_H
#define kpEffectEmbossWidget_H

#include "imagelib/kpColor.h"
#include "kpEffectWidgetBase.h"

class QCheckBox;

class kpEffectEmbossWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectEmbossWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectEmbossWidget() override;

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected:
    int strength() const;

    QCheckBox *m_enableCheckBox;
};

#endif // kpEffectEmbossWidget_H
