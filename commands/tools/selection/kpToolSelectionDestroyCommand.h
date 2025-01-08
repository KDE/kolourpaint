
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolSelectionDestroyCommand_H
#define kpToolSelectionDestroyCommand_H

#include "commands/kpNamedCommand.h"
#include "imagelib/kpImage.h"

class kpAbstractSelection;

class kpToolSelectionDestroyCommand : public kpNamedCommand
{
public:
    kpToolSelectionDestroyCommand(const QString &name, bool pushOntoDocument, kpCommandEnvironment *environ);
    ~kpToolSelectionDestroyCommand() override;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    bool m_pushOntoDocument;
    kpImage m_oldDocImage;
    kpAbstractSelection *m_oldSelectionPtr;

    int m_textRow, m_textCol;
};

#endif // kpToolSelectionDestroyCommand_H
