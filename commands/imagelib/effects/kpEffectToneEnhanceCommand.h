
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2006 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectToneEnhanceCommand_H
#define kpEffectToneEnhanceCommand_H

#include "kpEffectCommandBase.h"

class kpEffectToneEnhanceCommand : public kpEffectCommandBase
{
public:
    kpEffectToneEnhanceCommand(double granularity, double amount, bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectToneEnhanceCommand() override;

protected:
    kpImage applyEffect(const kpImage &image) override;

protected:
    double m_granularity, m_amount;
};

#endif // kpEffectToneEnhanceCommand_H
