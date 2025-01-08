
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

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
#include "tools/selection/text/kpToolText.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

#include <QList>

#include <KLocalizedString>

// protected static
bool kpToolText::CursorIsOnWordChar(const QList<QString> &textLines, int cursorRow, int cursorCol)
{
    return (cursorRow >= 0 && cursorRow < textLines.size() && cursorCol >= 0 && cursorCol < textLines[cursorRow].length()
            && !textLines[cursorRow][cursorCol].isSpace());
}

// protected static
bool kpToolText::CursorIsAtStart(const QList<QString> &, int cursorRow, int cursorCol)
{
    return (cursorRow == 0 && cursorCol == 0);
}

// protected static
bool kpToolText::CursorIsAtEnd(const QList<QString> &textLines, int cursorRow, int cursorCol)
{
    if (textLines.isEmpty()) {
        return (cursorRow == 0 && cursorCol == 0);
    }

    return (cursorRow == textLines.size() - 1 && cursorCol == textLines[cursorRow].length());
}

// protected static
void kpToolText::MoveCursorLeft(const QList<QString> &textLines, int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty()) {
        return;
    }

    (*cursorCol)--;

    if (*cursorCol < 0) {
        (*cursorRow)--;
        if (*cursorRow < 0) {
            *cursorRow = 0;
            *cursorCol = 0;
        } else {
            *cursorCol = textLines[*cursorRow].length();
        }
    }
}

// protected static
void kpToolText::MoveCursorRight(const QList<QString> &textLines, int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty()) {
        return;
    }

    (*cursorCol)++;

    if (*cursorCol > textLines[*cursorRow].length()) {
        (*cursorRow)++;
        if (*cursorRow > textLines.size() - 1) {
            *cursorRow = textLines.size() - 1;
            *cursorCol = textLines[*cursorRow].length();
        } else {
            *cursorCol = 0;
        }
    }
}

#define IS_ON_SPACE_OR_EOL() !CursorIsOnWordChar(textLines, *cursorRow, *cursorCol)

// protected static
int kpToolText::MoveCursorToWordStart(const QList<QString> &textLines, int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty()) {
        return 0;
    }

    int numMoves = 0;

#define IS_ON_ANCHOR()                                                                                                                                         \
    (CursorIsOnWordChar(textLines, *cursorRow, *cursorCol) && (cursorCol == nullptr || !CursorIsOnWordChar(textLines, *cursorRow, *cursorCol - 1)))
#define MOVE_CURSOR_LEFT() (MoveCursorLeft(textLines, cursorRow, cursorCol), ++numMoves)

    // (these comments will exclude the row=0,col=0 boundary case)

    if (IS_ON_ANCHOR()) {
        MOVE_CURSOR_LEFT();
    }

    // --- now we're not on an anchor point (start of word) ---

    // End up on a letter...
    while (!(*cursorRow == 0 && *cursorCol == 0) && (IS_ON_SPACE_OR_EOL())) {
        MOVE_CURSOR_LEFT();
    }

    // --- now we're on a letter ---

    // Find anchor point
    while (!(*cursorRow == 0 && *cursorCol == 0) && !IS_ON_ANCHOR()) {
        MOVE_CURSOR_LEFT();
    }

#undef IS_ON_ANCHOR
#undef MOVE_CURSOR_LEFT

    return numMoves;
}

// protected static
int kpToolText::MoveCursorToNextWordStart(const QList<QString> &textLines, int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty()) {
        return 0;
    }

    int numMoves = 0;

#define IS_AT_END() CursorIsAtEnd(textLines, *cursorRow, *cursorCol)
#define MOVE_CURSOR_RIGHT() (MoveCursorRight(textLines, cursorRow, cursorCol), ++numMoves)

    // (these comments will exclude the last row,end col boundary case)

    // Find space
    while (!IS_AT_END() && !IS_ON_SPACE_OR_EOL()) {
        MOVE_CURSOR_RIGHT();
    }

    // --- now we're on a space ---

    // Find letter
    while (!IS_AT_END() && IS_ON_SPACE_OR_EOL()) {
        MOVE_CURSOR_RIGHT();
    }

    // --- now we're on a letter ---

#undef IS_AT_END
#undef MOVE_CURSOR_RIGHT

    return numMoves;
}

#undef IS_ON_SPACE_OR_EOL
