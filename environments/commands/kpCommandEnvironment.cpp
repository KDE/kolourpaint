
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


#include <kpCommandEnvironment.h>

#include <kpColorToolBar.h>
#include <kpDocument.h>
#include <kpMainWindow.h>
#include <kpImageSelectionTransparency.h>
#include <kpTextStyle.h>
#include <kpTool.h>


struct kpCommandEnvironmentPrivate
{
};

kpCommandEnvironment::kpCommandEnvironment (kpMainWindow *mainWindow)
    : kpEnvironmentBase (mainWindow),
      d (new kpCommandEnvironmentPrivate ())
{
}

kpCommandEnvironment::~kpCommandEnvironment ()
{
    delete d;
}


// public
void kpCommandEnvironment::setColor (int which, const kpColor &color) const
{
    kpColorToolBar *toolBar = mainWindow ()->colorToolBar ();
    Q_ASSERT (toolBar);

    toolBar->setColor (which, color);
}


// public
void kpCommandEnvironment::somethingBelowTheCursorChanged () const
{
    kpTool *tool = mainWindow ()->tool ();
    Q_ASSERT (tool);

    tool->somethingBelowTheCursorChanged ();
}


// public
kpImageSelectionTransparency kpCommandEnvironment::imageSelectionTransparency () const
{
    return mainWindow ()->imageSelectionTransparency ();
}

// public
void kpCommandEnvironment::setImageSelectionTransparency (
        const kpImageSelectionTransparency &transparency,
        bool forceColorChange)
{
    mainWindow ()->setImageSelectionTransparency (transparency, forceColorChange);
}


// public
kpTextStyle kpCommandEnvironment::textStyle () const
{
    return mainWindow ()->textStyle ();
}

// public
void kpCommandEnvironment::setTextStyle (const kpTextStyle &textStyle)
{
    mainWindow ()->setTextStyle (textStyle);
}


#include <kpCommandEnvironment.moc>
