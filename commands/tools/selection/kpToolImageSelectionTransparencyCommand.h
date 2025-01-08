
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolImageSelectionTransparencyCommand_H
#define kpToolImageSelectionTransparencyCommand_H

#include "commands/kpNamedCommand.h"
#include "layers/selections/image/kpImageSelectionTransparency.h"

class kpToolImageSelectionTransparencyCommand : public kpNamedCommand
{
public:
    kpToolImageSelectionTransparencyCommand(const QString &name,
                                            const kpImageSelectionTransparency &st,
                                            const kpImageSelectionTransparency &oldST,
                                            kpCommandEnvironment *environ);
    ~kpToolImageSelectionTransparencyCommand() override;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    kpImageSelectionTransparency m_st, m_oldST;
};

#endif // kpToolImageSelectionTransparencyCommand_H
