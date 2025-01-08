
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_PIXMAP_FX 0

#include "kpEffectGrayscale.h"

#include "pixmapfx/kpPixmapFX.h"

static QRgb toGray(QRgb rgb)
{
    // naive way that doesn't preserve brightness
    // int gray = (qRed (rgb) + qGreen (rgb) + qBlue (rgb)) / 3;

    // over-exaggerates red & blue
    // int gray = qGray (rgb);

    int gray = (212671 * qRed(rgb) + 715160 * qGreen(rgb) + 72169 * qBlue(rgb)) / 1000000;
    return qRgba(gray, gray, gray, qAlpha(rgb));
}

// public static
kpImage kpEffectGrayscale::applyEffect(const kpImage &image)
{
    kpImage qimage(image);

    // TODO: Why not just write to the kpImage directly?
    if (qimage.depth() > 8) {
        for (int y = 0; y < qimage.height(); y++) {
            for (int x = 0; x < qimage.width(); x++) {
                qimage.setPixel(x, y, toGray(qimage.pixel(x, y)));
            }
        }
    } else {
        // 1- & 8- bit images use a color table
        for (int i = 0; i < qimage.colorCount(); i++) {
            qimage.setColor(i, toGray(qimage.color(i)));
        }
    }

    return qimage;
}
