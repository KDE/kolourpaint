
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kpeffectemboss.h>

#include <qcheckbox.h>
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


kpEffectEmbossCommand::kpEffectEmbossCommand (double radius, double sigma,
                                              int repeat,
                                              bool actOnSelection,
                                              kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Emboss"), actOnSelection, mainWindow),
      m_radius (radius), m_sigma (sigma),
      m_repeat (repeat)
{
}

kpEffectEmbossCommand::~kpEffectEmbossCommand ()
{
}


// public static
QPixmap kpEffectEmbossCommand::apply (const QPixmap &pixmap,
                                      double radius, double sigma,
                                      int repeat)
{
#if DEBUG_KP_EFFECT_EMBOSS
    kdDebug () << "kpEffectEmbossCommand::apply()"
               << " radius=" << radius
               << " sigma=" << sigma
               << " repeat=" << repeat
               << ")"
               << endl;
#endif

    // (KImageEffect::emboss() ignores mask)
    QPixmap usePixmap = kpPixmapFX::pixmapWithDefinedTransparentPixels (
        pixmap,
        Qt::white/*arbitrarily chosen*/);


    QImage image = kpPixmapFX::convertToImage (usePixmap);

    for (int i = 0; i < repeat; i++)
    {
        image = KImageEffect::emboss (image, radius, sigma);
    }

    QPixmap retPixmap = kpPixmapFX::convertToPixmap (image);


    // KImageEffect::emboss() nukes mask - restore it
    if (usePixmap.mask ())
        retPixmap.setMask (*usePixmap.mask ());


    return retPixmap;
}

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectEmbossCommand::applyColorEffect (const QPixmap &pixmap)
{
    return apply (pixmap, m_radius, m_sigma, m_repeat);
}



kpEffectEmbossWidget::kpEffectEmbossWidget (bool actOnSelection,
                                            kpMainWindow *mainWindow,
                                            QWidget *parent, const char *name)
    : kpColorEffectWidget (actOnSelection, mainWindow, parent, name)
{
    QGridLayout *lay = new QGridLayout (this, 4, 2, marginHint (), spacingHint ());


#if 0
    QLabel *amountLabel = new QLabel (i18n ("&Amount:"), this);
    m_amountInput = new KIntNumInput (this);
    m_amountInput->setRange (0, 10, 1/*step*/, true/*slider*/);
    m_amountInput->setSpecialValueText (i18n ("None"));


    amountLabel->setBuddy (m_amountInput);


    lay->addWidget (amountLabel, 0, 0);
    lay->addWidget (m_amountInput, 0, 1);

    lay->setColStretch (1, 1);


    connect (m_amountInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));
#endif

    m_enableCheckBox = new QCheckBox (i18n ("E&nable"), this);


    lay->addMultiCellWidget (m_enableCheckBox, 0, 0, 0, 1, Qt::AlignCenter);


    // (settingsChangedDelayed() instead of settingsChanged() so that the
    //  user can quickly press OK to apply effect to document directly and
    //  not have to wait for the also slow preview)
    connect (m_enableCheckBox, SIGNAL (toggled (bool)),
             this, SIGNAL (settingsChangedDelayed ()));
}

kpEffectEmbossWidget::~kpEffectEmbossWidget ()
{
}


// public virtual [base kpColorEffectWidget]
QString kpEffectEmbossWidget::caption () const
{
    return QString::null;
}


// public virtual [base kpColorEffectWidget]
bool kpEffectEmbossWidget::isNoOp () const
{
    //return (m_amountInput->value () == 0);
    return !m_enableCheckBox->isChecked ();
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectEmbossWidget::applyColorEffect (const QPixmap &pixmap)
{
    if (isNoOp ())
        return pixmap;

    return kpEffectEmbossCommand::apply (pixmap, radius (), sigma (), repeat ());
}

// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectEmbossWidget::createCommand () const
{
    return new kpEffectEmbossCommand (radius (), sigma (), repeat (),
                                      m_actOnSelection,
                                      m_mainWindow);
}


// The numbers that follow were picked by experimentation.
// I still have no idea what "radius" and "sigma" mean
// (even after reading the API).

// protected
double kpEffectEmbossWidget::radius () const
{
    //if (m_amountInput->value () == 0)
    //    return 0;

    return 0;
}


// protected
double kpEffectEmbossWidget::sigma () const
{
#if 0
    if (m_amountInput->value () == 0)
        return 0;

    const double Min = 1;
    const double Max = 1.2;

    return Min +
               (m_amountInput->maxValue () - m_amountInput->value ()) *
                    (Max - Min) /
                        (m_amountInput->maxValue () - 1);
#endif

    return 1;
}

// protected
int kpEffectEmbossWidget::repeat () const
{
    return 1;
}


#include <kpeffectemboss.moc>
