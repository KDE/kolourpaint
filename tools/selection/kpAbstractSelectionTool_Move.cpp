
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DEBUG_KP_TOOL_SELECTION 0


#include "kpAbstractSelectionTool.h"
#include "kpAbstractSelectionToolPrivate.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "commands/kpMacroCommand.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "commands/tools/selection/kpToolSelectionDestroyCommand.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/kpToolSelectionMoveCommand.h"
#include "commands/tools/selection/kpToolSelectionResizeScaleCommand.h"
#include "commands/tools/selection/kpToolImageSelectionTransparencyCommand.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

#include <QTimer>

#include <KLocalizedString>

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::initMove ()
{
    d->currentMoveCommand = nullptr;

    // d->currentMoveCommandIsSmear

    // d->startMoveDragFromSelectionTopLeft

    d->RMBMoveUpdateGUITimer = new QTimer (this);
    d->RMBMoveUpdateGUITimer->setSingleShot (true);
    connect (d->RMBMoveUpdateGUITimer, &QTimer::timeout,
             this, &kpAbstractSelectionTool::slotRMBMoveUpdateGUI);
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::uninitMove ()
{
    // (state must be after construction, or after some time after endMove())
    Q_ASSERT (!d->currentMoveCommand);

    // d->currentMoveCommandIsSmear

    // d->startMoveDragFromSelectionTopLeft

    // d->RMBMoveUpdateGUITimer (deleted by QObject mechanism)
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::beginMove ()
{
    // (state must be after construction, or after some time after endMove())
    Q_ASSERT (!d->currentMoveCommand);

    // d->currentMoveCommandIsSmear

    // d->startMoveDragFromSelectionTopLeft;

    // d->RMBMoveUpdateGUITimer
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::endMove ()
{
    // (should have been killed by cancelMove() or endDrawMove())
    Q_ASSERT (!d->currentMoveCommand);

    // d->currentMoveCommandIsSmear

    // d->startMoveDragFromSelectionTopLeft

    // d->RMBMoveUpdateGUITimer
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::setCursorMove ()
{
    viewManager ()->setCursor (Qt::SizeAllCursor);
}

//---------------------------------------------------------------------

// protected virtual
void kpAbstractSelectionTool::setSelectionBorderForBeginDrawMove ()
{
    // don't show border while moving
    viewManager ()->setQueueUpdates ();
    {
        viewManager ()->setSelectionBorderVisible (false);
        viewManager ()->setSelectionBorderFinished (true);
    }
    viewManager ()->restoreQueueUpdates ();
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::beginDrawMove ()
{
    d->startMoveDragFromSelectionTopLeft =
        currentPoint () - document ()->selection ()->topLeft ();

    if (mouseButton () == 0)
    {
        /*virtual*/setSelectionBorderForBeginDrawMove ();
    }
    else
    {
        // Don't hide sel border momentarily if user is just
        // right _clicking_ selection.
        // (single shot timer)
        d->RMBMoveUpdateGUITimer->start (100/*ms*/);
    }

    setUserMessage (cancelUserMessage ());
}

//---------------------------------------------------------------------

// private slot
void kpAbstractSelectionTool::slotRMBMoveUpdateGUI ()
{
    // (just in case not called from single shot)
    d->RMBMoveUpdateGUITimer->stop ();

    /*virtual*/setSelectionBorderForBeginDrawMove ();

    kpAbstractSelection * const sel = document ()->selection ();
    if (sel) {
        setUserShapePoints (sel->topLeft ());
    }
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::drawMove (const QPoint &thisPoint, const QRect &/*normalizedRect*/)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "\tmoving selection";
#endif

    kpAbstractSelection *sel = document ()->selection ();

    QRect targetSelRect (thisPoint.x () - d->startMoveDragFromSelectionTopLeft.x (),
        thisPoint.y () - d->startMoveDragFromSelectionTopLeft.y (),
        sel->width (),
        sel->height ());

#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "\t\tstartPoint=" << startPoint ()
                << " thisPoint=" << thisPoint
                << " startDragFromSel=" << d->startMoveDragFromSelectionTopLeft
                << " targetSelRect=" << targetSelRect;
#endif

    // Try to make sure selection still intersects document so that it's
    // reachable.

    if (targetSelRect.right () < 0) {
        targetSelRect.translate (-targetSelRect.right (), 0);
    }
    else if (targetSelRect.left () >= document ()->width ()) {
        targetSelRect.translate (document ()->width () - targetSelRect.left () - 1, 0);
    }

    if (targetSelRect.bottom () < 0) {
        targetSelRect.translate (0, -targetSelRect.bottom ());
    }
    else if (targetSelRect.top () >= document ()->height ()) {
        targetSelRect.translate (0, document ()->height () - targetSelRect.top () - 1);
    }

#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "\t\t\tafter ensure sel rect clickable=" << targetSelRect;
#endif


    if (!d->dragAccepted &&
        targetSelRect.topLeft () + d->startMoveDragFromSelectionTopLeft == startPoint ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        qCDebug(kpLogTools) << "\t\t\t\tnop";
    #endif


        if (!d->RMBMoveUpdateGUITimer->isActive ())
        {
            // (slotRMBMoveUpdateGUI() calls similar line)
            setUserShapePoints (sel->topLeft ());
        }

        // Prevent both NOP drag-moves
        return;
    }


    if (d->RMBMoveUpdateGUITimer->isActive ())
    {
        d->RMBMoveUpdateGUITimer->stop ();
        slotRMBMoveUpdateGUI ();
    }


    giveContentIfNeeded ();


    if (!d->currentMoveCommand)
    {
        d->currentMoveCommand = new kpToolSelectionMoveCommand (
            QString()/*uninteresting child of macro cmd*/,
            environ ()->commandEnvironment ());
        d->currentMoveCommandIsSmear = false;
    }


    //viewManager ()->setQueueUpdates ();
    //viewManager ()->setFastUpdates ();

    if (shiftPressed ()) {
        d->currentMoveCommandIsSmear = true;
    }

    if (!d->dragAccepted && (controlPressed () || shiftPressed ())) {
        d->currentMoveCommand->copyOntoDocument ();
    }

    d->currentMoveCommand->moveTo (targetSelRect.topLeft ());

    if (shiftPressed ()) {
        d->currentMoveCommand->copyOntoDocument ();
    }

    //viewManager ()->restoreFastUpdates ();
    //viewManager ()->restoreQueueUpdates ();

    // REFACTOR: yuck, yuck
    kpAbstractSelection *orgSel = d->currentMoveCommand->originalSelectionClone ();
    QPoint start = orgSel->topLeft ();
    delete orgSel;
    QPoint end = targetSelRect.topLeft ();
    setUserShapePoints (start, end, false/*don't set size*/);
    setUserShapeSize (end.x () - start.x (), end.y () - start.y ());


    d->dragAccepted = true;
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::cancelMove ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\twas drag moving - undo drag and undo acquire";
#endif

    d->RMBMoveUpdateGUITimer->stop ();

    // NOP drag?
    if (!d->currentMoveCommand) {
        return;
    }

#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\t\tundo currentMoveCommand";
#endif
    d->currentMoveCommand->finalize ();
    d->currentMoveCommand->unexecute ();
    delete d->currentMoveCommand;
    d->currentMoveCommand = nullptr;
}

//---------------------------------------------------------------------

// protected virtual
QString kpAbstractSelectionTool::nonSmearMoveCommandName () const
{
    return i18n ("Selection: Move");
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::endDrawMove ()
{
    d->RMBMoveUpdateGUITimer->stop ();

    // NOP drag?
    if (!d->currentMoveCommand) {
        return;
    }

    d->currentMoveCommand->finalize ();

    kpMacroCommand *renamedCmd = nullptr;
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\thave moveCommand";
#endif
    if (d->currentMoveCommandIsSmear)
    {
        renamedCmd = new kpMacroCommand (i18n ("%1: Smear",
            document ()->selection ()->name ()),
            environ ()->commandEnvironment ());
    }
    else
    {
        renamedCmd = new kpMacroCommand (
            /*virtual*/nonSmearMoveCommandName (),
            environ ()->commandEnvironment ());
    }

    renamedCmd->addCommand (d->currentMoveCommand);
    d->currentMoveCommand = nullptr;

    addNeedingContentCommand (renamedCmd);
}

//---------------------------------------------------------------------

// private
QVariant kpAbstractSelectionTool::operationMove (Operation op,
        const QVariant &data1, const QVariant &data2)
{
    (void) data1;
    (void) data2;


    switch (op)
    {
    case HaventBegunDrawUserMessage:
        return /*virtual*/haventBegunDrawUserMessageMove ();

    case SetCursor:
        setCursorMove ();
        break;

    case BeginDraw:
        beginDrawMove ();
        break;

    case Draw:
        drawMove (currentPoint (), normalizedRect ());
        break;

    case Cancel:
        cancelMove ();
        break;

    case EndDraw:
        endDrawMove ();
        break;

    default:
        Q_ASSERT (!"Unhandled operation");
        break;
    }

    return {};
}

//---------------------------------------------------------------------
