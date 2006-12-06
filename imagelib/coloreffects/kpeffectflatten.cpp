
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


#define DEBUG_KP_EFFECT_FLATTEN 0


#include <kpeffectflatten.h>

#include <qcheckbox.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <kvbox.h>

#include <kpdefs.h>
#include <kppixmapfx.h>


kpEffectFlattenCommand::kpEffectFlattenCommand (const QColor &color1,
                                                const QColor &color2,
                                                bool actOnSelection,
                                                kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Flatten"), actOnSelection, mainWindow),
      m_color1 (color1), m_color2 (color2)
{
}

kpEffectFlattenCommand::~kpEffectFlattenCommand ()
{
}


// public static
void kpEffectFlattenCommand::apply (QPixmap *destPixmapPtr,
                                    const QColor &color1, const QColor &color2)
{
    if (!destPixmapPtr)
        return;

    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    apply (&image, color1, color2);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);
}

// public static
QPixmap kpEffectFlattenCommand::apply (const QPixmap &pm,
                                       const QColor &color1, const QColor &color2)
{
    QImage image = kpPixmapFX::convertToImage (pm);
    apply (&image, color1, color2);
    return kpPixmapFX::convertToPixmap (image);
}

// public static
void kpEffectFlattenCommand::apply (QImage *destImagePtr,
                                    const QColor &color1, const QColor &color2)
{
    if (!destImagePtr)
        return;

    KImageEffect::flatten (*destImagePtr/*ref*/, color1, color2);
}

// public static
QImage kpEffectFlattenCommand::apply (const QImage &img,
                                      const QColor &color1, const QColor &color2)
{
    QImage retImage = img;
    apply (&retImage, color1, color2);
    return retImage;
}


//
// kpEffectFlattenCommand implements kpColorEffectCommand interface
//

// protected virtual [base kpColorEffectCommand]
kpImage kpEffectFlattenCommand::applyColorEffect (const kpImage &image)
{
    return apply (image, m_color1, m_color2);
}


#include <kpeffectflatten.moc>

