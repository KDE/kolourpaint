
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


#define DEBUG_KP_EFFECT_BLUR_SHARPEN 0


#include <kpeffectblursharpen.h>

#include <qbitmap.h>
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

#include <kpmainwindow.h>
#include <kppixmapfx.h>


kpEffectBlurSharpenCommand::kpEffectBlurSharpenCommand (Type type,
                                                        double radius, double sigma,
                                                        int repeat,
                                                        bool actOnSelection,
                                                        kpMainWindow *mainWindow)
    : kpColorEffectCommand (kpEffectBlurSharpenCommand::nameForType (type),
                            actOnSelection, mainWindow),
      m_type (type),
      m_radius (radius), m_sigma (sigma),
      m_repeat (repeat)
{
}

kpEffectBlurSharpenCommand::~kpEffectBlurSharpenCommand ()
{
}


// public static
QString kpEffectBlurSharpenCommand::nameForType (kpEffectBlurSharpenCommand::Type type)
{
    if (type == kpEffectBlurSharpenCommand::Blur)
        return i18n ("Soften");
    else if (type == kpEffectBlurSharpenCommand::Sharpen)
        return i18n ("Sharpen");
    else
        return QString::null;
}


// public static
QPixmap kpEffectBlurSharpenCommand::apply (const QPixmap &pixmap,
                                           Type type, double radius, double sigma,
                                           int repeat)
{
#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    kDebug () << "kpEffectBlurSharpenCommand::apply(type="
               << int (type)
               << " radius=" << radius
               << " sigma=" << sigma
               << " repeat=" << repeat
               << ")"
               << endl;
#endif

    // (KImageEffect::(blur|sharpen)() ignores mask)
    QPixmap usePixmap = kpPixmapFX::pixmapWithDefinedTransparentPixels (
        pixmap,
        Qt::white/*arbitrarily chosen*/);


    QImage image = kpPixmapFX::convertToImage (usePixmap);

    for (int i = 0; i < repeat; i++)
    {
        if (type == Blur)
            image = KImageEffect::blur (image, radius, sigma);
        else if (type == Sharpen)
            image = KImageEffect::sharpen (image, radius, sigma);
    }

    QPixmap retPixmap = kpPixmapFX::convertToPixmap (image);


    // KImageEffect::(blur|sharpen)() nukes mask - restore it
    if (!usePixmap.mask ().isNull())
        retPixmap.setMask (usePixmap.mask ());


    return retPixmap;
}

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectBlurSharpenCommand::applyColorEffect (const QPixmap &pixmap)
{
    return apply (pixmap, m_type, m_radius, m_sigma, m_repeat);
}


#include <kpeffectblursharpen.moc>
