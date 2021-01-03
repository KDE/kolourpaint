
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


#define DEBUG_KP_EFFECT_BALANCE 0


#include "kpEffectBalance.h"

#include <cmath>

#include <QImage>

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"


#if DEBUG_KP_EFFECT_BALANCE
    #include <qdatetime.h>
#endif


static inline int between0And255 (int val)
{
    if (val < 0) {
        return 0;
    }

    if (val > 255) {
        return 255;
    }

    return val;
}


static inline int brightness (int base, int strength)
{
    return between0And255 (base + strength * 255 / 50);
}

static inline int contrast (int base, int strength)
{
    return between0And255 ((base - 127) * (strength + 50) / 50 + 127);
}

static inline int gamma (int base, int strength)
{
    return between0And255 (qRound (255.0 * std::pow (base / 255.0, 1.0 / std::pow (10., strength / 50.0))));
}


static inline int brightnessContrastGamma (int base,
                                           int newBrightness,
                                           int newContrast,
                                           int newGamma)
{
    return gamma (contrast (brightness (base, newBrightness),
                            newContrast),
                  newGamma);
}

// public static
kpImage kpEffectBalance::applyEffect (const kpImage &image,
        int channels,
        int brightness, int contrast, int gamma)
{
#if DEBUG_KP_EFFECT_BALANCE
    qCDebug(kpLogImagelib) << "kpEffectBalance::applyEffect("
               << "channels=" << channels
               << ",brightness=" << brightness
               << ",contrast=" << contrast
               << ",gamma=" << gamma
               << ")";
    QTime timer; timer.start ();
#endif

    QImage qimage = image;
#if DEBUG_KP_EFFECT_BALANCE
    qCDebug(kpLogImagelib) << "\tconvertToImage=" << timer.restart ();
#endif


    quint8 transformRed [256],
            transformGreen [256],
            transformBlue [256];

    for (int i = 0; i < 256; i++)
    {
        auto applied = static_cast<quint8> (brightnessContrastGamma (i, brightness, contrast, gamma));

        if (channels & kpEffectBalance::Red) {
            transformRed [i] = applied;
        }
        else {
            transformRed [i] = static_cast<quint8> (i);
        }

        if (channels & kpEffectBalance::Green) {
            transformGreen [i] = applied;
        }
        else {
            transformGreen [i] = static_cast<quint8> (i);
        }

        if (channels & kpEffectBalance::Blue) {
            transformBlue [i] = applied;
        }
        else {
            transformBlue [i] = static_cast<quint8> (i);
        }
    }

#if DEBUG_KP_EFFECT_BALANCE
    qCDebug(kpLogImagelib) << "\tbuild lookup=" << timer.restart ();
#endif


    if (qimage.depth () > 8)
    {
        for (int y = 0; y < qimage.height (); y++)
        {
            for (int x = 0; x < qimage.width (); x++)
            {
                const QRgb rgb = qimage.pixel (x, y);

                const auto red = static_cast<quint8> (qRed (rgb));
                const auto green = static_cast<quint8> (qGreen (rgb));
                const auto blue = static_cast<quint8> (qBlue (rgb));
                const auto alpha = static_cast<quint8> (qAlpha (rgb));

                qimage.setPixel (x, y,
                    qRgba (transformRed [red],
                           transformGreen [green],
                           transformBlue [blue],
                           alpha));
            }
        }
    }
    else
    {
        for (int i = 0; i < qimage.colorCount (); i++)
        {
            const QRgb rgb = qimage.color (i);

            const auto red = static_cast<quint8> (qRed (rgb));
            const auto green = static_cast<quint8> (qGreen (rgb));
            const auto blue = static_cast<quint8> (qBlue (rgb));
            const auto alpha = static_cast<quint8> (qAlpha (rgb));

            qimage.setColor (i,
                qRgba (transformRed [red],
                       transformGreen [green],
                       transformBlue [blue],
                       alpha));
        }

    }

    return qimage;
}

