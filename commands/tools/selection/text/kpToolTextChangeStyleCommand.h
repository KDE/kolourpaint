
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_TEXT_CHANGE_STYLE_COMMAND_H
#define KP_TOOL_TEXT_CHANGE_STYLE_COMMAND_H

#include "commands/kpNamedCommand.h"
#include "layers/selections/text/kpTextStyle.h"

class kpToolTextChangeStyleCommand : public kpNamedCommand
{
public:
    kpToolTextChangeStyleCommand(const QString &name, const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle, kpCommandEnvironment *environ);
    ~kpToolTextChangeStyleCommand() override;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

protected:
    kpTextStyle m_newTextStyle, m_oldTextStyle;
};

#endif // KP_TOOL_TEXT_CHANGE_STYLE_COMMAND_H
