
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


#ifndef kpCommandHistory_H
#define kpCommandHistory_H


#include <kpCommandHistoryBase.h>


// Intercepts Undo/Redo requests:
//
// If the user is currently drawing a shape, it cancels it.
// Else it passes on the Undo/Redo request to kpCommandHistoryBase.
//
// TODO: This is wrong.  It won't work if the Undo action is disabled,
//       for instance.  Later: What about kpToolText::viewEvent()'s use of
//       QEvent::ShortcutOverride?
//
//       Maybe the real solution is to call kpCommandHistoryBase::addCommand()
//       as _soon_ as the shape starts - not after it ends.  But the
//       trouble with this solution is that if the user Undoes/cancels
//       the shape s/he's currently drawing, it would replace a Redo
//       slot in the history.  Arguably you shouldn't be able to Redo
//       something you never finished drawing.
//
//       The solution is to add this functionality to kpCommandHistoryBase.
class kpCommandHistory : public kpCommandHistoryBase
{
Q_OBJECT

public:
    kpCommandHistory (bool doReadConfig, kpMainWindow *mainWindow);
    virtual ~kpCommandHistory ();

public slots:
    virtual void undo ();
    virtual void redo ();

protected:
    kpMainWindow *m_mainWindow;
};


#endif  // kpCommandHistory_H
