
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_TEXT_BACKSPACE_COMMAND_H
#define KP_TOOL_TEXT_BACKSPACE_COMMAND_H

#include "commands/kpNamedCommand.h"

class kpToolTextBackspaceCommand : public kpNamedCommand
{
public:
    enum Action {
        DontAddBackspaceYet,
        AddBackspaceNow
    };

    kpToolTextBackspaceCommand(const QString &name, int row, int col, Action action, kpCommandEnvironment *environ);
    ~kpToolTextBackspaceCommand() override;

    void addBackspace();

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

protected:
    int m_row, m_col;
    int m_numBackspaces;
    QString m_deletedText;
};

#endif // KP_TOOL_TEXT_BACKSPACE_COMMAND_H
