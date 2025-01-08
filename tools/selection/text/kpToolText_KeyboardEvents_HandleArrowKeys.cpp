
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextPrivate.h"
#include "tools/selection/text/kpToolText.h"

#include <QList>

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
#include "layers/selections/text/kpTextSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

// protected
void kpToolText::handleUpKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tup pressed";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    if (!textLines.isEmpty() && cursorRow > 0) {
        cursorRow--;
        cursorCol = qMin(cursorCol, textLines[cursorRow].length());
        viewManager()->setTextCursorPosition(cursorRow, cursorCol);
    }

    e->accept();
}

// protected
void kpToolText::handleDownKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tdown pressed";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    if (!textLines.isEmpty() && cursorRow < textLines.size() - 1) {
        cursorRow++;
        cursorCol = qMin(cursorCol, textLines[cursorRow].length());
        viewManager()->setTextCursorPosition(cursorRow, cursorCol);
    }

    e->accept();
}

// protected
void kpToolText::handleLeftKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tleft pressed";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    if (!textLines.isEmpty()) {
        if ((e->modifiers() & Qt::ControlModifier) == 0) {
#if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove single char";
#endif

            MoveCursorLeft(textLines, &cursorRow, &cursorCol);
            viewManager()->setTextCursorPosition(cursorRow, cursorCol);
        } else {
#if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove to start of word";
#endif

            MoveCursorToWordStart(textLines, &cursorRow, &cursorCol);
            viewManager()->setTextCursorPosition(cursorRow, cursorCol);
        }
    }

    e->accept();
}

// protected
void kpToolText::handleRightKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tright pressed";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    if (!textLines.isEmpty()) {
        if ((e->modifiers() & Qt::ControlModifier) == 0) {
#if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove single char";
#endif

            MoveCursorRight(textLines, &cursorRow, &cursorCol);
            viewManager()->setTextCursorPosition(cursorRow, cursorCol);
        } else {
#if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove to start of next word";
#endif

            MoveCursorToNextWordStart(textLines, &cursorRow, &cursorCol);
            viewManager()->setTextCursorPosition(cursorRow, cursorCol);
        }
    }

    e->accept();
}

// protected
void kpToolText::handleHomeKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\thome pressed";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    if (!textLines.isEmpty()) {
        if (e->modifiers() & Qt::ControlModifier) {
            cursorRow = 0;
        }

        cursorCol = 0;

        viewManager()->setTextCursorPosition(cursorRow, cursorCol);
    }

    e->accept();
}

// protected
void kpToolText::handleEndKeyPress(QKeyEvent *e, const QList<QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tend pressed";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    if (!textLines.isEmpty()) {
        if (e->modifiers() & Qt::ControlModifier) {
            cursorRow = textLines.size() - 1;
        }

        cursorCol = textLines[cursorRow].length();

        viewManager()->setTextCursorPosition(cursorRow, cursorCol);
    }

    e->accept();
}
