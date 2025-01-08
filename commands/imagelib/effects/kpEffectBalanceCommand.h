
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectBalanceCommand_H
#define kpEffectBalanceCommand_H

#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectBalanceCommand : public kpEffectCommandBase
{
public:
    // (<brightness>, <contrast> & <gamma> are from -50 to 50)
    kpEffectBalanceCommand(int channels, int brightness, int contrast, int gamma, bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectBalanceCommand() override;

protected:
    kpImage applyEffect(const kpImage &image) override;

protected:
    int m_channels;
    int m_brightness, m_contrast, m_gamma;
};

#endif // kpEffectBalanceCommand_H
