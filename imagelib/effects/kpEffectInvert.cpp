
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_INVERT 0

#include "kpEffectInvert.h"

#include <QImage>

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"

// public static
void kpEffectInvert::applyEffect(QImage *destImagePtr, int channels)
{
    if (channels == kpEffectInvert::RGB) {
        destImagePtr->invertPixels();
        return;
    }

    QRgb mask = qRgba((channels & Red) ? 0xFF : 0, (channels & Green) ? 0xFF : 0, (channels & Blue) ? 0xFF : 0, 0 /*don't invert alpha*/);
#if DEBUG_KP_EFFECT_INVERT
    qCDebug(kpLogImagelib) << "kpEffectInvert::applyEffect(channels=" << channels << ") mask=" << (int *)mask;
#endif

    if (destImagePtr->depth() > 8) {
        // Above version works for Qt 3.2 at least.
        // But this version will always work (slower, though) and supports
        // inverting particular channels.
        for (int y = 0; y < destImagePtr->height(); y++) {
            for (int x = 0; x < destImagePtr->width(); x++) {
                destImagePtr->setPixel(x, y, destImagePtr->pixel(x, y) ^ mask);
            }
        }
    } else {
        for (int i = 0; i < destImagePtr->colorCount(); i++) {
            destImagePtr->setColor(i, destImagePtr->color(i) ^ mask);
        }
    }
}

// public static
QImage kpEffectInvert::applyEffect(const QImage &img, int channels)
{
    QImage retImage = img;
    applyEffect(&retImage, channels);
    return retImage;
}
