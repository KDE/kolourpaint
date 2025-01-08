
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "commands/kpCommandHistory.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "commands/tools/selection/text/kpToolTextBackspaceCommand.h"
#include "commands/tools/selection/text/kpToolTextChangeStyleCommand.h"
#include "commands/tools/selection/text/kpToolTextDeleteCommand.h"
#include "commands/tools/selection/text/kpToolTextEnterCommand.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "kpLogCategories.h"
#include "kpToolTextPrivate.h"
#include "layers/selections/text/kpTextSelection.h"
#include "tools/selection/text/kpToolText.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

#include <QList>

//---------------------------------------------------------------------

// protected
void kpToolText::handleBackspaceKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tbackspace pressed";
#endif

    if (!textLines.isEmpty()) {
        if ((e->modifiers() & Qt::ControlModifier) == 0) {
            if (!d->backspaceCommand) {
                addNewBackspaceCommand(&d->backspaceCommand);
            }

            d->backspaceCommand->addBackspace();
        } else {
            if (!d->backspaceWordCommand) {
                addNewBackspaceCommand(&d->backspaceWordCommand);
            }

            const int numMoves = MoveCursorToWordStart(textLines, &cursorRow, &cursorCol);

            viewManager()->setQueueUpdates();
            {
                for (int i = 0; i < numMoves; i++) {
                    d->backspaceWordCommand->addBackspace();
                }
            }
            viewManager()->restoreQueueUpdates();

            Q_ASSERT(cursorRow == viewManager()->textCursorRow());
            Q_ASSERT(cursorCol == viewManager()->textCursorCol());
        }
    }

    e->accept();
}

//---------------------------------------------------------------------

// protected
void kpToolText::handleDeleteKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tdelete pressed";
#endif

    if (!textLines.isEmpty()) {
        if ((e->modifiers() & Qt::ControlModifier) == 0) {
            if (!d->deleteCommand) {
                addNewDeleteCommand(&d->deleteCommand);
            }

            d->deleteCommand->addDelete();
        } else {
            if (!d->deleteWordCommand) {
                addNewDeleteCommand(&d->deleteWordCommand);
            }

            // We don't want to know the cursor pos of the next word start
            // as delete should keep cursor in same pos.
            int cursorRowThrowAway = cursorRow, cursorColThrowAway = cursorCol;
            const int numMoves = MoveCursorToNextWordStart(textLines, &cursorRowThrowAway, &cursorColThrowAway);

            viewManager()->setQueueUpdates();
            {
                for (int i = 0; i < numMoves; i++) {
                    d->deleteWordCommand->addDelete();
                }
            }
            viewManager()->restoreQueueUpdates();

            // Assert unchanged as delete should keep cursor in same pos.
            Q_ASSERT(cursorRow == viewManager()->textCursorRow());
            Q_ASSERT(cursorCol == viewManager()->textCursorCol());
        }
    }

    e->accept();
}

//---------------------------------------------------------------------

// protected
void kpToolText::handleEnterKeyPress(QKeyEvent *e, const QList<QString> & /*textLines*/, int /*cursorRow*/, int /*cursorCol*/)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tenter pressed";
#endif

    // It's OK for <textLines> to be empty.

    if (!d->enterCommand) {
        addNewEnterCommand(&d->enterCommand);
    }

    d->enterCommand->addEnter();

    e->accept();
}

//---------------------------------------------------------------------

// protected
void kpToolText::handleTextTyped(QKeyEvent *e, const QList<QString> & /*textLines*/, int /*cursorRow*/, int /*cursorCol*/)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\ttext=" << e->text();
#endif
    QString usableText;
    for (int i = 0; i < e->text().length(); i++) {
        if (e->text().at(i).category() != QChar::Other_Control)
            usableText += e->text().at(i);
    }
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tusableText=" << usableText;
#endif

    if (usableText.isEmpty()) {
        // Don't end the current shape nor accept the event -- the event
        // wasn't for us.
        return;
    }

    // --- It's OK for <textLines> to be empty. ---

    if (!d->insertCommand) {
        addNewInsertCommand(&d->insertCommand);
    }

    d->insertCommand->addText(usableText);

    e->accept();
}
