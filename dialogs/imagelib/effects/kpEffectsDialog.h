
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#ifndef KP_EFFECTS_DIALOG_H
#define KP_EFFECTS_DIALOG_H


#include "dialogs/imagelib/transforms/kpTransformPreviewDialog.h"


class QComboBox;
class QGroupBox;
class QImage;
class QTimer;
class QVBoxLayout;

class kpEffectCommandBase;
class kpEffectWidgetBase;


class kpEffectsDialog : public kpTransformPreviewDialog
{
Q_OBJECT

public:
    // Specifying <defaultSelectedEffect> is more efficient than leaving it
    // as 0 and then calling selectEffect() afterwards.
    kpEffectsDialog (bool actOnSelection,
                     kpTransformDialogEnvironment *_env,
                     QWidget *parent,
                     int defaultSelectedEffect = 0);
    ~kpEffectsDialog () override;

    bool isNoOp () const override;
    kpEffectCommandBase *createCommand () const;

protected:
    QSize newDimensions () const override;
    QImage transformPixmap (const QImage &pixmap,
                                     int targetWidth, int targetHeight) const override;

public:
    int selectedEffect () const;
public slots:
    void selectEffect (int which);

protected slots:
    void slotUpdate () override;
    void slotUpdateWithWaitCursor () override;

    void slotDelayedUpdate ();

protected:
    static int s_lastWidth, s_lastHeight;

    QTimer *m_delayedUpdateTimer;

    QComboBox *m_effectsComboBox;
    QGroupBox *m_settingsGroupBox;
    QVBoxLayout *m_settingsLayout;

    kpEffectWidgetBase *m_effectWidget;
};


#endif  // KP_EFFECTS_DIALOG_H
