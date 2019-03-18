/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DEBUG_KP_EFFECT_EMBOSS 0


#include "kpEffectEmboss.h"
#include "blitz.h"

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"


static QImage EmbossQImage (const QImage &qimage_, int strength)
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


    for (int i = 0; i < repeat; i++)
    {
        qimage = Blitz::emboss (qimage, radius, sigma);
    }


    return qimage;
}


// public static
kpImage kpEffectEmboss::applyEffect (const kpImage &image, int strength)
{
#if DEBUG_KP_EFFECT_EMBOSS
    qCDebug(kpLogImagelib) << "kpEffectEmboss::applyEffect(strength=" << strength << ")"
               << endl;
#endif

    Q_ASSERT (strength >= MinStrength && strength <= MaxStrength);

    return ::EmbossQImage (image, strength);
}
