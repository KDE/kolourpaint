
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


#include "kpCommandHistoryBase.h"

class kpMainWindow;
class kpToolSelectionCreateCommand;


//
// KolourPaint-specific command history functionality.
//
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
//
class kpCommandHistory : public kpCommandHistoryBase
{
Q_OBJECT

public:
    kpCommandHistory (bool doReadConfig, kpMainWindow *mainWindow);
    ~kpCommandHistory () override;

public:
    // Same as addCommand(), except that this has a more desirable behavior
    // when adding a selection border creation command: If the next undo command
    // also creates a selection border, it overwrites that command
    // with the given <cmd>, instead of adding to the undo history.
    //
    // This helps to reduce the number of consecutive selection border
    // creation commands in the history.  Exactly one border creation
    // command before each "real" selection command is useful as it allows
    // users to undo just that "real" operation and then do a different "real"
    // operation with the same border (as sometimes, exact borders are difficult
    // to recreate).  However, multiple consecutive border creation
    // commands get annoying since none of them mutate the document,
    // so if the user has not done a "real" command with the last selection
    // border (i.e. the next undo command), what this method is saying is
    // that the user wanted to throw away that border drag anyway.
    //
    // This special behavior is perfectly safe since border creation commands
    // do not mutate the document.
    //
    // If <cmd> creates a selection that is not just a border, this
    // method has the same effect as addCommand().
    //
    // REFACTOR: Why not just override addCommand() and test if it was given a
    //           kpToolSelectionCreateCommand?
    void addCreateSelectionCommand (kpToolSelectionCreateCommand *cmd,
        bool execute = true);

public slots:
    void undo () override;
    void redo () override;

protected:
    kpMainWindow *m_mainWindow;
};


#endif  // kpCommandHistory_H
