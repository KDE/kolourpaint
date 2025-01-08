
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_TEXT_ENTER_COMMAND_H
#define KP_TOOL_TEXT_ENTER_COMMAND_H

#include "commands/kpNamedCommand.h"

class kpToolTextEnterCommand : public kpNamedCommand
{
public:
    enum Action {
        DontAddEnterYet,
        AddEnterNow
    };

    kpToolTextEnterCommand(const QString &name, int row, int col, Action action, kpCommandEnvironment *environ);
    ~kpToolTextEnterCommand() override;

    void addEnter();

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

protected:
    int m_row, m_col;
    int m_numEnters;
};

#endif // KP_TOOL_TEXT_ENTER_COMMAND_H
