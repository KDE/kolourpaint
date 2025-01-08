
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_COMMAND_HISTORY 0

#include "commands/kpMacroCommand.h"
#include "views/manager/kpViewManager.h"

#include <climits>

#include <QtAlgorithms>

//---------------------------------------------------------------------

kpMacroCommand::kpMacroCommand(const QString &name, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
{
}

//---------------------------------------------------------------------

kpMacroCommand::~kpMacroCommand()
{
    qDeleteAll(m_commandList);
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpMacroCommand::size() const
{
#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "kpMacroCommand::size()";
#endif
    SizeType s = 0;

#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "\tcalculating:";
#endif
    for (kpCommand *cmd : m_commandList) {
#if DEBUG_KP_COMMAND_HISTORY && 0
        qCDebug(kpLogCommands) << "\t\tcurrentSize=" << s << " + " << cmd->name() << ".size=" << cmd->size();
#endif
        s += cmd->size();
    }

#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "\treturning " << s;
#endif
    return s;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::execute()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpMacroCommand::execute()";
#endif

    viewManager()->setQueueUpdates();

    for (kpCommand *command : std::as_const(m_commandList)) {
#if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\texecuting " << command->name();
#endif
        command->execute();
    }

    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpMacroCommand::unexecute()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpMacroCommand::unexecute()";
#endif

    viewManager()->setQueueUpdates();

    for (int i = m_commandList.count() - 1; i >= 0; i--) {
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
