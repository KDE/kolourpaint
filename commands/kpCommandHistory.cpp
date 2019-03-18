
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


#define DEBUG_KP_COMMAND_HISTORY 0


#include "kpCommandHistory.h"

#include "layers/selections/kpAbstractSelection.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"


kpCommandHistory::kpCommandHistory (bool doReadConfig, kpMainWindow *mainWindow)
    : kpCommandHistoryBase (doReadConfig, mainWindow->actionCollection ()),
      m_mainWindow (mainWindow)
{
}

kpCommandHistory::~kpCommandHistory () = default;


static bool NextUndoCommandIsCreateBorder (kpCommandHistory *commandHistory)
{
    Q_ASSERT (commandHistory);

    kpCommand *cmd = commandHistory->nextUndoCommand ();
    if (!cmd) {
        return false;
    }

    auto *c = dynamic_cast <kpToolSelectionCreateCommand *> (cmd);
    if (!c) {
        return false;
    }

    const kpAbstractSelection *sel = c->fromSelection ();
    Q_ASSERT (sel);

    return (!sel->hasContent ());
}

// public
void kpCommandHistory::addCreateSelectionCommand (kpToolSelectionCreateCommand *cmd,
        bool execute)
{
    if (cmd->fromSelection ()->hasContent ())
    {
        addCommand (cmd, execute);
        return;
    }

    if (::NextUndoCommandIsCreateBorder (this))
    {
        setNextUndoCommand (cmd);
        if (execute) {
            cmd->execute ();
        }
    }
    else {
        addCommand (cmd, execute);
    }
}

//---------------------------------------------------------------------

// public slot virtual [base KCommandHistory]
void kpCommandHistory::undo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistory::undo() CALLED!";
#endif
    if (m_mainWindow && m_mainWindow->toolHasBegunShape ())
    {
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\thas begun shape - cancel draw";
    #endif
        m_mainWindow->tool ()->cancelShapeInternal ();
    }
    else {
        kpCommandHistoryBase::undo ();
    }
}

//---------------------------------------------------------------------

// public slot virtual [base KCommandHistory]
void kpCommandHistory::redo ()
{
    if (m_mainWindow && m_mainWindow->toolHasBegunShape ())
    {
        // Not completely obvious but what else can we do?
        //
        // Ignoring the request would not be intuitive for tools like
        // Polygon & Polyline (where it's not always apparent to the user
        // that s/he's still drawing a shape even though the mouse isn't
        // down).
        m_mainWindow->tool ()->cancelShapeInternal ();
    }
    else {
        kpCommandHistoryBase::redo ();
    }
}


