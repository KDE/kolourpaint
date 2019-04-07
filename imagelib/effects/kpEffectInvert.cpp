
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


#define DEBUG_KP_EFFECT_INVERT 0


#include "kpEffectInvert.h"

#include <QImage>

#include "kpLogCategories.h"

#include "pixmapfx/kpPixmapFX.h"


// public static
void kpEffectInvert::applyEffect (QImage *destImagePtr, int channels)
{
    if (channels == kpEffectInvert::RGB)
    {
        destImagePtr->invertPixels ();
        return;
    }

    QRgb mask = qRgba ((channels & Red) ? 0xFF : 0,
                       (channels & Green) ? 0xFF : 0,
                       (channels & Blue) ? 0xFF : 0,
                       0/*don't invert alpha*/);
#if DEBUG_KP_EFFECT_INVERT
    qCDebug(kpLogImagelib) << "kpEffectInvert::applyEffect(channels=" << channels
               << ") mask=" << (int *) mask;
#endif

    if (destImagePtr->depth () > 8)
    {
        // Above version works for Qt 3.2 at least.
        // But this version will always work (slower, though) and supports
        // inverting particular channels.
        for (int y = 0; y < destImagePtr->height (); y++)
        {
            for (int x = 0; x < destImagePtr->width (); x++)
            {
                destImagePtr->setPixel (x, y, destImagePtr->pixel (x, y) ^ mask);
            }
        }
    }
    else
    {
        for (int i = 0; i < destImagePtr->colorCount (); i++)
        {
            destImagePtr->setColor (i, destImagePtr->color (i) ^ mask);
        }
    }
}

// public static
QImage kpEffectInvert::applyEffect (const QImage &img, int channels)
{
    QImage retImage = img;
    applyEffect (&retImage, channels);
    return retImage;
}


