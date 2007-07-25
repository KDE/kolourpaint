
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


#include <kpEffectEmboss.h>

#include <qbitmap.h>
#include <qcheckbox.h>
#include <qgridlayout.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpPixmapFX.h>


static QImage EmbossQImage (const QImage &qimage_, int strength)
{
    QImage qimage = qimage_;
    if (strength == 0)
        return qimage;


    // The numbers that follow were picked by experimentation.
    // I still have no idea what "radius" and "sigma" mean
    // (even after reading the API).

    const double radius = 0;

#if 0
    const double SigmaMin = 1;
    const double SigmaMax = 1.2;

    return SigmaMin +
        (kpEffectEmboss::MaxStrength - strength) *
        (SigmaMax - SigmaMin) /
        (kpEffectEmboss::MaxStrength - 1);
#endif
    const double sigma = 1;

    const int repeat = 1;


    for (int i = 0; i < repeat; i++)
    {
        qimage = KImageEffect::emboss (qimage, radius, sigma);
    }


    return qimage;
}


// public static
kpImage kpEffectEmboss::applyEffect (const kpImage &image, int strength)
{
#if DEBUG_KP_EFFECT_EMBOSS
    kDebug () << "kpEffectEmboss::applyEffect(strength=" << strength << ")"
               << endl;
#endif

    Q_ASSERT (strength >= MinStrength && strength <= MaxStrength);


    // (KImageEffect::emboss() ignores mask)
    QPixmap usePixmap = kpPixmapFX::pixmapWithDefinedTransparentPixels (
        image,
        Qt::white/*arbitrarily chosen*/);


    QImage qimage = kpPixmapFX::convertToImage (usePixmap);

    qimage = ::EmbossQImage (qimage, strength);

    QPixmap retPixmap = kpPixmapFX::convertToPixmap (qimage);


    // KImageEffect::emboss() nukes mask - restore it
    if (!usePixmap.mask ().isNull())
        retPixmap.setMask (usePixmap.mask ());


    return retPixmap;
}


#include <kpEffectEmboss.moc>
