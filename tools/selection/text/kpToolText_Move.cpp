
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextPrivate.h"
#include "tools/selection/text/kpToolText.h"

#include <KLocalizedString>

#include "views/manager/kpViewManager.h"

// protected virtual [kpAbstractSelectionTool]
QString kpToolText::haventBegunDrawUserMessageMove() const
{
    return i18n("Left drag to move text box.");
}

// protected virtual [base kpAbstractSelectionTool]
void kpToolText::setSelectionBorderForBeginDrawMove()
{
    viewManager()->setQueueUpdates();
    {
        kpAbstractSelectionTool::setSelectionBorderForBeginDrawMove();
        viewManager()->setTextCursorEnabled(false);
    }
    viewManager()->restoreQueueUpdates();
}

// protected virtual [kpAbstractSelectionTool]
QString kpToolText::nonSmearMoveCommandName() const
{
    return i18n("Text: Move Box");
}
