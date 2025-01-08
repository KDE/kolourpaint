
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_BALANCE 0

#include "kpEffectBalance.h"

#include <cmath>

#include <QImage>

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"

#if DEBUG_KP_EFFECT_BALANCE
#include <qdatetime.h>
#endif

static inline int between0And255(int val)
{
    if (val < 0) {
        return 0;
    }

    if (val > 255) {
        return 255;
    }

    return val;
}

static inline int brightness(int base, int strength)
{
    return between0And255(base + strength * 255 / 50);
}

static inline int contrast(int base, int strength)
{
    return between0And255((base - 127) * (strength + 50) / 50 + 127);
}

static inline int gamma(int base, int strength)
{
    return between0And255(qRound(255.0 * std::pow(base / 255.0, 1.0 / std::pow(10., strength / 50.0))));
}

static inline int brightnessContrastGamma(int base, int newBrightness, int newContrast, int newGamma)
{
    return gamma(contrast(brightness(base, newBrightness), newContrast), newGamma);
}

// public static
kpImage kpEffectBalance::applyEffect(const kpImage &image, int channels, int brightness, int contrast, int gamma)
{
#if DEBUG_KP_EFFECT_BALANCE
    qCDebug(kpLogImagelib) << "kpEffectBalance::applyEffect("
                           << "channels=" << channels << ",brightness=" << brightness << ",contrast=" << contrast << ",gamma=" << gamma << ")";
    QTime timer;
    timer.start();
#endif

    QImage qimage = image;
#if DEBUG_KP_EFFECT_BALANCE
    qCDebug(kpLogImagelib) << "\tconvertToImage=" << timer.restart();
#endif

    quint8 transformRed[256], transformGreen[256], transformBlue[256];

    for (int i = 0; i < 256; i++) {
        auto applied = static_cast<quint8>(brightnessContrastGamma(i, brightness, contrast, gamma));

        if (channels & kpEffectBalance::Red) {
            transformRed[i] = applied;
        } else {
            transformRed[i] = static_cast<quint8>(i);
        }

        if (channels & kpEffectBalance::Green) {
            transformGreen[i] = applied;
        } else {
            transformGreen[i] = static_cast<quint8>(i);
        }

        if (channels & kpEffectBalance::Blue) {
            transformBlue[i] = applied;
        } else {
            transformBlue[i] = static_cast<quint8>(i);
        }
    }

#if DEBUG_KP_EFFECT_BALANCE
    qCDebug(kpLogImagelib) << "\tbuild lookup=" << timer.restart();
#endif

    if (qimage.depth() > 8) {
        for (int y = 0; y < qimage.height(); y++) {
            for (int x = 0; x < qimage.width(); x++) {
                const QRgb rgb = qimage.pixel(x, y);

                const auto red = static_cast<quint8>(qRed(rgb));
                const auto green = static_cast<quint8>(qGreen(rgb));
                const auto blue = static_cast<quint8>(qBlue(rgb));
                const auto alpha = static_cast<quint8>(qAlpha(rgb));

                qimage.setPixel(x, y, qRgba(transformRed[red], transformGreen[green], transformBlue[blue], alpha));
            }
        }
    } else {
        for (int i = 0; i < qimage.colorCount(); i++) {
            const QRgb rgb = qimage.color(i);

            const auto red = static_cast<quint8>(qRed(rgb));
            const auto green = static_cast<quint8>(qGreen(rgb));
            const auto blue = static_cast<quint8>(qBlue(rgb));
            const auto alpha = static_cast<quint8>(qAlpha(rgb));

            qimage.setColor(i, qRgba(transformRed[red], transformGreen[green], transformBlue[blue], alpha));
        }
    }

    return qimage;
}
