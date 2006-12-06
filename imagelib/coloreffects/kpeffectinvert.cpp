
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#include <kpeffectinvert.h>

#include <qcheckbox.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kppixmapfx.h>


kpEffectInvertCommand::kpEffectInvertCommand (int channels,
                                              bool actOnSelection,
                                              kpMainWindow *mainWindow)
    : kpColorEffectCommand (channels == RGB ?
                                i18n ("Invert Colors") : i18n ("Invert"),
                            actOnSelection, mainWindow),
      m_channels (channels)
{
}

kpEffectInvertCommand::kpEffectInvertCommand (bool actOnSelection,
                                              kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Invert Colors"), actOnSelection, mainWindow),
      m_channels (RGB)
{
}

kpEffectInvertCommand::~kpEffectInvertCommand ()
{
}


// public static
void kpEffectInvertCommand::apply (QPixmap *destPixmapPtr, int channels)
{
    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    apply (&image, channels);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);
}

// public static
QPixmap kpEffectInvertCommand::apply (const QPixmap &pm, int channels)
{
    QImage image = kpPixmapFX::convertToImage (pm);
    apply (&image, channels);
    return kpPixmapFX::convertToPixmap (image);
}

// public static
void kpEffectInvertCommand::apply (QImage *destImagePtr, int channels)
{
    QRgb mask = qRgba ((channels & Red) ? 0xFF : 0,
                       (channels & Green) ? 0xFF : 0,
                       (channels & Blue) ? 0xFF : 0,
                       0/*don't invert alpha*/);
#if DEBUG_KP_EFFECT_INVERT
    kDebug () << "kpEffectInvertCommand::apply(channels=" << channels
               << ") mask=" << (int *) mask
               << endl;
#endif

    if (destImagePtr->depth () > 8)
    {
    #if 0
        // SYNC: TODO: Qt BUG - invertAlpha argument is inverted!!!
        destImagePtr->invertPixels (true/*no invert alpha (Qt 3.2)*/);
    #else
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
    #endif
    }
    else
    {
        for (int i = 0; i < destImagePtr->numColors (); i++)
        {
            destImagePtr->setColor (i, destImagePtr->color (i) ^ mask);
        }
    }
}

// public static
QImage kpEffectInvertCommand::apply (const QImage &img, int channels)
{
    QImage retImage = img;
    apply (&retImage, channels);
    return retImage;
}


//
// kpEffectInvertCommand implements kpColorEffectCommand interface
//

// protected virtual [base kpColorEffectCommand]
kpImage kpEffectInvertCommand::applyColorEffect (const kpImage &image)
{
    return apply (image, m_channels);
}


#include <kpeffectinvert.moc>

