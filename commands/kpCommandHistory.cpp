
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_COMMAND_HISTORY 0

#include "kpCommandHistory.h"

#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "layers/selections/kpAbstractSelection.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"

kpCommandHistory::kpCommandHistory(bool doReadConfig, kpMainWindow *mainWindow)
    : kpCommandHistoryBase(doReadConfig, mainWindow->actionCollection())
    , m_mainWindow(mainWindow)
{
}

kpCommandHistory::~kpCommandHistory() = default;

static bool NextUndoCommandIsCreateBorder(kpCommandHistory *commandHistory)
{
    Q_ASSERT(commandHistory);

    kpCommand *cmd = commandHistory->nextUndoCommand();
    if (!cmd) {
        return false;
    }

    auto *c = dynamic_cast<kpToolSelectionCreateCommand *>(cmd);
    if (!c) {
        return false;
    }

    const kpAbstractSelection *sel = c->fromSelection();
    Q_ASSERT(sel);

    return (!sel->hasContent());
}

// public
void kpCommandHistory::addCreateSelectionCommand(kpToolSelectionCreateCommand *cmd, bool execute)
{
    if (cmd->fromSelection()->hasContent()) {
        addCommand(cmd, execute);
        return;
    }

    if (::NextUndoCommandIsCreateBorder(this)) {
        setNextUndoCommand(cmd);
        if (execute) {
            cmd->execute();
        }
    } else {
        addCommand(cmd, execute);
    }
}

//---------------------------------------------------------------------

// public slot virtual [base KCommandHistory]
void kpCommandHistory::undo()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistory::undo() CALLED!";
#endif
    if (m_mainWindow && m_mainWindow->toolHasBegunShape()) {
#if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\thas begun shape - cancel draw";
#endif
        m_mainWindow->tool()->cancelShapeInternal();
    } else {
        kpCommandHistoryBase::undo();
    }
}

//---------------------------------------------------------------------

// public slot virtual [base KCommandHistory]
void kpCommandHistory::redo()
{
    if (m_mainWindow && m_mainWindow->toolHasBegunShape()) {
        // Not completely obvious but what else can we do?
        //
        // Ignoring the request would not be intuitive for tools like
        // Polygon & Polyline (where it's not always apparent to the user
        // that s/he's still drawing a shape even though the mouse isn't
        // down).
        m_mainWindow->tool()->cancelShapeInternal();
    } else {
        kpCommandHistoryBase::redo();
    }
}

#include "moc_kpCommandHistory.cpp"
