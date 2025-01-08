
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_ERASER 0

#include "kpToolEraser.h"

#include "commands/imagelib/effects/kpEffectClearCommand.h"
#include "commands/kpCommandHistory.h"
#include "environments/tools/kpToolEnvironment.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpToolEraser::kpToolEraser(kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowPixmapBase(i18n("Eraser"), i18n("Lets you rub out mistakes"), Qt::Key_A, environ, parent, QStringLiteral("tool_eraser"))
{
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolEraser::globalDraw()
{
#if DEBUG_KP_TOOL_ERASER
    qCDebug(kpLogTools) << "kpToolEraser::globalDraw()";
#endif

    commandHistory()->addCommand(new kpEffectClearCommand(false /*act on doc, not sel*/, backgroundColor(), environ()->commandEnvironment()));
}

//---------------------------------------------------------------------

// protected virtual [base kpToolFlowBase]
QString kpToolEraser::haventBegunDrawUserMessage() const
{
    return i18n("Click or drag to erase.");
}

//---------------------------------------------------------------------

// See our corresponding .h for brush selection.

// Logic is in kpToolFlowPixmapBase.

#include "moc_kpToolEraser.cpp"
