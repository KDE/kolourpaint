
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

#define DEBUG_KP_EFFECT_BALANCE 1


#include <kpeffectbalance.h>

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <knuminput.h>

#include <kppixmapfx.h>


kpEffectBalanceCommand::kpEffectBalanceCommand (int channels,
        int contrast, int brightness, int gamma,
        bool actOnSelection,
        kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Balance"), actOnSelection, mainWindow),
      m_channels (channels),
      m_contrast (contrast), m_brightness (brightness), m_gamma (gamma)
{
}

kpEffectBalanceCommand::~kpEffectBalanceCommand ()
{
}


static inline int between0And255 (int val)
{
    if (val < 0)
        return 0;
    else if (val > 255)
        return 255;
    else
        return val;
}


static inline int contrast (int base, int strength)
{
    return between0And255 ((base - 127) * (strength + 50) / 50 + 127);
}

static inline int brightness (int base, int strength)
{
    return between0And255 (base + strength * 255 / 50);
}

#include <math.h>
static inline int gamma (int base, int strength)
{
    return between0And255 (int (255.0 * pow (base / 255.0, 1.0 - 2.2 * strength / 50.0)));
}


static inline int contrastBrightnessGamma (int base,
                                           int newContrast,
                                           int newBrightness,
                                           int newGamma)
{
    return gamma (contrast (brightness (base, newBrightness),
                            newContrast),
                  newGamma);
}


// public static
QPixmap kpEffectBalanceCommand::applyColorEffect (const QPixmap &pixmap,
    int channels,
    int contrast, int brightness, int gamma)
{
#if DEBUG_KP_EFFECT_BALANCE
    kdDebug () << "kpEffectBalanceCommand::applyColorEffect("
               << "channels=" << channels
               << ",contrast=" << contrast
               << ",brightness=" << brightness
               << ",gamma=" << gamma
               << ")" << endl;
#endif

    QImage image = kpPixmapFX::convertToImage (pixmap);

    for (int y = 0; y < image.height (); y++)
    {
        for (int x = 0; x < image.width (); x++)
        {
            QRgb rgb = image.pixel (x, y);

            int red = qRed (rgb);
            int blue = qBlue (rgb);
            int green = qGreen (rgb);


            if (channels & Red)
                red = contrastBrightnessGamma (red, contrast, brightness, gamma);
            if (channels & Green)
                green = contrastBrightnessGamma (green, contrast, brightness, gamma);
            if (channels & Blue)
                blue = contrastBrightnessGamma (blue, contrast, brightness, gamma);


            // Gamma
            image.setPixel (x, y, qRgba (red, green, blue, qAlpha (rgb)));
        }
    }

    return kpPixmapFX::convertToPixmap (image);
}

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectBalanceCommand::applyColorEffect (const QPixmap &pixmap)
{
    return applyColorEffect (pixmap, m_channels,
                             m_contrast, m_brightness, m_gamma);
}



kpEffectBalanceWidget::kpEffectBalanceWidget (bool actOnSelection,
                                              kpMainWindow *mainWindow,
                                              QWidget *parent, const char *name)
    : kpColorEffectWidget (actOnSelection, mainWindow, parent, name)
{
    QGridLayout *lay = new QGridLayout (this, 5, 4, marginHint (), spacingHint ());


    QLabel *contrastLabel = new QLabel (i18n ("Co&ntrast:"), this);
    m_contrastInput = new KIntNumInput (0/*value*/, this);
    m_contrastInput->setRange (-50, 50, 1/*step*/, true/*slider*/);
    QPushButton *contrastResetPushButton = new QPushButton (i18n ("&Reset"), this);

    QLabel *brightnessLabel = new QLabel (i18n ("&Brightness:"), this);
    m_brightnessInput = new KIntNumInput (0/*value*/, this);
    m_brightnessInput->setRange (-50, 50, 1/*step*/, true/*slider*/);
    QPushButton *brightnessResetPushButton = new QPushButton (i18n ("Re&set"), this);

    QLabel *gammaLabel = new QLabel (i18n ("&Gamma:"), this);
    m_gammaInput = new KIntNumInput (0/*value*/, this);
    m_gammaInput->setRange (-50, 50, 1/*step*/, true/*slider*/);
    QPushButton *gammaResetPushButton = new QPushButton (i18n ("Rese&t"), this);


    QWidget *spaceWidget = new QLabel (this);
    spaceWidget->setFixedSize (1, spacingHint ());


    QLabel *channelLabel = new QLabel (i18n ("C&hannels:"), this);
    m_channelsComboBox = new KComboBox (this);
    m_channelsComboBox->insertItem (i18n ("All"));
    m_channelsComboBox->insertItem (i18n ("Red"));
    m_channelsComboBox->insertItem (i18n ("Green"));
    m_channelsComboBox->insertItem (i18n ("Blue"));


    QPushButton *resetPushButton = new QPushButton (i18n ("Reset &All Values"), this);


    contrastLabel->setBuddy (m_contrastInput);
    brightnessLabel->setBuddy (m_brightnessInput);
    gammaLabel->setBuddy (m_gammaInput);

    channelLabel->setBuddy (m_channelsComboBox);


    lay->addWidget (contrastLabel, 0, 0);
    lay->addMultiCellWidget (m_contrastInput, 0, 0, 1, 2);
    lay->addWidget (contrastResetPushButton, 0, 3);

    lay->addWidget (brightnessLabel, 1, 0);
    lay->addMultiCellWidget (m_brightnessInput, 1, 1, 1, 2);
    lay->addWidget (brightnessResetPushButton, 1, 3);

    lay->addWidget (gammaLabel, 2, 0);
    lay->addMultiCellWidget (m_gammaInput, 2, 2, 1, 2);
    lay->addWidget (gammaResetPushButton, 2, 3);

    lay->addMultiCellWidget (spaceWidget, 3, 3, 0, 3);
    lay->addMultiCellWidget (resetPushButton, 4, 4, 2, 3, Qt::AlignRight);

    lay->addWidget (channelLabel, 4, 0);
    lay->addWidget (m_channelsComboBox, 4, 1, Qt::AlignLeft);
    //lay->addWidget (resetPushButton, 4, 2, Qt::AlignRight);

    lay->setColStretch (1, 1);


    connect (m_contrastInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));
    connect (m_brightnessInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));
    connect (m_gammaInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChanged ()));

    connect (m_channelsComboBox, SIGNAL (activated (int)),
             this, SIGNAL (settingsChanged ()));

    connect (contrastResetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetContrast ()));
    connect (brightnessResetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetBrightness ()));
    connect (gammaResetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetGamma ()));

    connect (resetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetAll ()));
}

kpEffectBalanceWidget::~kpEffectBalanceWidget ()
{
}


// public virtual [base kpColorEffectWidget]
QString kpEffectBalanceWidget::caption () const
{
    return i18n ("Settings");
}


// public virtual [base kpColorEffectWidget]
bool kpEffectBalanceWidget::isNoOp () const
{
    return (contrast () == 0 && brightness () == 0 && gamma () == 0);
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectBalanceWidget::applyColorEffect (const QPixmap &pixmap)
{
    return kpEffectBalanceCommand::applyColorEffect (pixmap,
               channels (), contrast (), brightness (), gamma ());
}

// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectBalanceWidget::createCommand () const
{
    return new kpEffectBalanceCommand (channels (),
                                       contrast (), brightness (), gamma (),
                                       m_actOnSelection,
                                       m_mainWindow);
}


// protected
int kpEffectBalanceWidget::channels () const
{
    switch (m_channelsComboBox->currentItem ())
    {
    default:
    case 0:
        return kpEffectBalanceCommand::RGB;

    case 1:
        return kpEffectBalanceCommand::Red;

    case 2:
        return kpEffectBalanceCommand::Green;

    case 3:
        return kpEffectBalanceCommand::Blue;
    }
}


// protected
int kpEffectBalanceWidget::contrast () const
{
    return m_contrastInput->value ();
}

// protected
int kpEffectBalanceWidget::brightness () const
{
    return m_brightnessInput->value ();
}

// protected
int kpEffectBalanceWidget::gamma () const
{
    return m_gammaInput->value ();
}


// protected slot
void kpEffectBalanceWidget::resetContrast ()
{
    m_contrastInput->setValue (0);
}

// protected slot
void kpEffectBalanceWidget::resetBrightness ()
{
    m_brightnessInput->setValue (0);
}

// protected slot
void kpEffectBalanceWidget::resetGamma ()
{
    m_gammaInput->setValue (0);
}


// protected slot
void kpEffectBalanceWidget::resetAll ()
{
    // Prevent multiple settingsChanged() which would normally result in
    // redundant, expensive preview repaints
    blockSignals (true);

    resetContrast ();
    resetBrightness ();
    resetGamma ();

    blockSignals (false);

    emit settingsChanged ();
}


#include <kpeffectbalance.moc>
