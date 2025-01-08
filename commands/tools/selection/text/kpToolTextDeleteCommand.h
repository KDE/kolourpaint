
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_TEXT_DELETE_COMMAND_H
#define KP_TOOL_TEXT_DELETE_COMMAND_H

#include "commands/kpNamedCommand.h"

class kpToolTextDeleteCommand : public kpNamedCommand
{
public:
    enum Action {
        DontAddDeleteYet,
        AddDeleteNow
    };

    kpToolTextDeleteCommand(const QString &name, int row, int col, Action action, kpCommandEnvironment *environ);
    ~kpToolTextDeleteCommand() override;

    void addDelete();

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

protected:
    int m_row, m_col;
    int m_numDeletes;
    QString m_deletedText;
};

#endif // KP_TOOL_TEXT_DELETE_COMMAND_H
