
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


#ifndef __kptoolflip_h__
#define __kptoolflip_h__

#include <kpcommandhistory.h>
#include <kdialogbase.h>

class QRadioButton;
class QString;

class kpDocument;
class kpMainWindow;


class kpToolFlipCommand : public kpCommand
{
public:
    kpToolFlipCommand (bool actOnSelection,
                       bool horiz, bool vert,
                       kpMainWindow *mainWindow);
    virtual ~kpToolFlipCommand ();

    virtual QString name () const;

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    void flip ();

    bool m_actOnSelection;
    bool m_horiz, m_vert;
};


class kpToolFlipDialog : public KDialogBase
{
Q_OBJECT

public:
    kpToolFlipDialog (bool actOnSelection, QWidget *parent);
    ~kpToolFlipDialog ();

private:
    static bool s_lastIsVerticalFlip;

public slots:
    void slotIsVerticalFlipChanged ();

public:
    bool getHorizontalFlip () const;
    bool getVerticalFlip () const;
    bool isNoOp () const;

private:
    QRadioButton *m_horizontalFlipRadioButton, *m_verticalFlipRadioButton;
};

#endif  // __kptoolflip_h__
