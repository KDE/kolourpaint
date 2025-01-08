
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "kpAbstractSelectionTool.h"
#include "kpAbstractSelectionToolPrivate.h"

#include <QCursor>
#include <QPixmap>
#include <QTimer>

#include <KLocalizedString>

#include "commands/kpCommandHistory.h"
#include "commands/kpMacroCommand.h"
#include "commands/tools/selection/kpToolImageSelectionTransparencyCommand.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "commands/tools/selection/kpToolSelectionDestroyCommand.h"
#include "commands/tools/selection/kpToolSelectionMoveCommand.h"
#include "commands/tools/selection/kpToolSelectionResizeScaleCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

// private
void kpAbstractSelectionTool::initCreate()
{
    d->createNOPTimer = new QTimer(this);
    d->createNOPTimer->setSingleShot(true);
    connect(d->createNOPTimer, &QTimer::timeout, this, &kpAbstractSelectionTool::delayedDrawCreate);
}

// private
void kpAbstractSelectionTool::uninitCreate()
{
    // d->createNOPTimer (deleted by QObject mechanism)
}

// private
void kpAbstractSelectionTool::beginCreate()
{
    // d->createNOPTimer
}

// private
void kpAbstractSelectionTool::endCreate()
{
    // d->createNOPTimer
}

//---------------------------------------------------------------------
// use a crosshair cursor which is really always exactly 1 pixel wide
// to the contrary of the "themed" crosshair cursors which might look nice
// but does not allow to exactly position the hot-spot.
/* XPM */
static const char *crosshair[] = {"17 17 3 1",         ". c None",          "x c #FFFFFF",       "# c #000000",       ".......xxx.......", ".......x#x.......",
                                  ".......x#x.......", ".......x#x.......", ".......x#x.......", ".......x#x.......", ".......x#x.......", "xxxxxxxx#xxxxxxxx",
                                  "x#######.#######x", "xxxxxxxx#xxxxxxxx", ".......x#x.......", ".......x#x.......", ".......x#x.......", ".......x#x.......",
                                  ".......x#x.......", ".......x#x.......", ".......xxx......."};

// private
void kpAbstractSelectionTool::setCursorCreate()
{
    viewManager()->setCursor(QCursor(QPixmap(crosshair), 8, 8));
}

//---------------------------------------------------------------------

// protected virtual
void kpAbstractSelectionTool::setSelectionBorderForBeginDrawCreate()
{
    viewManager()->setQueueUpdates();
    {
        // LOREFACTOR: I suspect some calls to viewManager() in this
        //             file (including this) are redundant since any
        //             code that tweaks such settings, returns them to
        //             their original state, after the code is complete.
        viewManager()->setSelectionBorderVisible(true);

        viewManager()->setSelectionBorderFinished(false);
    }
    viewManager()->restoreQueueUpdates();
}

// private
void kpAbstractSelectionTool::beginDrawCreate()
{
    if (document()->selection()) {
        pushOntoDocument();
    }

    /*virtual*/ setSelectionBorderForBeginDrawCreate();

    // (single shot)
    d->createNOPTimer->start(200 /*ms*/);

    setUserMessage(cancelUserMessage());
}

// private
void kpAbstractSelectionTool::drawCreate(const QPoint &thisPoint, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "\tnot moving - resizing rect to" << normalizedRect;
    qCDebug(kpLogTools) << "\t\tcreateNOPTimer->isActive()=" << d->createNOPTimer->isActive()
                        << " viewManhattanLength from startPoint=" << viewUnderStartPoint()->transformDocToViewX((thisPoint - startPoint()).manhattanLength());
#endif

    QPoint accidentalDragAdjustedPoint = thisPoint;
    if (d->createNOPTimer->isActive()) {
        // See below "d->createNOPTimer->stop()".
        Q_ASSERT(!d->dragAccepted);

        if (viewUnderStartPoint()->transformDocToViewX((accidentalDragAdjustedPoint - startPoint()).manhattanLength()) <= 6) {
#if DEBUG_KP_TOOL_SELECTION && 1
            qCDebug(kpLogTools) << "\t\tsuppress accidental movement";
#endif
            accidentalDragAdjustedPoint = startPoint();
        } else {
#if DEBUG_KP_TOOL_SELECTION && 1
            qCDebug(kpLogTools) << "\t\tit's a \"big\" intended move - stop timer";
#endif
            d->createNOPTimer->stop();
        }
    }

    const bool hadSelection = document()->selection();

    const bool oldDrawAcceptedAsDrag = d->dragAccepted;
    d->dragAccepted = /*virtual*/ drawCreateMoreSelectionAndUpdateStatusBar(d->dragAccepted, accidentalDragAdjustedPoint, normalizedRect);
    if (oldDrawAcceptedAsDrag) {
        Q_ASSERT(d->dragAccepted);
    }
    if (d->dragAccepted) {
#if DEBUG_KP_TOOL_SELECTION && 1
        qCDebug(kpLogTools) << "\t\tdrawHasDoneSomething - kill create timer";
#endif
        // No longer a NOP.
        d->createNOPTimer->stop();
    }

    // Did we just create a selection?
    if (!hadSelection && document()->selection()) {
        viewManager()->setSelectionBorderVisible(true);
    }
}

// private slot
void kpAbstractSelectionTool::delayedDrawCreate()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::delayedDrawCreate() hasBegunDraw=" << hasBegunDraw() << " currentPoint=" << currentPoint()
                        << " lastPoint=" << lastPoint() << " startPoint=" << startPoint();
#endif

    // (just in case not called from single shot)
    d->createNOPTimer->stop();

    if (hasBegunDraw()) {
        draw(currentPoint(), lastPoint(), normalizedRect());
    }
}

// private
void kpAbstractSelectionTool::cancelCreate()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\twas creating sel - kill";
#endif

    d->createNOPTimer->stop();

    // TODO: should we give the user back the selection s/he had before (if any)?
    if (document()->selection()) {
        document()->selectionDelete();
    }
}

// private
void kpAbstractSelectionTool::endDrawCreate()
{
    d->createNOPTimer->stop();
}

// private
QVariant kpAbstractSelectionTool::operationCreate(Operation op, const QVariant &data1, const QVariant &data2)
{
    (void)data1;
    (void)data2;

    switch (op) {
    case HaventBegunDrawUserMessage:
        return /*virtual*/ haventBegunDrawUserMessageCreate();

    case SetCursor:
        setCursorCreate();
        break;

    case BeginDraw:
        beginDrawCreate();
        break;

    case Draw:
        drawCreate(currentPoint(), normalizedRect());
        break;

    case Cancel:
        cancelCreate();
        break;

    case EndDraw:
        endDrawCreate();
        break;

    default:
        Q_ASSERT(!"Unhandled operation");
        break;
    }

    return {};
}
