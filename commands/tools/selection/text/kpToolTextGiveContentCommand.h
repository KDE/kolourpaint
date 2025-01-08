
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolTextGiveContentCommand_H
#define kpToolTextGiveContentCommand_H

#include "commands/tools/selection/kpAbstractSelectionContentCommand.h"

class kpTextSelection;

// Converts a text border (no text lines) to a text selection with 1 empty
// text line.  This must be executed before any manipulations can be made
// to a text selection.
//
// Text analog of kpToolSelectionPullFromDocumentCommand.
class kpToolTextGiveContentCommand : public kpAbstractSelectionContentCommand
{
public:
    kpToolTextGiveContentCommand(const kpTextSelection &originalSelBorder, const QString &name, kpCommandEnvironment *environ);
    ~kpToolTextGiveContentCommand() override;

    void execute() override;
    void unexecute() override;
};

#endif // kpToolTextGiveContentCommand_H
