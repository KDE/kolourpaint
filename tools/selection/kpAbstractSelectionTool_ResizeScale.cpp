
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

#include <KLocalizedString>

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


// private
int kpAbstractSelectionTool::onSelectionResizeHandle () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v) {
        return 0;
    }

    return v->mouseOnSelectionResizeHandle (currentViewPoint ());
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::initResizeScale ()
{
    d->currentResizeScaleCommand = nullptr;

   // d->resizeScaleType
}

// private
void kpAbstractSelectionTool::uninitResizeScale ()
{
    // (state must be after construction, or after some time after endResizeScale())
    Q_ASSERT (!d->currentResizeScaleCommand);

    // d->resizeScaleType
}


// private
void kpAbstractSelectionTool::beginResizeScale ()
{
    // (state must be after construction, or after some time after endResizeScale())
    Q_ASSERT (!d->currentResizeScaleCommand);

    // d->resizeScaleType
}

// private
void kpAbstractSelectionTool::endResizeScale ()
{
    // (should have been killed by cancelResizeScale() or endResizeScale())
    Q_ASSERT (!d->currentResizeScaleCommand);

    // d->resizeScaleType
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::setCursorResizeScale ()
{
#if DEBUG_KP_TOOL_SELECTION && 0
    qCDebug(kpLogTools) << "\tonSelectionResizeHandle="
                << onSelectionResizeHandle ();
#endif
    Qt::CursorShape shape = Qt::ArrowCursor;

    switch (onSelectionResizeHandle ())
    {
    case (kpView::Top | kpView::Left):
    case (kpView::Bottom | kpView::Right):
        shape = Qt::SizeFDiagCursor;
        break;

    case (kpView::Bottom | kpView::Left):
    case (kpView::Top | kpView::Right):
        shape = Qt::SizeBDiagCursor;
        break;

    case kpView::Top:
    case kpView::Bottom:
        shape = Qt::SizeVerCursor;
        break;

    case kpView::Left:
    case kpView::Right:
        shape = Qt::SizeHorCursor;
        break;
    }

    viewManager ()->setCursor (shape);
}

//---------------------------------------------------------------------

// protected virtual
void kpAbstractSelectionTool::setSelectionBorderForBeginDrawResizeScale ()
{
    viewManager ()->setQueueUpdates ();
    {
        viewManager ()->setSelectionBorderVisible (true);
        viewManager ()->setSelectionBorderFinished (true);
    }
    viewManager ()->restoreQueueUpdates ();
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::beginDrawResizeScale ()
{
    d->resizeScaleType = onSelectionResizeHandle ();

    /*virtual*/setSelectionBorderForBeginDrawResizeScale ();

    setUserMessage (cancelUserMessage ());
}

//---------------------------------------------------------------------


// private
void kpAbstractSelectionTool::drawResizeScaleTryKeepAspect (
        int newWidth, int newHeight,
        bool horizontalGripDragged, bool verticalGripDragged,
        const kpAbstractSelection &originalSelection,
        int *newWidthOut, int *newHeightOut)
{
    const int oldWidth = originalSelection.width (),
        oldHeight = originalSelection.height ();

    // Width changed more than height?  At equality, favor width.
    // Fix width, change height.
    //
    // We use <horizontalGripDragged> and <verticalGripDragged> to prevent
    // e.g. the situation where we've dragged such that newWidth < oldWidth but
    // we're not dragging a vertical grip.  We certainly don't want this
    // code to modify the width - we want to fix the width and change the
    // height.
    if ((horizontalGripDragged ? double (newWidth) / oldWidth : 0) >=
        (verticalGripDragged ? double (newHeight) / oldHeight : 0))
    {
        *newHeightOut = newWidth * oldHeight / oldWidth;
        *newHeightOut = qMax (originalSelection.minimumHeight (), *newHeightOut);
    }
    // Height changed more than width?
    // Fix height, change width.
    else
    {
        *newWidthOut = newHeight * oldWidth / oldHeight;
        *newWidthOut = qMax (originalSelection.minimumWidth (), *newWidthOut);
    }
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::drawResizeScaleCalculateNewSelectionPosSize (
        const kpAbstractSelection &originalSelection,
        int *newX, int *newY,
        int *newWidth, int *newHeight)
{
    //
    // Determine new width.
    //

    // Dragging left or right grip?
    // If left, positive X drags decrease width.
    // If right, positive X drags increase width.
    int userXSign = 0;
    if (d->resizeScaleType & kpView::Left) {
        userXSign = -1;
    }
    else if (d->resizeScaleType & kpView::Right) {
        userXSign = +1;
    }

    // Calculate new width.
    *newWidth = originalSelection.width () +
        userXSign * (currentPoint ().x () - startPoint ().x ());

    // Don't allow new width to be less than that kind of selection type's
    // minimum.
    *newWidth = qMax (originalSelection.minimumWidth (), *newWidth);


    //
    // Determine new height.
    //

    // Dragging top or bottom grip?
    // If top, positive Y drags decrease height.
    // If bottom, positive Y drags increase height.
    int userYSign = 0;
    if (d->resizeScaleType & kpView::Top) {
        userYSign = -1;
    }
    else if (d->resizeScaleType & kpView::Bottom) {
        userYSign = +1;
    }

    // Calculate new height.
    *newHeight = originalSelection.height () +
        userYSign * (currentPoint ().y () - startPoint ().y ());

    // Don't allow new height to be less than that kind of selection type's
    // minimum.
    *newHeight = qMax (originalSelection.minimumHeight (), *newHeight);


    // Keep aspect ratio?
    if (shiftPressed ())
    {
        drawResizeScaleTryKeepAspect (*newWidth, *newHeight,
            (userXSign != 0)/*X or XY grip dragged*/,
                (userYSign != 0)/*Y or XY grip dragged*/,
            originalSelection,
            newWidth/*ptr*/, newHeight/*ptr*/);
    }


    *newX = originalSelection.x ();
    *newY = originalSelection.y ();


    //
    // Adjust x/y to new width/height for left/top resizes.
    //

    if (d->resizeScaleType & kpView::Left)
    {
        *newX -= (*newWidth - originalSelection.width ());
    }

    if (d->resizeScaleType & kpView::Top)
    {
        *newY -= (*newHeight - originalSelection.height ());
    }

#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "\t\tnewX=" << *newX
                << " newY=" << *newY
                << " newWidth=" << *newWidth
                << " newHeight=" << *newHeight;
#endif
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::drawResizeScale (
        const QPoint &thisPoint,
        const QRect &/*normalizedRect*/)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "\tresize/scale";
#endif

    kpAbstractSelection *sel = document ()->selection ();

    if (!d->dragAccepted && thisPoint == startPoint ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        qCDebug(kpLogTools) << "\t\tnop";
    #endif

        setUserShapePoints (QPoint (sel->width (), sel->height ()));
        return;
    }


    giveContentIfNeeded ();


    if (!d->currentResizeScaleCommand)
    {
        d->currentResizeScaleCommand
            = new kpToolSelectionResizeScaleCommand (environ ()->commandEnvironment ());
    }


    const kpAbstractSelection *originalSelection =
        d->currentResizeScaleCommand->originalSelection ();


    // There is nothing illegal about position (-1,-1) but why not.
    int newX = -1, newY = -1,
        newWidth = 0, newHeight = 0;

    // This should change all of the above values.
    drawResizeScaleCalculateNewSelectionPosSize (
        *originalSelection,
        &newX, &newY,
        &newWidth, &newHeight);


    viewManager ()->setFastUpdates ();
    {
        d->currentResizeScaleCommand->resizeAndMoveTo (
            newWidth, newHeight,
            QPoint (newX, newY),
            true/*smooth scale delayed*/);
    }
    viewManager ()->restoreFastUpdates ();

    setUserShapePoints (QPoint (originalSelection->width (),
                                originalSelection->height ()),
                        QPoint (newWidth,
                                newHeight),
                        false/*don't set size*/);
    setUserShapeSize (newWidth - originalSelection->width (),
                        newHeight - originalSelection->height ());


    d->dragAccepted = true;
}

//---------------------------------------------------------------------


// private
void kpAbstractSelectionTool::cancelResizeScale ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\twas resize/scale sel - kill";
#endif

    // NOP drag?
    if (!d->currentResizeScaleCommand) {
        return;
    }

#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\t\tundo currentResizeScaleCommand";
#endif
    d->currentResizeScaleCommand->finalize ();  // (unneeded but let's be safe)
    d->currentResizeScaleCommand->unexecute ();
    delete d->currentResizeScaleCommand;
    d->currentResizeScaleCommand = nullptr;
}

//---------------------------------------------------------------------

// private
void kpAbstractSelectionTool::endDrawResizeScale ()
{
    // NOP drag?
    if (!d->currentResizeScaleCommand) {
        return;
    }

    d->currentResizeScaleCommand->finalize ();

    addNeedingContentCommand (d->currentResizeScaleCommand);
    d->currentResizeScaleCommand = nullptr;
}

//---------------------------------------------------------------------

// private
QVariant kpAbstractSelectionTool::operationResizeScale (Operation op,
        const QVariant &data1, const QVariant &data2)
{
    (void) data1;
    (void) data2;


    switch (op)
    {
    case HaventBegunDrawUserMessage:
        return /*virtual*/haventBegunDrawUserMessageResizeScale ();

    case SetCursor:
        setCursorResizeScale ();
        break;

    case BeginDraw:
        beginDrawResizeScale ();
        break;

    case Draw:
        drawResizeScale (currentPoint (), normalizedRect ());
        break;

    case Cancel:
        cancelResizeScale ();
        break;

    case EndDraw:
        endDrawResizeScale ();
        break;

    default:
        Q_ASSERT (!"Unhandled operation");
        break;
    }


    return {};
}

//---------------------------------------------------------------------
