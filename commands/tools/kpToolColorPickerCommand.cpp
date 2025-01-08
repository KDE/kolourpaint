
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_COLOR_PICKER 0

#include "kpToolColorPickerCommand.h"

#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"

#include <KLocalizedString>

kpToolColorPickerCommand::kpToolColorPickerCommand(int mouseButton, const kpColor &newColor, const kpColor &oldColor, kpCommandEnvironment *environ)

    : kpCommand(environ)
    , m_mouseButton(mouseButton)
    , m_newColor(newColor)
    , m_oldColor(oldColor)
{
}

kpToolColorPickerCommand::~kpToolColorPickerCommand() = default;

// public virtual [base kpCommand]
QString kpToolColorPickerCommand::name() const
{
    return i18n("Color Picker");
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolColorPickerCommand::size() const
{
    return 0;
}

// public virtual [base kpCommand]
void kpToolColorPickerCommand::execute()
{
    environ()->setColor(m_mouseButton, m_newColor);
}

// public virtual [base kpCommand]
void kpToolColorPickerCommand::unexecute()
{
    environ()->setColor(m_mouseButton, m_oldColor);
}
