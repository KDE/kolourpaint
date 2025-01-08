
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
QString kpToolText::haventBegunDrawUserMessageResizeScale() const
{
    return i18n("Left drag to resize text box.");
}

// protected virtual [base kpAbstractSelectionTool]
void kpToolText::setSelectionBorderForBeginDrawResizeScale()
{
    viewManager()->setQueueUpdates();
    {
        kpAbstractSelectionTool::setSelectionBorderForBeginDrawResizeScale();
        viewManager()->setTextCursorEnabled(false);
    }
    viewManager()->restoreQueueUpdates();
}
