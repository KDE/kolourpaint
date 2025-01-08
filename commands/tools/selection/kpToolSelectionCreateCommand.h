
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolSelectionCreateCommand_H
#define kpToolSelectionCreateCommand_H

#include "commands/kpNamedCommand.h"

class kpAbstractSelection;

class kpToolSelectionCreateCommand : public kpNamedCommand
{
public:
    // (if fromSelection doesn't have a pixmap, it will only recreate the region)
    kpToolSelectionCreateCommand(const QString &name, const kpAbstractSelection &fromSelection, kpCommandEnvironment *environ);
    ~kpToolSelectionCreateCommand() override;

    kpCommandSize::SizeType size() const override;

    const kpAbstractSelection *fromSelection() const;
    void setFromSelection(const kpAbstractSelection &fromSelection);

    void execute() override;
    void unexecute() override;

private:
    kpAbstractSelection *m_fromSelection;

    int m_textRow, m_textCol;
};

#endif // kpToolSelectionCreateCommand_H
