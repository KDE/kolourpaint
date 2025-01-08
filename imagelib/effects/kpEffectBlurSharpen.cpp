/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_BLUR_SHARPEN 0

#include "kpEffectBlurSharpen.h"
#include "blitz.h"

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
#include <QTime>
#endif

//---------------------------------------------------------------------

//
// For info on "radius" and "sigma", see https://redskiesatnight.com/2005/04/06/sharpening-using-image-magick/
//
// Daniel Duley says:
//
// <quote>
// I don't think I can describe it any better than the article: The radius
// controls many how pixels are taken into account when determining the value
// of the center pixel. This controls the quality [and speed] of the result but not
// necessarily the strength. The sigma controls how those neighboring pixels
// are weighted depending on how far they are from the center one. This is
// closer to strength, but not exactly >:)
// </quote>
//

static QImage BlurQImage(const QImage &qimage, int strength)
{
    if (strength == 0) {
        return qimage;
    }

    // The numbers that follow were picked by experimentation to try to get
    // an effect linearly proportional to <strength> and at the same time,
    // be fast enough.
    //
    // I still have no idea what "radius" means.

    const double RadiusMin = 1;
    const double RadiusMax = 10;
    const double radius = RadiusMin + (strength - 1) * (RadiusMax - RadiusMin) / (kpEffectBlurSharpen::MaxStrength - 1);

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    qCDebug(kpLogImagelib) << "kpEffectBlurSharpen.cpp:BlurQImage(strength=" << strength << ")"
                           << " radius=" << radius;
#endif

    QImage img(qimage);
    return Blitz::blur(img, qRound(radius));
}

//---------------------------------------------------------------------

static QImage SharpenQImage(const QImage &qimage_, int strength)
{
    QImage qimage = qimage_;
    if (strength == 0) {
        return qimage;
    }

    // The numbers that follow were picked by experimentation to try to get
    // an effect linearly proportional to <strength> and at the same time,
    // be fast enough.
    //
    // I still have no idea what "radius" and "sigma" mean.

    const double RadiusMin = 0.1;
    const double RadiusMax = 2.5;
    const double radius = RadiusMin + (strength - 1) * (RadiusMax - RadiusMin) / (kpEffectBlurSharpen::MaxStrength - 1);

    const double SigmaMin = 0.5;
    const double SigmaMax = 3.0;
    const double sigma = SigmaMin + (strength - 1) * (SigmaMax - SigmaMin) / (kpEffectBlurSharpen::MaxStrength - 1);

    const double RepeatMin = 1;
    const double RepeatMax = 2;
    const double repeat = qRound(RepeatMin + (strength - 1) * (RepeatMax - RepeatMin) / (kpEffectBlurSharpen::MaxStrength - 1));

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    qCDebug(kpLogImagelib) << "kpEffectBlurSharpen.cpp:SharpenQImage(strength=" << strength << ")"
                           << " radius=" << radius << " sigma=" << sigma << " repeat=" << repeat;
#endif

    for (int i = 0; i < repeat; i++) {
#if DEBUG_KP_EFFECT_BLUR_SHARPEN
        QTime timer;
        timer.start();
#endif
        qimage = Blitz::gaussianSharpen(qimage, static_cast<float>(radius), static_cast<float>(sigma));
#if DEBUG_KP_EFFECT_BLUR_SHARPEN
        qCDebug(kpLogImagelib) << "\titeration #" + QString::number(i) << ": " + QString::number(timer.elapsed()) << "ms";
#endif
    }

    return qimage;
}

//---------------------------------------------------------------------

// public static
kpImage kpEffectBlurSharpen::applyEffect(const kpImage &image, Type type, int strength)
{
#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    qCDebug(kpLogImagelib) << "kpEffectBlurSharpen::applyEffect(image.rect=" << image.rect() << ",type=" << int(type) << ",strength=" << strength << ")";
#endif

    Q_ASSERT(strength >= MinStrength && strength <= MaxStrength);

    if (type == Blur) {
        return ::BlurQImage(image, strength);
    }

    if (type == Sharpen) {
        return ::SharpenQImage(image, strength);
    }

    if (type == MakeConfidential) {
        QImage img(image);
        return Blitz::blur(img, qMin(20, img.width() / 2));
    }

    return kpImage();
}

//---------------------------------------------------------------------
