
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpLogCategories.h"
#include "kpToolText.h"
#include "kpToolTextPrivate.h"

#include "commands/tools/selection/text/kpToolTextBackspaceCommand.h"
#include "commands/tools/selection/text/kpToolTextDeleteCommand.h"
#include "commands/tools/selection/text/kpToolTextEnterCommand.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "views/manager/kpViewManager.h"

#include <KLocalizedString>

// private
void kpToolText::endTypingCommands()
{
    d->insertCommand = nullptr;
    d->enterCommand = nullptr;

    d->backspaceCommand = nullptr;
    d->backspaceWordCommand = nullptr;

    d->deleteCommand = nullptr;
    d->deleteWordCommand = nullptr;
}

// private
void kpToolText::addNewBackspaceCommand(kpToolTextBackspaceCommand **cmd)
{
    // TODO: why not endShapeInternal(); ditto for everywhere else in kpToolText*.cpp?
    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    giveContentIfNeeded();

    *cmd = new kpToolTextBackspaceCommand(i18n("Text: Backspace"),
                                          viewManager()->textCursorRow(),
                                          viewManager()->textCursorCol(),
                                          kpToolTextBackspaceCommand::DontAddBackspaceYet,
                                          environ()->commandEnvironment());
    addNeedingContentCommand(*cmd);
}

// private
void kpToolText::addNewDeleteCommand(kpToolTextDeleteCommand **cmd)
{
    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    giveContentIfNeeded();

    *cmd = new kpToolTextDeleteCommand(i18n("Text: Delete"),
                                       viewManager()->textCursorRow(),
                                       viewManager()->textCursorCol(),
                                       kpToolTextDeleteCommand::DontAddDeleteYet,
                                       environ()->commandEnvironment());
    addNeedingContentCommand(*cmd);
}

// private
void kpToolText::addNewEnterCommand(kpToolTextEnterCommand **cmd)
{
    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    giveContentIfNeeded();

    *cmd = new kpToolTextEnterCommand(i18n("Text: New Line"),
                                      viewManager()->textCursorRow(),
                                      viewManager()->textCursorCol(),
                                      kpToolTextEnterCommand::DontAddEnterYet,
                                      environ()->commandEnvironment());
    addNeedingContentCommand(*cmd);
}

// private
void kpToolText::addNewInsertCommand(kpToolTextInsertCommand **cmd)
{
    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    giveContentIfNeeded();

    *cmd = new kpToolTextInsertCommand(i18n("Text: Write"),
                                       viewManager()->textCursorRow(),
                                       viewManager()->textCursorCol(),
                                       QString(),
                                       environ()->commandEnvironment());
    addNeedingContentCommand(*cmd);
}
