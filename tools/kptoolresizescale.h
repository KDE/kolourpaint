
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __kptoolresizescale_h__
#define __kptoolresizescale_h__

#include <qcolor.h>
#include <qpixmap.h>
#include <kcommand.h>
#include <kdialogbase.h>

class QCheckBox;
class QString;

class KDoubleNumInput;
class KIntNumInput;

class kpDocument;
class kpMainWindow;
class kpViewManager;

class kpToolResizeScaleCommand : public KCommand
{
public:
    kpToolResizeScaleCommand (bool actOnSelection,
                              int newWidth, int newHeight,
                              bool scaleToFit,
                              kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolResizeScaleCommand ();

private:
    kpDocument *document () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    bool m_actOnSelection;
    int m_newWidth, m_newHeight;
    bool m_scaleToFit, m_isLosslessScale;
    kpMainWindow *m_mainWindow;
    QColor m_backgroundColor;

    int m_oldWidth, m_oldHeight;
    QPixmap m_oldPixmap, m_oldRightPixmap, m_oldBottomPixmap;
};

class kpToolResizeScaleDialog : public KDialogBase
{
Q_OBJECT

public:
    kpToolResizeScaleDialog (kpMainWindow *mainWindow);
    virtual ~kpToolResizeScaleDialog ();

public slots:
    void slotWidthChanged (int width);
    void slotHeightChanged (int height);

    void slotWidthPercentChanged (double widthPercent);
    void slotHeightPercentChanged (double heightPercent);

    void slotLockAspectRatioToggled (bool on);

private:
    void widthFitHeightToAspectRatio (int width);
    void heightFitWidthToAspectRatio (int height);

public:
    int imageWidth () const;
    int imageHeight () const;
    bool scaleToFit () const;

    bool isNoop () const;

private:
    int m_oldWidth, m_oldHeight;
    bool m_dontAdjustAspectRatio;

    KIntNumInput *m_inpWidthVal, *m_inpHeightVal;
    KDoubleNumInput *m_inpWidthPercentVal, *m_inpHeightPercentVal;
    QCheckBox *m_cbScaleToFit, *m_cbLockAspectRatio;
};

#endif  // __kptoolresizescale_h__
