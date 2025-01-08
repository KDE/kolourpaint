/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectFlatten.h"
#include "blitz.h"

//--------------------------------------------------------------------------------
// public static

void kpEffectFlatten::applyEffect(QImage *destImagePtr, const QColor &color1, const QColor &color2)
{
    if (!destImagePtr) {
        return;
    }

    Blitz::flatten(*destImagePtr /*ref*/, color1, color2);
}

//--------------------------------------------------------------------------------
// public static

QImage kpEffectFlatten::applyEffect(const QImage &img, const QColor &color1, const QColor &color2)
{
    QImage retImage = img;
    applyEffect(&retImage, color1, color2);
    return retImage;
}

//--------------------------------------------------------------------------------
