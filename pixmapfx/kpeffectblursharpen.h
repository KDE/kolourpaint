
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


#ifndef KP_EFFECT_BLUR_SHARPEN_H
#define KP_EFFECT_BLUR_SHARPEN_H


#include <kpcolor.h>

#include <kpcoloreffect.h>


class QLabel;

class KIntNumInput;

class kpMainWindow;


class kpEffectBlurSharpenCommand : public kpColorEffectCommand
{
public:
    enum Type
    {
        None = 0, Blur, Sharpen
    };

    kpEffectBlurSharpenCommand (Type type,
                                double radius, double sigma,
                                int repeat,
                                bool actOnSelection,
                                kpMainWindow *mainWindow);
    virtual ~kpEffectBlurSharpenCommand ();

    static QPixmap apply (const QPixmap &pixmap,
                          Type type, double radius, double sigma,
                          int repeat);

protected:
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

protected:
    Type m_type;
    double m_radius, m_sigma;
    int m_repeat;
};


class kpEffectBlurSharpenWidget : public kpColorEffectWidget
{
Q_OBJECT

public:
    kpEffectBlurSharpenWidget (bool actOnSelection,
                               kpMainWindow *mainWindow,
                               QWidget *parent, const char *name = 0);
    virtual ~kpEffectBlurSharpenWidget ();

    virtual QString caption () const;

    virtual bool isNoOp () const;
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

    virtual kpColorEffectCommand *createCommand () const;

protected slots:
    void slotUpdateTypeLabel ();

protected:
    kpEffectBlurSharpenCommand::Type type () const;
    double radius () const;
    double sigma () const;
    int repeat () const;

    KIntNumInput *m_amountInput;
    QLabel *m_typeLabel;
};


#endif  // KP_EFFECT_BLUR_SHARPEN_H
