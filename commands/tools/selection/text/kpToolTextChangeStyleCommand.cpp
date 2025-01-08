
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextChangeStyleCommand.h"

#include "environments/commands/kpCommandEnvironment.h"
#include "layers/selections/text/kpTextSelection.h"

#include "kpLogCategories.h"

kpToolTextChangeStyleCommand::kpToolTextChangeStyleCommand(const QString &name,
                                                           const kpTextStyle &newTextStyle,
                                                           const kpTextStyle &oldTextStyle,
                                                           kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_newTextStyle(newTextStyle)
    , m_oldTextStyle(oldTextStyle)
{
}

kpToolTextChangeStyleCommand::~kpToolTextChangeStyleCommand() = default;

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextChangeStyleCommand::size() const
{
    return 0;
}

// public virtual [base kpCommand]
void kpToolTextChangeStyleCommand::execute()
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogCommands) << "kpToolTextChangeStyleCommand::execute()"
                           << " font=" << m_newTextStyle.fontFamily() << " fontSize=" << m_newTextStyle.fontSize() << " isBold=" << m_newTextStyle.isBold()
                           << " isItalic=" << m_newTextStyle.isItalic() << " isUnderline=" << m_newTextStyle.isUnderline()
                           << " isStrikeThru=" << m_newTextStyle.isStrikeThru();
#endif

    environ()->setTextStyle(m_newTextStyle);

    if (textSelection()) {
        textSelection()->setTextStyle(m_newTextStyle);
    }
}

// public virtual [base kpCommand]
void kpToolTextChangeStyleCommand::unexecute()
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogCommands) << "kpToolTextChangeStyleCommand::unexecute()"
                           << " font=" << m_newTextStyle.fontFamily() << " fontSize=" << m_newTextStyle.fontSize() << " isBold=" << m_newTextStyle.isBold()
                           << " isItalic=" << m_newTextStyle.isItalic() << " isUnderline=" << m_newTextStyle.isUnderline()
                           << " isStrikeThru=" << m_newTextStyle.isStrikeThru();
#endif

    environ()->setTextStyle(m_oldTextStyle);

    if (textSelection())
        textSelection()->setTextStyle(m_oldTextStyle);
}
