
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

#define DEBUG_KP_EFFECT_BALANCE 0


#include <kpeffectbalance.h>

#include <math.h>

#include <qfontmetrics.h>
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


#if DEBUG_KP_EFFECT_BALANCE
    #include <qdatetime.h>
#endif


kpEffectBalanceCommand::kpEffectBalanceCommand (int channels,
        int brightness, int contrast, int gamma,
        bool actOnSelection,
        kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Balance"), actOnSelection, mainWindow),
      m_channels (channels),
      m_brightness (brightness), m_contrast (contrast), m_gamma (gamma)
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


static inline int brightness (int base, int strength)
{
    return between0And255 (base + strength * 255 / 50);
}

static inline int contrast (int base, int strength)
{
    return between0And255 ((base - 127) * (strength + 50) / 50 + 127);
}

static inline int gamma (int base, int strength)
{
    return between0And255 (qRound (255.0 * pow (base / 255.0, 1.0 / pow (10, strength / 50.0))));
}


static inline int brightnessContrastGamma (int base,
                                           int newBrightness,
                                           int newContrast,
                                           int newGamma)
{
    return gamma (contrast (brightness (base, newBrightness),
                            newContrast),
                  newGamma);
}

static inline QRgb brightnessContrastGammaForRGB (QRgb rgb,
    int channels,
    int brightness, int contrast, int gamma)
{
    int red = qRed (rgb);
    int green = qGreen (rgb);
    int blue = qBlue (rgb);


    if (channels & kpEffectBalanceCommand::Red)
        red = brightnessContrastGamma (red, brightness, contrast, gamma);
    if (channels & kpEffectBalanceCommand::Green)
        green = brightnessContrastGamma (green, brightness, contrast, gamma);
    if (channels & kpEffectBalanceCommand::Blue)
        blue = brightnessContrastGamma (blue, brightness, contrast, gamma);


    return qRgba (red, green, blue, qAlpha (rgb));
}


// public static
QPixmap kpEffectBalanceCommand::applyColorEffect (const QPixmap &pixmap,
    int channels,
    int brightness, int contrast, int gamma)
{
#if DEBUG_KP_EFFECT_BALANCE
    kdDebug () << "kpEffectBalanceCommand::applyColorEffect("
               << "channels=" << channels
               << ",brightness=" << brightness
               << ",contrast=" << contrast
               << ",gamma=" << gamma
               << ")" << endl;
    QTime timer; timer.start ();
#endif

    QImage image = kpPixmapFX::convertToImage (pixmap);
#if DEBUG_KP_EFFECT_BALANCE
    kdDebug () << "\tconvertToImage=" << timer.restart () << endl;
#endif


    Q_UINT8 transformRed [256],
            transformGreen [256],
            transformBlue [256];

    for (int i = 0; i < 256; i++)
    {
        Q_UINT8 applied = (Q_UINT8) brightnessContrastGamma (i, brightness, contrast, gamma);

        if (channels & kpEffectBalanceCommand::Red)
            transformRed [i] = applied;
        else
            transformRed [i] = i;

        if (channels & kpEffectBalanceCommand::Green)
            transformGreen [i] = applied;
        else
            transformGreen [i] = i;

        if (channels & kpEffectBalanceCommand::Blue)
            transformBlue [i] = applied;
        else
            transformBlue [i] = i;
    }

#if DEBUG_KP_EFFECT_BALANCE
    kdDebug () << "\tbuild lookup=" << timer.restart () << endl;
#endif


    if (image.depth () > 8)
    {
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                const QRgb rgb = image.pixel (x, y);

                const Q_UINT8 red = (Q_UINT8) qRed (rgb);
                const Q_UINT8 green = (Q_UINT8) qGreen (rgb);
                const Q_UINT8 blue = (Q_UINT8) qBlue (rgb);
                const Q_UINT8 alpha = (Q_UINT8) qAlpha (rgb);

                image.setPixel (x, y,
                    qRgba (transformRed [red],
                           transformGreen [green],
                           transformBlue [blue],
                           alpha));

            #if 0
                image.setPixel (x, y,
                    brightnessContrastGammaForRGB (image.pixel (x, y),
                        channels,
                        brightness, contrast, gamma));
            #endif
            }
        }
    }
    else
    {
        for (int i = 0; i < image.numColors (); i++)
        {
            const QRgb rgb = image.color (i);

            const Q_UINT8 red = (Q_UINT8) qRed (rgb);
            const Q_UINT8 green = (Q_UINT8) qGreen (rgb);
            const Q_UINT8 blue = (Q_UINT8) qBlue (rgb);
            const Q_UINT8 alpha = (Q_UINT8) qAlpha (rgb);

            image.setColor (i,
                qRgba (transformRed [red],
                       transformGreen [green],
                       transformBlue [blue],
                       alpha));

        #if 0
            image.setColor (i,
                brightnessContrastGammaForRGB (image.color (i),
                    channels,
                    brightness, contrast, gamma));
        #endif
        }

    }
#if DEBUG_KP_EFFECT_BALANCE
    kdDebug () << "\teffect=" << timer.restart () << endl;
#endif

    const QPixmap retPixmap = kpPixmapFX::convertToPixmap (image);
#if DEBUG_KP_EFFECT_BALANCE
    kdDebug () << "\tconvertToPixmap=" << timer.restart () << endl;
#endif

    return retPixmap;
}

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectBalanceCommand::applyColorEffect (const QPixmap &pixmap)
{
    return applyColorEffect (pixmap, m_channels,
                             m_brightness, m_contrast, m_gamma);
}



kpEffectBalanceWidget::kpEffectBalanceWidget (bool actOnSelection,
                                              kpMainWindow *mainWindow,
                                              QWidget *parent, const char *name)
    : kpColorEffectWidget (actOnSelection, mainWindow, parent, name)
{
    QGridLayout *lay = new QGridLayout (this, 5, 5, marginHint (), spacingHint ());


    QLabel *brightnessLabel = new QLabel (i18n ("&Brightness:"), this);
    m_brightnessInput = new KIntNumInput (0/*value*/, this);
    m_brightnessInput->setRange (-50, 50, 1/*step*/, true/*slider*/);
    QPushButton *brightnessResetPushButton = new QPushButton (i18n ("Re&set"), this);

    QLabel *contrastLabel = new QLabel (i18n ("Co&ntrast:"), this);
    m_contrastInput = new KIntNumInput (0/*value*/, this);
    m_contrastInput->setRange (-50, 50, 1/*step*/, true/*slider*/);
    QPushButton *contrastResetPushButton = new QPushButton (i18n ("&Reset"), this);

    QLabel *gammaLabel = new QLabel (i18n ("&Gamma:"), this);
    m_gammaInput = new KIntNumInput (0/*value*/, this);
    m_gammaInput->setRange (-50, 50, 1/*step*/, true/*slider*/);
    // TODO: This is what should be shown in the m_gammaInput spinbox
    m_gammaLabel = new QLabel (this);
    // TODO: This doesn't seem to be wide enough with some fonts so the
    //       whole layout moves when we drag the gamma slider.
    m_gammaLabel->setMinimumWidth (m_gammaLabel->fontMetrics ().width (" 10.00 "));
    m_gammaLabel->setAlignment (m_gammaLabel->alignment () | Qt::AlignRight);
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


    brightnessLabel->setBuddy (m_brightnessInput);
    contrastLabel->setBuddy (m_contrastInput);
    gammaLabel->setBuddy (m_gammaInput);

    channelLabel->setBuddy (m_channelsComboBox);


    lay->addWidget (brightnessLabel, 0, 0);
    lay->addMultiCellWidget (m_brightnessInput, 0, 0, 1, 2);
    lay->addWidget (brightnessResetPushButton, 0, 4);

    lay->addWidget (contrastLabel, 1, 0);
    lay->addMultiCellWidget (m_contrastInput, 1, 1, 1, 2);
    lay->addWidget (contrastResetPushButton, 1, 4);

    lay->addWidget (gammaLabel, 2, 0);
    lay->addMultiCellWidget (m_gammaInput, 2, 2, 1, 2);
    lay->addWidget (m_gammaLabel, 2, 3);
    lay->addWidget (gammaResetPushButton, 2, 4);

    lay->addMultiCellWidget (spaceWidget, 3, 3, 0, 4);
    lay->addMultiCellWidget (resetPushButton, 4, 4, 2, 4, Qt::AlignRight);

    lay->addWidget (channelLabel, 4, 0);
    lay->addWidget (m_channelsComboBox, 4, 1, Qt::AlignLeft);
    //lay->addWidget (resetPushButton, 4, 2, Qt::AlignRight);

    lay->setColStretch (1, 1);


    // (no need for settingsChangedDelayed() since BCG effect is so fast :))
    connect (m_brightnessInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChangedNoWaitCursor ()));
    connect (m_contrastInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChangedNoWaitCursor ()));

    connect (m_gammaInput, SIGNAL (valueChanged (int)),
             this, SLOT (recalculateGammaLabel ()));
    connect (m_gammaInput, SIGNAL (valueChanged (int)),
             this, SIGNAL (settingsChangedNoWaitCursor ()));

    connect (m_channelsComboBox, SIGNAL (activated (int)),
             this, SIGNAL (settingsChanged ()));

    connect (brightnessResetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetBrightness ()));
    connect (contrastResetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetContrast ()));
    connect (gammaResetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetGamma ()));

    connect (resetPushButton, SIGNAL (clicked ()),
             this, SLOT (resetAll ()));


    recalculateGammaLabel ();
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
    return (brightness () == 0 && contrast () == 0 && gamma () == 0);
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectBalanceWidget::applyColorEffect (const QPixmap &pixmap)
{
    return kpEffectBalanceCommand::applyColorEffect (pixmap,
               channels (), brightness (), contrast (), gamma ());
}

// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectBalanceWidget::createCommand () const
{
    return new kpEffectBalanceCommand (channels (),
                                       brightness (), contrast (), gamma (),
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
int kpEffectBalanceWidget::brightness () const
{
    return m_brightnessInput->value ();
}

// protected
int kpEffectBalanceWidget::contrast () const
{
    return m_contrastInput->value ();
}

// protected
int kpEffectBalanceWidget::gamma () const
{
    return m_gammaInput->value ();
}


// protected slot
void kpEffectBalanceWidget::recalculateGammaLabel ()
{
    m_gammaLabel->setText (
        " " +
        QString::number (pow (10, gamma () / 50.0),
                         'f'/*[-]9.9*/,
                         2/*precision*/) +
        " ");
    m_gammaLabel->repaint ();
}


// protected slot
void kpEffectBalanceWidget::resetBrightness ()
{
    if (brightness () == 0)
        return;

    bool sb = signalsBlocked ();

    if (!sb) blockSignals (true);
    m_brightnessInput->setValue (0);
    if (!sb) blockSignals (false);

    // Immediate update (if signals aren't blocked)
    emit settingsChanged ();
}

// protected slot
void kpEffectBalanceWidget::resetContrast ()
{
    if (contrast () == 0)
        return;

    bool sb = signalsBlocked ();

    if (!sb) blockSignals (true);
    m_contrastInput->setValue (0);
    if (!sb) blockSignals (false);

    // Immediate update (if signals aren't blocked)
    emit settingsChanged ();
}

// protected slot
void kpEffectBalanceWidget::resetGamma ()
{
    if (gamma () == 0)
        return;

    bool sb = signalsBlocked ();

    if (!sb) blockSignals (true);
    m_gammaInput->setValue (0);
    recalculateGammaLabel ();
    if (!sb) blockSignals (false);

    // Immediate update (if signals aren't blocked)
    emit settingsChanged ();
}


// protected slot
void kpEffectBalanceWidget::resetAll ()
{
    if (isNoOp ())
        return;

    // Prevent multiple settingsChanged() which would normally result in
    // redundant, expensive preview repaints
    blockSignals (true);

    resetBrightness ();
    resetContrast ();
    resetGamma ();

    recalculateGammaLabel ();

    blockSignals (false);

    emit settingsChanged ();
}


#include <kpeffectbalance.moc>
