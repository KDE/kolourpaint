/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_EMBOSS 0

#include "kpEffectEmboss.h"
#include "blitz.h"

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"

static QImage EmbossQImage(const QImage &qimage_, int strength)
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

    const auto radius = 0.0;

    const auto sigma = 1.0;

    const auto repeat = 1;

    for (int i = 0; i < repeat; i++) {
        qimage = Blitz::emboss(qimage, radius, sigma);
    }

    return qimage;
}

// public static
kpImage kpEffectEmboss::applyEffect(const kpImage &image, int strength)
{
#if DEBUG_KP_EFFECT_EMBOSS
    qCDebug(kpLogImagelib) << "kpEffectEmboss::applyEffect(strength=" << strength << ")" << endl;
#endif

    Q_ASSERT(strength >= MinStrength && strength <= MaxStrength);

    return ::EmbossQImage(image, strength);
}
