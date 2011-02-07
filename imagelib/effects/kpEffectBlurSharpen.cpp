
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


#define DEBUG_KP_EFFECT_BLUR_SHARPEN 0


#include <kpEffectBlurSharpen.h>

#include <qimageblitz.h>

#include <qbitmap.h>
#include <qimage.h>

#include <kdebug.h>

#include <kpPixmapFX.h>


#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    #include <QTime>
#endif


//---------------------------------------------------------------------

//
// For info on "radius" and "sigma", see http://redskiesatnight.com/Articles/IMsharpen/
//
// Daniel Duley says:
//
// <quote>
// I don't think I can describe it any better than the article: The radius
// controls many how pixels are taken into account when determining the value
// of the center pixel. This controls the quality [and speed] of the result but not
// necessarily the strength. The sigma controls how those neighboring pixels
// are weighted depending on how far the are from the center one. This is
// closer to strength, but not exactly >:)
// </quote>
//


static QImage BlurQImage (const QImage qimage_, int strength)
{
    QImage qimage = qimage_;
    if (strength == 0)
        return qimage;


    // The numbers that follow were picked by experimentation to try to get
    // an effect linearly proportional to <strength> and at the same time,
    // be fast enough.
    //
    // I still have no idea what "radius" means.

    const double RadiusMin = 1;
    const double RadiusMax = 10;
    const double radius = RadiusMin +
        (strength - 1) *
        (RadiusMax - RadiusMin) /
        (kpEffectBlurSharpen::MaxStrength - 1);

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    kDebug () << "kpEffectBlurSharpen.cpp:BlurQImage(strength=" << strength << ")"
               << " radius=" << radius
               << endl;
#endif


    qimage = Blitz::blur (qimage, qRound (radius));


    return qimage;
}

//---------------------------------------------------------------------

static QImage SharpenQImage (const QImage &qimage_, int strength)
{
    QImage qimage = qimage_;
    if (strength == 0)
        return qimage;


    // The numbers that follow were picked by experimentation to try to get
    // an effect linearly proportional to <strength> and at the same time,
    // be fast enough.
    //
    // I still have no idea what "radius" and "sigma" mean.

    const double RadiusMin = .1;
    const double RadiusMax = 2.5;
    const double radius = RadiusMin +
       (strength - 1) *
       (RadiusMax - RadiusMin) /
       (kpEffectBlurSharpen::MaxStrength - 1);

    const double SigmaMin = .5;
    const double SigmaMax = 3.0;
    const double sigma = SigmaMin +
        (strength - 1) *
        (SigmaMax - SigmaMin) /
        (kpEffectBlurSharpen::MaxStrength - 1);

    const double RepeatMin = 1;
    const double RepeatMax = 2;
    const double repeat = qRound (RepeatMin +
        (strength - 1) *
        (RepeatMax - RepeatMin) /
        (kpEffectBlurSharpen::MaxStrength - 1));

// I guess these values are more proper as they use an auto-calculated
// radius but they cause sharpen() to be too slow.
#if 0
    const double radius = 0/*auto-calculate*/;

    const double SigmaMin = .6;
    const double SigmaMax = 1.0;
    const double sigma = SigmaMin +
        (strength - 1) *
        (SigmaMax - SigmaMin) /
        (kpEffectBlurSharpen::MaxStrength - 1);

    const double RepeatMin = 1;
    const double RepeatMax = 3;
    const double repeat = qRound (RepeatMin +
        (strength - 1) *
        (RepeatMax - RepeatMin) /
        (kpEffectBlurSharpen::MaxStrength - 1));
#endif

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    kDebug () << "kpEffectBlurSharpen.cpp:SharpenQImage(strength=" << strength << ")"
               << " radius=" << radius
               << " sigma=" << sigma
               << " repeat=" << repeat
               << endl;
#endif


    for (int i = 0; i < repeat; i++)
    {
    #if DEBUG_KP_EFFECT_BLUR_SHARPEN
	QTime timer; timer.start ();
    #endif
        qimage = Blitz::gaussianSharpen (qimage, radius, sigma);
    #if DEBUG_KP_EFFECT_BLUR_SHARPEN
        kDebug () << "\titeration #" + QString::number (i)
                  << ": " + QString::number (timer.elapsed ()) << "ms" << endl;
    #endif
    }


    return qimage;
}

//---------------------------------------------------------------------

// public static
kpImage kpEffectBlurSharpen::applyEffect (const kpImage &image,
        Type type, int strength)
{
#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    kDebug () << "kpEffectBlurSharpen::applyEffect(image.rect=" << image.rect ()
              << ",type=" << int (type)
              << ",strength=" << strength
              << ")" << endl;
#endif

    Q_ASSERT (strength >= MinStrength && strength <= MaxStrength);

    if (type == Blur)
        return ::BlurQImage (image, strength);
    else if (type == Sharpen)
        return ::SharpenQImage (image, strength);
    else
      return kpImage();
}

//---------------------------------------------------------------------
