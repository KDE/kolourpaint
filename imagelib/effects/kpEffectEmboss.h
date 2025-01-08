
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectEmboss_H
#define kpEffectEmboss_H

#include "imagelib/kpImage.h"

class kpEffectEmboss
{
public:
    // This will always be 0 - this constant will not change.
    static const int MinStrength = 0;

    static const int MaxStrength = 10;

    // <strength> = strength of the effect
    //              (must be between MinStrength and MaxStrength inclusive)
    //
    //              Currently, all non-zero strengths are the same.
    static kpImage applyEffect(const kpImage &image, int strength);
};

#endif // kpEffectEmboss_H
