
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextPrivate.h"
#include "tools/selection/text/kpToolText.h"

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

// private
bool kpToolText::onSelectionToSelectText() const
{
    kpView *v = viewManager()->viewUnderCursor();
    if (!v) {
        return false;
    }

    return v->mouseOnSelectionToSelectText(currentViewPoint());
}

// private
QString kpToolText::haventBegunDrawUserMessageSelectText() const
{
    return i18n("Left click to change cursor position.");
}

// private
void kpToolText::setCursorSelectText()
{
    viewManager()->setCursor(Qt::IBeamCursor);
}

// private
void kpToolText::beginDrawSelectText()
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\t\tis select cursor pos";
#endif
    kpTextSelection *textSel = document()->textSelection();
    Q_ASSERT(textSel);

    int newRow, newCol;

    if (textSel->hasContent()) {
        newRow = textSel->closestTextRowForPoint(currentPoint());
        newCol = textSel->closestTextColForPoint(currentPoint());
    } else {
        newRow = newCol = 0;
    }

#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\t\t\told: row=" << viewManager()->textCursorRow() << "col=" << viewManager()->textCursorCol();
    qCDebug(kpLogTools) << "\t\t\tnew: row=" << newRow << "col=" << newCol;
#endif
    viewManager()->setTextCursorPosition(newRow, newCol);
}

// protected virtual
QVariant kpToolText::selectTextOperation(Operation op, const QVariant &data1, const QVariant &data2)
{
    (void)data1;
    (void)data2;

    switch (op) {
    case HaventBegunDrawUserMessage:
        return haventBegunDrawUserMessageSelectText();

    case SetCursor:
        setCursorSelectText();
        break;

    case BeginDraw:
        beginDrawSelectText();
        break;

    case Draw:
        // Do nothing.
        break;

    case Cancel:
        // Not called.  REFACTOR: Change this?
        break;

    case EndDraw:
        // Do nothing.
        break;

    default:
        Q_ASSERT(!"Unhandled operation");
        break;
    }

    return {};
}
