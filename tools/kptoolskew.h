
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


#ifndef __kptool_skew_h__
#define __kptool_skew_h__

#include <kcommand.h>
#include <kdialogbase.h>

#include <kpcolor.h>

class QPixmap;

class KIntNumInput;

class kpDocument;
class kpMainWindow;
class kpToolSkewDialogWidget;


class kpToolSkewCommand : public KCommand
{
public:
    kpToolSkewCommand (bool actOnSelection,
                       double hangle, double vangle,
                       kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolSkewCommand ();

private:
    kpDocument *document () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    bool m_actOnSelection;
    double m_hangle, m_vangle;
    kpMainWindow *m_mainWindow;

    kpColor m_backgroundColor;
    QPixmap *m_oldPixmapPtr;
};


class kpToolSkewDialog : public KDialogBase
{
public:
    kpToolSkewDialog (QWidget *parent);
    virtual ~kpToolSkewDialog ();

    double horizontalAngle () const;
    double verticalAngle () const;

    bool isNoop () const;

private:
    kpToolSkewDialogWidget *m_mainWidget;
};

#endif  // __kptool_skew_h__
