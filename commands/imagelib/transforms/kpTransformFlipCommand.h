
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformFlipCommand_H
#define kpTransformFlipCommand_H

#include "commands/kpCommand.h"

class kpTransformFlipCommand : public kpCommand
{
public:
    kpTransformFlipCommand(bool actOnSelection, bool horiz, bool vert, kpCommandEnvironment *environ);

    ~kpTransformFlipCommand() override;

    QString name() const override;

    SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    void flip();

    bool m_actOnSelection;
    bool m_horiz, m_vert;
};

#endif // kpTransformFlipCommand_H
