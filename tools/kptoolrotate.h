
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


#ifndef __kptoolrotate_h__
#define __kptoolrotate_h__

#include <qpixmap.h>
#include <qpoint.h>

#include <kcommand.h>
#include <kdialogbase.h>

#include <kpcolor.h>


class QRadioButton;
class QString;

class KDoubleNumInput;

class kpDocument;
class kpViewManager;
class kpMainWindow;


class kpToolRotateCommand : public KCommand
{
public:
    kpToolRotateCommand (bool actOnSelection,
                         double angle,  // 0 <= angle < 360 (clockwise)
                         kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolRotateCommand ();

private:
    kpDocument *document () const;
    kpViewManager *viewManager () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    bool m_actOnSelection;
    double m_angle;
    kpMainWindow *m_mainWindow;

    kpColor m_backgroundColor;

    bool m_losslessRotation;
    QPixmap m_oldPixmap;
    QPoint m_oldTopLeft;
};


class kpToolRotateDialog : public KDialogBase
{
Q_OBJECT

public:
    kpToolRotateDialog (QWidget *parent);
    virtual ~kpToolRotateDialog ();

public slots:
    void slotAngleChanged ();

public:
    double angle () const;  // 0 <= angle < 360 (clockwise)
    bool isNoopRotate () const;

private:
    QRadioButton *m_rbRotateLeft, *m_rbRotateRight,
                 *m_rbRotate180,
                 *m_rbRotateArbitrary;
    KDoubleNumInput *m_inpAngle;
};

/*#include <qpixmap.h>
#include <kcommand.h>
#include <kptool.h>

class QPoint;
class QRect;

class kpToolRotateCommand;
class kpDocument;
class kpViewManager;

class kpToolRotate : public kpTool
{
public:
    kpToolRotate (kpMainWindow *);
    virtual ~kpToolRotate ();

    virtual bool careAboutModifierState () const { return true; }

    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void draw (const QPoint &thisPoint, const QPoint &, const QRect &);
    virtual void cancelShape ();

    virtual void endDraw (const QPoint &thisPoint, const QRect &);

    static double radiansToDegrees (const double &rad);

private:
    double angle () const;
    double angleFromStart () const;

    QPixmap m_oldPixmap;
    double m_startAngle;
};


*/

#endif  // __kptoolrotate_h__
