
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2006 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectToneEnhance_H
#define kpEffectToneEnhance_H

#include "imagelib/kpImage.h"

//
// Histogram Equalizer effect.
//
// It just divides out the color histogram from the pixel values. (So if
// you plot the color histogram after equalizing, you should get a nearly
// flat/uniform distribution.)
//
// The two sliders adjust:
//
// 1. The extent to which it equalizes
// 2. The local-ness of the equalization. (In other words, it computes just
//    the histogram of the regions near the pixel it is adjusting.)
//
// ASSUMPTION: The given <image> is not paletted (currently, this is the
//             same as the screen mode not being paletted).
//
class kpEffectToneEnhance
{
public:
    static kpImage applyEffect(const kpImage &image, double granularity, double amount);
};

#endif // kpEffectToneEnhance_H
