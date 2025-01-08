
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectBlurSharpen_H
#define kpEffectBlurSharpen_H

#include "imagelib/kpImage.h"

class kpEffectBlurSharpen
{
public:
    enum Type {
        None = 0,
        Blur,
        Sharpen,
        MakeConfidential
    };

    // Blur or Sharpen with this strength is the same as None.
    // This will always be 0 - this constant will not change.
    static const int MinStrength = 0;

    static const int MaxStrength = 10;

    // <strength> = strength of the effect
    //              (must be between MinStrength and MaxStrength inclusive)
    static kpImage applyEffect(const kpImage &image, Type type, int strength);
};

#endif // kpEffectBlurSharpen_H
