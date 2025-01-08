
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_CROP 0

#include "kpTransformCrop.h"
#include "kpTransformCropPrivate.h"

#include "commands/imagelib/effects/kpEffectClearCommand.h"
#include "commands/kpMacroCommand.h"
#include "commands/tools/selection/kpToolSelectionMoveCommand.h"
#include "mainWindow/kpMainWindow.h"

void kpTransformCrop_TextSelection(kpMainWindow *mainWindow, const QString &commandName, kpCommand *resizeDocCommand)
{
    kpCommandEnvironment *environ = mainWindow->commandEnvironment();

    auto *macroCmd = new kpMacroCommand(commandName, environ);

    macroCmd->addCommand(resizeDocCommand);

#if DEBUG_KP_TOOL_CROP
    qCDebug(kpLogImagelib) << "\tisText";
    qCDebug(kpLogImagelib) << "\tclearing doc with trans cmd";
#endif
    macroCmd->addCommand(new kpEffectClearCommand(false /*act on doc*/, kpColor::Transparent, environ));

#if DEBUG_KP_TOOL_CROP
    qCDebug(kpLogImagelib) << "\tmoving sel to (0,0) cmd";
#endif
    kpToolSelectionMoveCommand *moveCmd = new kpToolSelectionMoveCommand(QString() /*uninteresting child of macro cmd*/, environ);
    moveCmd->moveTo(QPoint(0, 0), true /*move on exec, not now*/);
    moveCmd->finalize();
    macroCmd->addCommand(moveCmd);

    mainWindow->addImageOrSelectionCommand(macroCmd, true /*add create cmd*/, true /*add create content cmd*/);
}
