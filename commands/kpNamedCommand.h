
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpNamedCommand_H
#define kpNamedCommand_H

#include "commands/kpCommand.h"

#include <QString>

class kpNamedCommand : public kpCommand
{
public:
    kpNamedCommand(const QString &name, kpCommandEnvironment *environ);

    QString name() const override;

protected:
    QString m_name;
};

#endif // kpNamedCommand_H
