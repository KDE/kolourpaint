
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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

#include <kpeffectswirl.h>

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kimageeffect.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpmainwindow.h>
#include <kppixmapfx.h>


kpEffectSwirlCommand::kpEffectSwirlCommand (int swirlDegrees,
                                            const kpColor &backgroundColor,
                                            bool actOnSelection,
                                            kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Swirl"), actOnSelection, mainWindow),
      m_swirlDegrees (swirlDegrees),
      m_backgroundColor (backgroundColor)
{
}

kpEffectSwirlCommand::~kpEffectSwirlCommand ()
{
}


// public static
QPixmap kpEffectSwirlCommand::applyColorEffect (const QPixmap &pixmap,
                                                int swirlDegrees,
                                                const kpColor &backgroundColor)
{
    QImage image = kpPixmapFX::convertToImage (pixmap);
    image = KImageEffect::swirl (image, swirlDegrees, backgroundColor.toQRgb ());
    return kpPixmapFX::convertToPixmap (image);
}

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectSwirlCommand::applyColorEffect (const QPixmap &pixmap)
{
    return applyColorEffect (pixmap, m_swirlDegrees, m_backgroundColor);
}



kpEffectSwirlWidget::kpEffectSwirlWidget (bool actOnSelection,
                                          kpMainWindow *mainWindow,
                                          QWidget *parent, const char *name)
    : kpColorEffectWidget (actOnSelection, mainWindow, parent, name)
{
    QGridLayout *lay = new QGridLayout (this, 1, 2, marginHint (), spacingHint ());


    QLabel *label = new QLabel (i18n ("Swirl &Angle:"), this);
    m_swirlDegreesInput = new KIntNumInput (this);
    m_swirlDegreesInput->setRange (-720, 720, 10/*step*/, true/*slider*/);


    label->setBuddy (m_swirlDegreesInput);


    connect (m_swirlDegreesInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));


    lay->addWidget (label, 0, 0);
    lay->addWidget (m_swirlDegreesInput, 0, 1);

    lay->setColStretch (1, 1);
}

kpEffectSwirlWidget::~kpEffectSwirlWidget ()
{
}


// public virtual [base kpColorEffectWidget]
bool kpEffectSwirlWidget::isNoOp () const
{
    return (m_swirlDegreesInput->value () == 0);
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectSwirlWidget::applyColorEffect (const QPixmap &pixmap)
{
    return kpEffectSwirlCommand::applyColorEffect (pixmap,
                                                   m_swirlDegreesInput->value (),
                                                   m_mainWindow->backgroundColor ());
}

// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectSwirlWidget::createCommand () const
{
    return new kpEffectSwirlCommand (m_swirlDegreesInput->value (),
                                     m_mainWindow->backgroundColor (),
                                     m_actOnSelection,
                                     m_mainWindow);
}
