
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectBlurSharpenCommand_H
#define kpEffectBlurSharpenCommand_H

#include "imagelib/effects/kpEffectBlurSharpen.h"
#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectBlurSharpenCommand : public kpEffectCommandBase
{
public:
    kpEffectBlurSharpenCommand(kpEffectBlurSharpen::Type type, int strength, bool actOnSelection, kpCommandEnvironment *environ);

    static QString nameForType(kpEffectBlurSharpen::Type type);

protected:
    kpImage applyEffect(const kpImage &image) override;

protected:
    kpEffectBlurSharpen::Type m_type;
    int m_strength;
};

#endif // kpEffectBlurSharpenCommand_H
