
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

#include <kpeffectcontrast.h>

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kimageeffect.h>
#include <klocale.h>
#include <knuminput.h>

#include <kppixmapfx.h>


kpEffectContrastCommand::kpEffectContrastCommand (int contrastValue,
                                                  bool actOnSelection,
                                                  kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Contrast"), actOnSelection, mainWindow),
      m_contrastValue (contrastValue)
{
}

kpEffectContrastCommand::~kpEffectContrastCommand ()
{
}


// public static
QPixmap kpEffectContrastCommand::applyColorEffect (const QPixmap &pixmap,
                                                   int contrastValue)
{
    QImage image = kpPixmapFX::convertToImage (pixmap);
    KImageEffect::contrast (/*ref*/image, contrastValue);
    return kpPixmapFX::convertToPixmap (image);
}

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectContrastCommand::applyColorEffect (const QPixmap &pixmap)
{
    return applyColorEffect (pixmap, m_contrastValue);
}



kpEffectContrastWidget::kpEffectContrastWidget (QWidget *parent, const char *name)
    : kpColorEffectWidget (parent, name)
{
    QGridLayout *lay = new QGridLayout (this, 1, 2, marginHint (), spacingHint ());


    QLabel *label = new QLabel (i18n ("Co&ntrast:"), this);
    m_contrastValueInput = new KIntNumInput (0/*value*/, this);
    m_contrastValueInput->setRange (-255, 255, 10/*step*/, true/*slider*/);


    label->setBuddy (m_contrastValueInput);


    connect (m_contrastValueInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));


    lay->addWidget (label, 0, 0);
    lay->addWidget (m_contrastValueInput, 0, 1);
}

kpEffectContrastWidget::~kpEffectContrastWidget ()
{
}


// public virtual [base kpColorEffectWidget]
bool kpEffectContrastWidget::isNoOp () const
{
    return (m_contrastValueInput->value () == 0);
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectContrastWidget::applyColorEffect (const QPixmap &pixmap)
{
    return kpEffectContrastCommand::applyColorEffect (pixmap, m_contrastValueInput->value ());
}

// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectContrastWidget::createCommand (bool actOnSelection,
                                                             kpMainWindow *mainWindow) const
{
    return new kpEffectContrastCommand (m_contrastValueInput->value (),
                                        actOnSelection,
                                        mainWindow);
}
