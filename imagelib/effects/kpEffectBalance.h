
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectBalance_H
#define kpEffectBalance_H

#include "imagelib/kpImage.h"

class kpEffectBalance
{
public:
    enum Channel {
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        RGB = Red | Green | Blue
    };

    // (<brightness>, <contrast> & <gamma> are from -50 to 50)
    static kpImage applyEffect(const kpImage &image, int channels, int brightness, int contrast, int gamma);
};

#endif // kpEffectBalance_H
