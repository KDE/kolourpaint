
/*
   SPDX-FileCopyrightText: 2007 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectHSVCommand_H
#define kpEffectHSVCommand_H

#include "kpEffectCommandBase.h"

class kpEffectHSVCommand : public kpEffectCommandBase
{
public:
    kpEffectHSVCommand(double hue, double saturation, double value, bool actOnSelection, kpCommandEnvironment *environ);

protected:
    kpImage applyEffect(const kpImage &image) override;

protected:
    double m_hue, m_saturation, m_value;
};

#endif // kpEffectHSVCommand_H
