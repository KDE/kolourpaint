
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
   SPDX-FileCopyrightText: 2010 Tasuku Suzuki <stasuku@gmail.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "kpToolTextPrivate.h"
#include "tools/selection/text/kpToolText.h"

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

//---------------------------------------------------------------------

void kpToolText::inputMethodEvent(QInputMethodEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "kpToolText::inputMethodEvent() preeditString='" << e->preeditString() << "commitString = " << e->commitString()
                        << " replacementStart=" << e->replacementStart() << " replacementLength=" << e->replacementLength();
#endif
    kpTextSelection *textSel = document()->textSelection();
    if (hasBegunDraw() || !textSel) {
        e->ignore();
        return;
    }

    kpPreeditText previous = textSel->preeditText();
    kpPreeditText next(e);

    int textCursorRow = viewManager()->textCursorRow();
    int textCursorCol = viewManager()->textCursorCol();
    if (!next.isEmpty()) {
        if (previous.position().x() < 0 && previous.position().y() < 0) {
            next.setPosition(QPoint(textCursorCol, textCursorRow));
        } else {
            next.setPosition(previous.position());
        }
    }
    textSel->setPreeditText(next);
    textCursorCol = textCursorCol - previous.cursorPosition() + next.cursorPosition();
    viewManager()->setTextCursorPosition(textCursorRow, textCursorCol);

    QString commitString = e->commitString();
    if (!commitString.isEmpty()) {
        // commit string
        if (!d->insertCommand) {
            addNewInsertCommand(&d->insertCommand);
        }

        d->insertCommand->addText(commitString);
    }
    e->accept();
}

//---------------------------------------------------------------------
