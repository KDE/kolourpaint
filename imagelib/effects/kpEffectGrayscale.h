
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectGrayscale_H
#define kpEffectGrayscale_H

#include "imagelib/kpImage.h"

//
// Converts the image to grayscale.
//

class kpEffectGrayscale
{
public:
    static kpImage applyEffect(const kpImage &image);
};

#endif // kpEffectGrayscale_H
