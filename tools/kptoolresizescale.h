
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


#ifndef __kptoolresizescale_h__
#define __kptoolresizescale_h__

#include <qpixmap.h>

#include <kcommand.h>
#include <kdialogbase.h>

#include <kpcolor.h>
#include <kpselection.h>

class QCheckBox;
class QGroupBox;
class QRadioButton;
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
    kpColor m_backgroundColor;

    int m_oldWidth, m_oldHeight;
    bool m_actOnTextSelection;
    QPixmap m_oldPixmap, m_oldRightPixmap, m_oldBottomPixmap;
    kpSelection m_oldSelection;
};

class kpToolResizeScaleDialog : public KDialogBase
{
Q_OBJECT

public:
    kpToolResizeScaleDialog (bool actOnSelection,
                             kpMainWindow *mainWindow);
    virtual ~kpToolResizeScaleDialog ();

private:
    static bool s_lastIsResize;
    static double s_lastPercentWidth, s_lastPercentHeight;
    static bool s_lastKeepAspectRatio;

private:
    void createOperationGroupBox (QWidget *baseWidget);
    void createDimensionsGroupBox (QWidget *baseWidget);

    void widthFitHeightToAspectRatio ();
    void heightFitWidthToAspectRatio ();

public slots:
    void slotIsResizeChanged ();

    void slotWidthChanged (int width);
    void slotHeightChanged (int height);

    void slotPercentWidthChanged (double percentWidth);
    void slotPercentHeightChanged (double percentHeight);

    void slotKeepAspectRatioToggled (bool on);

public:
    int imageWidth () const;
    int imageHeight () const;
    bool scaleToFit () const;

    bool isNoOp () const;

private:
    bool m_actOnSelection;
    bool m_actOnTextSelection;
    int m_oldWidth, m_oldHeight;

    QGroupBox *m_operationGroupBox;
    QRadioButton *m_resizeRadioButton, *m_scaleRadioButton;

    QGroupBox *m_dimensionsGroupBox;
    KIntNumInput *m_newWidthInput, *m_newHeightInput;
    KDoubleNumInput *m_percentWidthInput, *m_percentHeightInput;
    QCheckBox *m_keepAspectRatioCheckBox;

    int m_ignoreKeepAspectRatio;
};

#endif  // __kptoolresizescale_h__
