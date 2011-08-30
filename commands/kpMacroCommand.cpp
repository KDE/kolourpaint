
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


#include <kpMacroCommand.h>
#include <kpViewManager.h>

#include <climits>

#include <QtAlgorithms>

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
#if DEBUG_KP_COMMAND_HISTORY && 0
    kDebug () << "kpMacroCommand::size()";
#endif
    SizeType s = 0;

#if DEBUG_KP_COMMAND_HISTORY && 0
    kDebug () << "\tcalculating:";
#endif
    foreach (kpCommand *cmd, m_commandList)
    {
    #if DEBUG_KP_COMMAND_HISTORY && 0
        kDebug () << "\t\tcurrentSize=" << s << " + "
                   << cmd->name () << ".size=" << cmd->size ()
                   << endl;
    #endif
        s += cmd->size ();
    }

#if DEBUG_KP_COMMAND_HISTORY && 0
    kDebug () << "\treturning " << s;
#endif
    return s;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::execute ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpMacroCommand::execute()";
#endif

    viewManager()->setQueueUpdates();

    for (QLinkedList <kpCommand *>::const_iterator it = m_commandList.begin ();
         it != m_commandList.end ();
         ++it)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\texecuting " << (*it)->name ();
    #endif
        (*it)->execute ();
    }

    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::unexecute ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpMacroCommand::unexecute()";
#endif

    viewManager()->setQueueUpdates();

    QLinkedList <kpCommand *>::const_iterator it = m_commandList.end ();
    it--;

    while (it != m_commandList.end ())
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\tunexecuting " << (*it)->name ();
    #endif
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
