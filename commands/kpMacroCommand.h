
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpMacroCommand_H
#define kpMacroCommand_H

#include "commands/kpNamedCommand.h"

#include <QList>

class kpMacroCommand : public kpNamedCommand
{
public:
    kpMacroCommand(const QString &name, kpCommandEnvironment *environ);
    ~kpMacroCommand() override;

    //
    // kpCommand Interface
    //

    SizeType size() const override;

    void execute() override;
    void unexecute() override;

    //
    // Interface
    //

    void addCommand(kpCommand *command);

protected:
    QList<kpCommand *> m_commandList;
};

#endif // kpMacroCommand_H
