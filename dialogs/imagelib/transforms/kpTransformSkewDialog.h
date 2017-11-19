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


#ifndef kpTransformSkewDialog_H
#define kpTransformSkewDialog_H

#include "kpTransformPreviewDialog.h"
#include "imagelib/kpColor.h"

class QSpinBox;


class kpTransformSkewDialog : public kpTransformPreviewDialog
{
Q_OBJECT

public:
    kpTransformSkewDialog (bool actOnSelection,
        kpTransformDialogEnvironment *_env, QWidget *parent);
    ~kpTransformSkewDialog () override;

private:
    static int s_lastWidth, s_lastHeight;
    static int s_lastHorizontalAngle, s_lastVerticalAngle;

    void createAngleGroupBox ();

    QSize newDimensions () const override;
    QImage transformPixmap (const QImage &image,
                                    int targetWidth, int targetHeight) const override;

    void updateLastAngles ();

private slots:
    void slotUpdate () override;

public:
    // These are the angles the users sees in the dialog and...
    int horizontalAngle () const;
    int verticalAngle () const;

    // ...these functions translate them for use in kpPixmapFX::skew().
    static int horizontalAngleForPixmapFX (int hangle);
    static int verticalAngleForPixmapFX (int vangle);

    int horizontalAngleForPixmapFX () const;
    int verticalAngleForPixmapFX () const;

    bool isNoOp () const override;

private slots:
    void accept () override;

private:
    QSpinBox *m_horizontalSkewInput, *m_verticalSkewInput;
};


#endif  // kpTransformSkewDialog_H
