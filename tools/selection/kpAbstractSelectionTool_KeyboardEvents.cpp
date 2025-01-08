
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "kpAbstractSelectionTool.h"
#include "kpAbstractSelectionToolPrivate.h"

#include "commands/tools/selection/kpToolSelectionMoveCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "layers/selections/kpAbstractSelection.h"

#include <QKeyEvent>

#include "kpLogCategories.h"

//---------------------------------------------------------------------

// protected virtual [base kpTool]
void kpAbstractSelectionTool::keyPressEvent(QKeyEvent *e)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::keyPressEvent(e->text='" << e->text() << "')";
#endif

    e->ignore();

    if (document()->selection() && !hasBegunDraw() && e->key() == Qt::Key_Escape) {
#if DEBUG_KP_TOOL_SELECTION && 0
        qCDebug(kpLogTools) << "\tescape pressed with sel when not begun draw - deselecting";
#endif

        pushOntoDocument();
        e->accept();
    } else {
#if DEBUG_KP_TOOL_SELECTION && 0
        qCDebug(kpLogTools) << "\tkey processing did not accept (text was '" << e->text() << "') - passing on event to kpTool";
#endif

        if (document()->selection() && !hasBegunDraw()
            && ((e->key() == Qt::Key_Left) || (e->key() == Qt::Key_Right) || (e->key() == Qt::Key_Up) || (e->key() == Qt::Key_Down))) {
            // move selection with cursor keys pixel-wise
            giveContentIfNeeded();

            if (!d->currentMoveCommand) {
                d->currentMoveCommand = new kpToolSelectionMoveCommand(QString() /*uninteresting child of macro cmd*/, environ()->commandEnvironment());
                d->currentMoveCommandIsSmear = false;
            }

            int dx, dy;
            arrowKeyPressDirection(e, &dx, &dy);
            d->currentMoveCommand->moveTo(document()->selection()->topLeft() + QPoint(dx, dy));
            endDrawMove();
        } else
            kpTool::keyPressEvent(e);
    }
}

//---------------------------------------------------------------------
