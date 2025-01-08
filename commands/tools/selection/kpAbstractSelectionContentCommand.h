
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpAbstractSelectionContentCommand_H
#define kpAbstractSelectionContentCommand_H

#include "commands/kpNamedCommand.h"

// Converts a selection border to a selection with content.
// This must be executed before any manipulations can be made
// to a selection.
//
// Its construction and execution always follows that of a
// kpToolSelectionCreateCommand, which must be given a selection with
// no content.
//
// It's always the first subcommand of a kpMacroCommand, with the following
// subcommands being whatever the selection operation is (e.g. movement,
// resizing).
class kpAbstractSelectionContentCommand : public kpNamedCommand
{
    // LOREFACTOR: Pull up more methods into here?  Looking at the code, not
    //             much could be dragged up without unnecessarily complicated
    //             abstraction.
public:
    // <originalSelBorder> must be a border i.e. have no content.
    kpAbstractSelectionContentCommand(const kpAbstractSelection &originalSelBorder, const QString &name, kpCommandEnvironment *environ);
    ~kpAbstractSelectionContentCommand() override;

    kpCommandSize::SizeType size() const override;

    // Note: Returned pointer is only valid for as long as this command is
    //       alive.
    const kpAbstractSelection *originalSelection() const;

private:
    struct kpAbstractSelectionContentCommandPrivate *const d;
};

#endif // kpAbstractSelectionContentCommand_H
