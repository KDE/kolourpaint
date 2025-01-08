
/*
   SPDX-FileCopyrightText: 2007 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectHSV_H
#define kpEffectHSV_H

#include "imagelib/kpImage.h"

class kpEffectHSV
{
public:
    static kpImage applyEffect(const kpImage &image, double hue, double saturation, double value);
};

#endif // kpEffectHSV_H
