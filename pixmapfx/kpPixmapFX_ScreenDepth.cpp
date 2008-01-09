
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


#define DEBUG_KP_PIXMAP_FX 0


#include <kpPixmapFX.h>

#include <QPixmap>

#include <KLocale>
#include <KMessageBox>
#include <KDebug>


// public static
// (KApplication has not been constructed yet)
void kpPixmapFX::initScreenDepthPre ()
{
}

// public static
void kpPixmapFX::initScreenDepthPost ()
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::initScreenDepthPost() defaultDepth="
              << QPixmap::defaultDepth ();
#endif

    if (kpPixmapFX::screenIsPaletted ())
    {
        // Even though we support 15-bit truecolor, we're more ambitious and
        // ask for 24-bit since it's safer (see kpPixmapFX::WarnAboutLossInfo).
        KMessageBox::information (0/*parent*/,
            ki18n ("<qt><p>KolourPaint does not support the current screen depth of %1bpp."
                " KolourPaint will attempt to start but may act unreliably.</p>"

                "<p>To avoid this issue, please change your screen depth to 24bpp"
                " and then restart KolourPaint.</p></qt>")
                .subs (QPixmap::defaultDepth ()).toString (),
            i18n ("Unsupported Screen Mode"),
            "startup_unsupported_bpp"/*DontAskAgain ID*/);
    }
}


// public static
bool kpPixmapFX::screenIsPaletted ()
{
    return (QPixmap::defaultDepth () < 15/*smallest truecolor mode's bpp*/);
}

// public static
QString kpPixmapFX::effectDoesNotSupportPalettedScreenMessage ()
{
    Q_ASSERT (kpPixmapFX::screenIsPaletted ());

    // Even though we support 15-bit truecolor, we're more ambitious and
    // ask for 24-bit since it's safer (see kpPixmapFX::WarnAboutLossInfo).
    return ki18n ("<qt><p>This effect does not support the current screen depth of %1bpp.</p>"
                  "<p>To avoid this issue, please change your screen depth to 24bpp"
                  " and then restart KolourPaint.</p></qt>")
               .subs (QPixmap::defaultDepth ()).toString ();

}
