
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


#include <kpEffectBalance.h>

#include <math.h>

#include <qimage.h>

#include <kdebug.h>

#include <kpPixmapFX.h>


#if DEBUG_KP_EFFECT_BALANCE
    #include <qdatetime.h>
#endif


static inline int between0And255 (int val)
{
    if (val < 0)
        return 0;
    else if (val > 255)
        return 255;
    else
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
    return between0And255 (qRound (255.0 * pow (base / 255.0, 1.0 / pow (10., strength / 50.0))));
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

static inline QRgb brightnessContrastGammaForRGB (QRgb rgb,
    int channels,
    int brightness, int contrast, int gamma)
{
    int red = qRed (rgb);
    int green = qGreen (rgb);
    int blue = qBlue (rgb);


    if (channels & kpEffectBalance::Red)
        red = brightnessContrastGamma (red, brightness, contrast, gamma);
    if (channels & kpEffectBalance::Green)
        green = brightnessContrastGamma (green, brightness, contrast, gamma);
    if (channels & kpEffectBalance::Blue)
        blue = brightnessContrastGamma (blue, brightness, contrast, gamma);


    return qRgba (red, green, blue, qAlpha (rgb));
}


// public static
kpImage kpEffectBalance::applyEffect (const kpImage &image,
        int channels,
        int brightness, int contrast, int gamma)
{
#if DEBUG_KP_EFFECT_BALANCE
    kDebug () << "kpEffectBalance::applyEffect("
               << "channels=" << channels
               << ",brightness=" << brightness
               << ",contrast=" << contrast
               << ",gamma=" << gamma
               << ")" << endl;
    QTime timer; timer.start ();
#endif

    QImage qimage = image;
#if DEBUG_KP_EFFECT_BALANCE
    kDebug () << "\tconvertToImage=" << timer.restart ();
#endif


    quint8 transformRed [256],
            transformGreen [256],
            transformBlue [256];

    for (int i = 0; i < 256; i++)
    {
        quint8 applied = (quint8) brightnessContrastGamma (i, brightness, contrast, gamma);

        if (channels & kpEffectBalance::Red)
            transformRed [i] = applied;
        else
            transformRed [i] = i;

        if (channels & kpEffectBalance::Green)
            transformGreen [i] = applied;
        else
            transformGreen [i] = i;

        if (channels & kpEffectBalance::Blue)
            transformBlue [i] = applied;
        else
            transformBlue [i] = i;
    }

#if DEBUG_KP_EFFECT_BALANCE
    kDebug () << "\tbuild lookup=" << timer.restart ();
#endif


    if (qimage.depth () > 8)
    {
        for (int y = 0; y < qimage.height (); y++)
        {
            for (int x = 0; x < qimage.width (); x++)
            {
                const QRgb rgb = qimage.pixel (x, y);

                const quint8 red = (quint8) qRed (rgb);
                const quint8 green = (quint8) qGreen (rgb);
                const quint8 blue = (quint8) qBlue (rgb);
                const quint8 alpha = (quint8) qAlpha (rgb);

                qimage.setPixel (x, y,
                    qRgba (transformRed [red],
                           transformGreen [green],
                           transformBlue [blue],
                           alpha));

            #if 0
                qimage.setPixel (x, y,
                    brightnessContrastGammaForRGB (qimage.pixel (x, y),
                        channels,
                        brightness, contrast, gamma));
            #endif
            }
        }
    }
    else
    {
        for (int i = 0; i < qimage.numColors (); i++)
        {
            const QRgb rgb = qimage.color (i);

            const quint8 red = (quint8) qRed (rgb);
            const quint8 green = (quint8) qGreen (rgb);
            const quint8 blue = (quint8) qBlue (rgb);
            const quint8 alpha = (quint8) qAlpha (rgb);

            qimage.setColor (i,
                qRgba (transformRed [red],
                       transformGreen [green],
                       transformBlue [blue],
                       alpha));

        #if 0
            qimage.setColor (i,
                brightnessContrastGammaForRGB (qimage.color (i),
                    channels,
                    brightness, contrast, gamma));
        #endif
        }

    }

    return qimage;
}

