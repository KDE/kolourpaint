
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_COMMAND_HISTORY 0

#include "kpCommand.h"

#include "environments/commands/kpCommandEnvironment.h"

kpCommand::kpCommand(kpCommandEnvironment *environ)
    : m_environ(environ)
{
    Q_ASSERT(environ);
}

kpCommand::~kpCommand() = default;

kpCommandEnvironment *kpCommand::environ() const
{
    return m_environ;
}

// protected
kpDocument *kpCommand::document() const
{
    return m_environ->document();
}

// protected
kpAbstractSelection *kpCommand::selection() const
{
    return m_environ->selection();
}

// protected
kpAbstractImageSelection *kpCommand::imageSelection() const
{
    return m_environ->imageSelection();
}

// protected
kpTextSelection *kpCommand::textSelection() const
{
    return m_environ->textSelection();
}

// protected
kpViewManager *kpCommand::viewManager() const
{
    return m_environ->viewManager();
}
