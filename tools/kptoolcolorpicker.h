
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


#ifndef __kptoolcolorpicker_h__
#define __kptoolcolorpicker_h__

#include <kpcommandhistory.h>

#include <kpcolor.h>
#include <kptool.h>

class QPoint;
class QRect;

class kpColorToolBar;

class kpToolColorPicker : public kpTool
{
Q_OBJECT

public:
    kpToolColorPicker (kpMainWindow *);
    virtual ~kpToolColorPicker ();

    // generally the user goes to pick a color but wants to return to using
    // his/her previous drawing tool
    virtual bool returnToPreviousToolAfterEndDraw () const { return true; }

private:
    QString haventBegunDrawUserMessage () const;

public:
    virtual void begin ();
    virtual void beginDraw ();
    virtual void draw (const QPoint &thisPoint, const QPoint &, const QRect &);
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endDraw (const QPoint &thisPoint, const QRect &);

private:
    kpColor colorAtPixel (const QPoint &p);

    kpColor m_oldColor;
};

class kpToolColorPickerCommand : public kpCommand
{
public:
    kpToolColorPickerCommand (int mouseButton,
                              const kpColor &newColor, const kpColor &oldColor,
                              kpMainWindow *mainWindow);
    virtual ~kpToolColorPickerCommand ();

    virtual QString name () const;

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    kpColorToolBar *colorToolBar () const;
    
private:
    int m_mouseButton;
    kpColor m_newColor;
    kpColor m_oldColor;
};

#endif  // __kptoolcolorpicker_h__
