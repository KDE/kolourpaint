
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


#ifndef KP_EFFECT_SWIRL_H
#define KP_EFFECT_SWIRL_H


#include <kpcolor.h>

#include <kpcoloreffect.h>


class KIntNumInput;

class kpMainWindow;


class kpEffectSwirlCommand : public kpColorEffectCommand
{
public:
    kpEffectSwirlCommand (int swirlDegrees,
                          const kpColor &backgroundColor,
                          bool actOnSelection,
                          kpMainWindow *mainWindow);
    virtual ~kpEffectSwirlCommand ();

    static QPixmap applyColorEffect (const QPixmap &pixmap,
                                     int swirlDegrees,
                                     const kpColor &backgroundColor);

protected:
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

protected:
    int m_swirlDegrees;
    kpColor m_backgroundColor;
};


class kpEffectSwirlWidget : public kpColorEffectWidget
{
public:
    kpEffectSwirlWidget (bool actOnSelection,
                         kpMainWindow *mainWindow,
                         QWidget *parent, const char *name = 0);
    virtual ~kpEffectSwirlWidget ();

    virtual bool isNoOp () const;
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

    virtual kpColorEffectCommand *createCommand () const;

protected:
    KIntNumInput *m_swirlDegreesInput;
};


#endif  // KP_EFFECT_SWIRL_H
