
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectEmbossCommand_H
#define kpEffectEmbossCommand_H

#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectEmbossCommand : public kpEffectCommandBase
{
public:
    kpEffectEmbossCommand(int strength, bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectEmbossCommand() override;

protected:
    kpImage applyEffect(const kpImage &image) override;

protected:
    int m_strength;
};

#endif // kpEffectEmbossCommand_H
