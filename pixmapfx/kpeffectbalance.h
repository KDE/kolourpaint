
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


#ifndef KP_EFFECT_BALANCE_H
#define KP_EFFECT_BALANCE_H

#include <kpcoloreffect.h>


class QLabel;

class KComboBox;
class KIntNumInput;

class kpMainWindowe;


class kpEffectBalanceCommand : public kpColorEffectCommand
{
public:
    enum Channel
    {
        None = 0,
        Red = 1, Green = 2, Blue = 4,
        RGB = Red | Green | Blue
    };

    // <brightness>, <contrast> & <gamma> are from -50 to 50

    kpEffectBalanceCommand (int channels,
                            int brightness, int contrast, int gamma,
                            bool actOnSelection,
                            kpMainWindow *mainWindow);
    virtual ~kpEffectBalanceCommand ();

    static QPixmap applyColorEffect (const QPixmap &pixmap,
                                     int channels,
                                     int brightness, int contrast, int gamma);

protected:
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

protected:
    int m_channels;
    int m_brightness, m_contrast, m_gamma;
};


class kpEffectBalanceWidget : public kpColorEffectWidget
{
Q_OBJECT

public:
    kpEffectBalanceWidget (bool actOnSelection,
                           kpMainWindow *mainWindow,
                           QWidget *parent, const char *name = 0);
    virtual ~kpEffectBalanceWidget ();

    virtual QString caption () const;

    virtual bool isNoOp () const;
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

    virtual kpColorEffectCommand *createCommand () const;

protected:
    int channels () const;

    int brightness () const;
    int contrast () const;
    int gamma () const;

protected slots:
    void recalculateGammaLabel ();

    void resetBrightness ();
    void resetContrast ();
    void resetGamma ();

    void resetAll ();

protected:
    KIntNumInput *m_brightnessInput,
                 *m_contrastInput,
                 *m_gammaInput;
    QLabel *m_gammaLabel;
    KComboBox *m_channelsComboBox;
};


#endif  // KP_EFFECT_BALANCE_H
