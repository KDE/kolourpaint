
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


#include "commands/kpMacroCommand.h"
#include "views/manager/kpViewManager.h"

#include <climits>

#include <QtAlgorithms>

//---------------------------------------------------------------------

kpMacroCommand::kpMacroCommand (const QString &name, kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ)
{
}

//---------------------------------------------------------------------

kpMacroCommand::~kpMacroCommand ()
{
    qDeleteAll(m_commandList);
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpMacroCommand::size () const
{
#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "kpMacroCommand::size()";
#endif
    SizeType s = 0;

#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "\tcalculating:";
#endif
    foreach (kpCommand *cmd, m_commandList)
    {
    #if DEBUG_KP_COMMAND_HISTORY && 0
        qCDebug(kpLogCommands) << "\t\tcurrentSize=" << s << " + "
                   << cmd->name () << ".size=" << cmd->size ();
    #endif
        s += cmd->size ();
    }

#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "\treturning " << s;
#endif
    return s;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::execute ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpMacroCommand::execute()";
#endif

    viewManager()->setQueueUpdates();

    foreach (kpCommand *command, m_commandList)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\texecuting " << command->name();
    #endif
        command->execute();
    }

    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::unexecute ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpMacroCommand::unexecute()";
#endif

    viewManager()->setQueueUpdates();

    for (int i = m_commandList.count() - 1; i >= 0; i--)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\tunexecuting " << m_commandList[i]->name();
    #endif
        m_commandList[i]->unexecute();
    }

    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public
void kpMacroCommand::addCommand(kpCommand *command)
{
    m_commandList.append(command);
}

//---------------------------------------------------------------------
