
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


#ifndef __kptoolrotate_h__
#define __kptoolrotate_h__

#include <qpixmap.h>
#include <qpoint.h>

#include <kdialogbase.h>

#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpselection.h>
#include <kptoolpreviewdialog.h>


class QButtonGroup;
class QRadioButton;
class QString;

class KIntNumInput;

class kpDocument;
class kpViewManager;
class kpMainWindow;


class kpToolRotateCommand : public kpCommand
{
public:
    kpToolRotateCommand (bool actOnSelection,
                         double angle,  // 0 <= angle < 360 (clockwise)
                         kpMainWindow *mainWindow);
    virtual ~kpToolRotateCommand ();

    virtual QString name () const;

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    bool m_actOnSelection;
    double m_angle;

    kpColor m_backgroundColor;

    bool m_losslessRotation;
    QPixmap m_oldPixmap;
    kpSelection m_oldSelection;
};


class kpToolRotateDialog : public kpToolPreviewDialog
{
Q_OBJECT

public:
    kpToolRotateDialog (bool actOnSelection,
                        kpMainWindow *parent,
                        const char *name = 0);
    virtual ~kpToolRotateDialog ();

private:
    static int s_lastWidth, s_lastHeight;
    static bool s_lastIsClockwise;
    static int s_lastAngleRadioButtonID;
    static int s_lastAngleCustom;

    void createDirectionGroupBox ();
    void createAngleGroupBox ();

public:
    virtual bool isNoOp () const;
    int angle () const;  // 0 <= angle < 360 (clockwise);

private:
    virtual QSize newDimensions () const;
    virtual QPixmap transformPixmap (const QPixmap &pixmap,
                                     int targetWidth, int targetHeight) const;

private slots:
    void slotAngleCustomRadioButtonToggled (bool isChecked);
    virtual void slotUpdate ();

private slots:
    virtual void slotOk ();

private:
    QRadioButton *m_antiClockwiseRadioButton,
                 *m_clockwiseRadioButton;

    QButtonGroup *m_angleButtonGroup;
    QRadioButton *m_angle90RadioButton,
                 *m_angle180RadioButton,
                 *m_angle270RadioButton,
                 *m_angleCustomRadioButton;
    KIntNumInput *m_angleCustomInput;
};


#endif  // __kptoolrotate_h__
