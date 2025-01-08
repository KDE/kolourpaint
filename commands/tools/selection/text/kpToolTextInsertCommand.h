
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_TEXT_INSERT_COMMAND_H
#define KP_TOOL_TEXT_INSERT_COMMAND_H

#include "commands/kpNamedCommand.h"

class kpToolTextInsertCommand : public kpNamedCommand
{
public:
    kpToolTextInsertCommand(const QString &name, int row, int col, const QString &newText, kpCommandEnvironment *environ);

    void addText(const QString &moreText);

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

protected:
    int m_row, m_col;
    QString m_newText;
};

#endif // KP_TOOL_TEXT_INSERT_COMMAND_H
