
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


#ifndef KP_EFFECT_FLATTEN_H
#define KP_EFFECT_FLATTEN_H


#include <qcolor.h>

#include <kpcoloreffect.h>


class QCheckBox;
class QImage;

class KColorButton;

class kpMainWindow;


class kpEffectFlattenCommand : public kpColorEffectCommand
{
public:
    kpEffectFlattenCommand (const QColor &color1, const QColor &color2,
                            bool actOnSelection,
                            kpMainWindow *mainWindow);
    virtual ~kpEffectFlattenCommand ();


    static void apply (QPixmap *destPixmapPtr,
                       const QColor &color1, const QColor &color2);
    static QPixmap apply (const QPixmap &pm,
                          const QColor &color1, const QColor &color2);
    static void apply (QImage *destImagePtr,
                       const QColor &color1, const QColor &color2);
    static QImage apply (const QImage &img,
                         const QColor &color1, const QColor &color2);


    //
    // kpColorEffectCommand interface
    //

protected:
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

    QColor m_color1, m_color2;
};


class kpEffectFlattenWidget : public kpColorEffectWidget
{
Q_OBJECT

public:
    kpEffectFlattenWidget (bool actOnSelection,
                           kpMainWindow *mainWindow,
                           QWidget *parent, const char *name = 0);
    virtual ~kpEffectFlattenWidget ();


    static QColor s_lastColor1, s_lastColor2;


    QColor color1 () const;
    QColor color2 () const;


    //
    // kpColorEffectWidget interface
    //

    virtual QString caption () const;

    virtual bool isNoOp () const;
    virtual QPixmap applyColorEffect (const QPixmap &pixmap);

    virtual kpColorEffectCommand *createCommand () const;

protected slots:
    void slotEnableChanged (bool enable);

protected:
    QCheckBox *m_enableCheckBox;
    KColorButton *m_color1Button, *m_color2Button;
};



#endif  // KP_EFFECT_FLATTEN_H
