
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


#include <kpEffectBlurSharpenWidget.h>

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


kpEffectBlurSharpenWidget::kpEffectBlurSharpenWidget (bool actOnSelection,
                                                      kpMainWindow *mainWindow,
                                                      QWidget *parent)
    : kpEffectWidgetBase (actOnSelection, mainWindow, parent)
{
    QGridLayout *lay = new QGridLayout (this);
    lay->setSpacing (spacingHint ());
    lay->setMargin (marginHint ());


    QLabel *amountLabel = new QLabel (i18n ("&Amount:"), this);
    m_amountInput = new KIntNumInput (this);
    m_amountInput->setRange (-10, 10, 1/*step*/, true/*slider*/);

    m_typeLabel = new QLabel (this);


    amountLabel->setBuddy (m_amountInput);


    lay->addWidget (amountLabel, 0, 0);
    lay->addWidget (m_amountInput, 0, 1);

    lay->addWidget (m_typeLabel, 1, 0, 1, 2, Qt::AlignCenter);

    lay->setColumnStretch (1, 1);


    connect (m_amountInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChangedDelayed ()));

    connect (m_amountInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotUpdateTypeLabel ()));
}

kpEffectBlurSharpenWidget::~kpEffectBlurSharpenWidget ()
{
}


// public virtual [base kpEffectWidgetBase]
QString kpEffectBlurSharpenWidget::caption () const
{
    return QString::null;
}


// public virtual [base kpEffectWidgetBase]
bool kpEffectBlurSharpenWidget::isNoOp () const
{
    return (type () == kpEffectBlurSharpen::None);
}

// public virtual [base kpEffectWidgetBase]
kpImage kpEffectBlurSharpenWidget::applyEffect (const kpImage &image)
{
    return kpEffectBlurSharpen::applyEffect (image,
        type (), radius (), sigma (), repeat ());
}

// public virtual [base kpEffectWidgetBase]
kpEffectCommandBase *kpEffectBlurSharpenWidget::createCommand () const
{
    return new kpEffectBlurSharpenCommand (type (), radius (), sigma (), repeat (),
                                           m_actOnSelection,
                                           m_mainWindow);
}


// protected slot
void kpEffectBlurSharpenWidget::slotUpdateTypeLabel ()
{
    QString text = kpEffectBlurSharpenCommand::nameForType (type ());

#if DEBUG_KP_EFFECT_BLUR_SHARPEN
    kDebug () << "kpEffectBlurSharpenWidget::slotUpdateTypeLabel() text="
               << text << endl;
#endif
    m_typeLabel->setText (text);
}


// protected
kpEffectBlurSharpen::Type kpEffectBlurSharpenWidget::type () const
{
    if (m_amountInput->value () == 0)
        return kpEffectBlurSharpen::None;
    else if (m_amountInput->value () < 0)
        return kpEffectBlurSharpen::Blur;
    else
        return kpEffectBlurSharpen::Sharpen;
}

// The numbers that follow were picked by experimentation.
// I still have no idea what "radius" and "sigma" mean
// (even after reading the API).

// protected
double kpEffectBlurSharpenWidget::radius () const
{
    if (m_amountInput->value () == 0)
        return 0;

    if (m_amountInput->value () < 0)
    {
        return 8;
    }
    else
    {
        const double SharpenMin = .1;
        const double SharpenMax = 2.5;

        return SharpenMin +
                  (m_amountInput->value () - 1) *
                      (SharpenMax - SharpenMin) /
                          (m_amountInput->maximum () - 1);
    }
}

// protected
double kpEffectBlurSharpenWidget::sigma () const
{
    if (m_amountInput->value () == 0)
        return 0;

    if (m_amountInput->value () < 0)
    {
        const double BlurMin = .5;
        const double BlurMax = 4;

        return BlurMin +
                   (-m_amountInput->value () - 1) *
                        (BlurMax - BlurMin) /
                            (-m_amountInput->minimum () - 1);
    }
    else
    {
        const double SharpenMin = .5;
        const double SharpenMax = 3.0;

        return SharpenMin +
                   (m_amountInput->value () - 1) *
                       (SharpenMax - SharpenMin) /
                           (m_amountInput->maximum () - 1);
    }
}

// protected
int kpEffectBlurSharpenWidget::repeat () const
{
    if (m_amountInput->value () == 0)
        return 0;

    if (m_amountInput->value () < 0)
        return 1;
    else
    {
        const double SharpenMin = 1;
        const double SharpenMax = 2;

        return qRound (SharpenMin +
                          (m_amountInput->value () - 1) *
                              (SharpenMax - SharpenMin) /
                                  (m_amountInput->maximum () - 1));
    }
}


#include <kpEffectBlurSharpenWidget.moc>
