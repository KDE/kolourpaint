/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "commands/kpNamedCommand.h"

//---------------------------------------------------------------------

kpNamedCommand::kpNamedCommand(const QString &name, kpCommandEnvironment *environ)
    : kpCommand(environ)
    , m_name(name)
{
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
QString kpNamedCommand::name() const
{
    return m_name;
}

//---------------------------------------------------------------------
