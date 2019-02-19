
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
#include "kpLogCategories.h"

//---------------------------------------------------------------------

struct kpMacroCommandPrivate
{
};


kpMacroCommand::kpMacroCommand (const QString &name, kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      d (new kpMacroCommandPrivate ())
{
}

//---------------------------------------------------------------------

kpMacroCommand::~kpMacroCommand ()
{
    qDeleteAll (m_commandList.begin (), m_commandList.end ());
    delete d;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpMacroCommand::size () const
{
    qCDebug(kpLogCommands) << "kpMacroCommand::size()";
    SizeType s = 0;

    qCDebug(kpLogCommands) << "\tcalculating:";
    for (auto *cmd : m_commandList)
    {
        qCDebug(kpLogCommands) << "\t\tcurrentSize=" << s << " + "
                   << cmd->name () << ".size=" << cmd->size ();
        s += cmd->size ();
    }

    qCDebug(kpLogCommands) << "\treturning " << s;
    return s;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::execute ()
{
    qCDebug(kpLogCommands) << "kpMacroCommand::execute()";

    viewManager()->setQueueUpdates();

    for (QLinkedList <kpCommand *>::const_iterator it = m_commandList.begin ();
         it != m_commandList.end ();
         ++it)
    {
        qCDebug(kpLogCommands) << "\texecuting " << (*it)->name ();
        (*it)->execute ();
    }

    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::unexecute ()
{
    qCDebug(kpLogCommands) << "kpMacroCommand::unexecute()";

    viewManager()->setQueueUpdates();

    QLinkedList <kpCommand *>::const_iterator it = m_commandList.end ();
    it--;

    while (it != m_commandList.end ())
    {
        qCDebug(kpLogCommands) << "\tunexecuting " << (*it)->name ();
        (*it)->unexecute ();

        it--;
    }

    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public
void kpMacroCommand::addCommand (kpCommand *command)
{
    m_commandList.push_back (command);
}

//---------------------------------------------------------------------
