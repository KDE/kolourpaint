
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

#include <QCursor>
#include <QMenu>
#include <QTimer>

#include "kpLogCategories.h"
#include "layers/selections/kpAbstractSelection.h"
#include "commands/tools/selection/kpAbstractSelectionContentCommand.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "commands/kpMacroCommand.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "imagelib/kpPainter.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

// For either of these timers, they are only active during the "drawing" phase
// of kpTool.
static void AssertAllTimersInactive (struct kpAbstractSelectionToolPrivate *d)
{
    Q_ASSERT (!d->createNOPTimer->isActive ());
    Q_ASSERT (!d->RMBMoveUpdateGUITimer->isActive ());
}

//---------------------------------------------------------------------

kpAbstractSelectionTool::kpAbstractSelectionTool (
        const QString &text,
        const QString &description,
        int key,
        kpToolSelectionEnvironment *environ, QObject *parent,
        const QString &name)

    : kpTool (text, description, key, environ, parent, name),
      d (new kpAbstractSelectionToolPrivate ())
{
    d->drawType = None;
    d->currentSelContentCommand = nullptr;

    // d->dragAccepted
    // d->hadSelectionBeforeDrag

    // d->cancelledShapeButStillHoldingButtons

    d->toolWidgetOpaqueOrTransparent = nullptr;


    initCreate ();
    initMove ();
    initResizeScale ();

    // It would be bad practice to have timers ticking even when this tool
    // is not in use.
    ::AssertAllTimersInactive (d);
}

//---------------------------------------------------------------------

kpAbstractSelectionTool::~kpAbstractSelectionTool ()
{
    uninitCreate ();
    uninitMove ();
    uninitResizeScale ();


    // (state must be after construction, or after some time after end())
    Q_ASSERT (d->drawType == None);
    Q_ASSERT (!d->currentSelContentCommand);

    // d->dragAccepted
    // d->hadSelectionBeforeDraw

    // d->cancelledShapeButStillHoldingButtons

    // d->toolWidgetOpaqueOrTransparent


    delete d;
}

//---------------------------------------------------------------------

// protected
kpAbstractSelectionTool::DrawType kpAbstractSelectionTool::drawType () const
{
    return d->drawType;
}

//---------------------------------------------------------------------

// protected
bool kpAbstractSelectionTool::hadSelectionBeforeDraw () const
{
    return d->hadSelectionBeforeDraw;
}

//---------------------------------------------------------------------

// protected overrides [base kpTool]
kpToolSelectionEnvironment *kpAbstractSelectionTool::environ () const
{
    kpToolEnvironment *e = kpTool::environ ();
    Q_ASSERT (dynamic_cast <kpToolSelectionEnvironment *> (e));
    return dynamic_cast <kpToolSelectionEnvironment *> (e);
}

//---------------------------------------------------------------------

// protected
bool kpAbstractSelectionTool::controlOrShiftPressed () const
{
    return (controlPressed () || shiftPressed ());
}

//---------------------------------------------------------------------

// protected
void kpAbstractSelectionTool::pushOntoDocument ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::pushOntoDocument() selection="
              << document ()->selection ();
#endif
    Q_ASSERT (document ()->selection ());
    environ ()->deselectSelection ();
}

//---------------------------------------------------------------------

// protected
void kpAbstractSelectionTool::giveContentIfNeeded ()
{
    kpAbstractSelection *sel = document ()->selection ();
    Q_ASSERT (sel);

    if (sel->hasContent ()) {
        return;
    }

    if (d->currentSelContentCommand) {
        return;
    }

    d->currentSelContentCommand = /*virtual*/newGiveContentCommand ();
    d->currentSelContentCommand->execute ();
}

//---------------------------------------------------------------------

// protected
// REFACTOR: sync: Code dup with kpMainWindow::addImageOrSelectionCommand ().
void kpAbstractSelectionTool::addNeedingContentCommand (kpCommand *cmd)
{
    Q_ASSERT (cmd);

    // Did we fill the selection with content?
    if (d->currentSelContentCommand)
    {
        // Make the border creation a command.
    #if DEBUG_KP_TOOL_SELECTION
        qCDebug(kpLogTools) << "\thave currentSelContentCommand";
    #endif
        commandHistory ()->addCreateSelectionCommand (
            new kpToolSelectionCreateCommand (
                /*virtual*/nameOfCreateCommand (),
                *d->currentSelContentCommand->originalSelection (),
                environ ()->commandEnvironment ()),
            false/*no exec - user already dragged out sel*/);
    }

    // Do we have a content setting command we need to commit?
    // (yes, this is the same check as the previous "if")
    if (d->currentSelContentCommand)
    {
        // Put the content command + given command (e.g. movement) together
        // as a macro command, in the command history.
        kpMacroCommand *macroCmd = new kpMacroCommand (
            cmd->name (), environ ()->commandEnvironment ());

        macroCmd->addCommand (d->currentSelContentCommand);
        d->currentSelContentCommand = nullptr;

        macroCmd->addCommand (cmd);

        commandHistory ()->addCommand (macroCmd, false/*no exec*/);
    }
    else
    {
        // Put the given command into the command history.
        commandHistory ()->addCommand (cmd, false/*no exec*/);
    }
}

//---------------------------------------------------------------------


// protected virtual
void kpAbstractSelectionTool::setSelectionBorderForHaventBegunDraw ()
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
QString kpAbstractSelectionTool::haventBegunDrawUserMessage ()
{
#if DEBUG_KP_TOOL_SELECTION && 0
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::haventBegunDrawUserMessage()"
                  " cancelledShapeButStillHoldingButtons="
               << d->cancelledShapeButStillHoldingButtons;
#endif

    if (d->cancelledShapeButStillHoldingButtons) {
        return i18n ("Let go of all the mouse buttons.");
    }

    return operation (calculateDrawType (), HaventBegunDrawUserMessage).toString ();
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpAbstractSelectionTool::begin ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractSelectionTool<" << objectName () << ">::begin()";
#endif

    ::AssertAllTimersInactive (d);

    // (state must be after construction, or after some time after end())
    Q_ASSERT (d->drawType == None);
    Q_ASSERT (!d->currentSelContentCommand);

    d->dragAccepted = false;
    // d->hadSelectionBeforeDraw

    d->cancelledShapeButStillHoldingButtons = false;


    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

    d->toolWidgetOpaqueOrTransparent = tb->toolWidgetOpaqueOrTransparent ();
    Q_ASSERT (d->toolWidgetOpaqueOrTransparent);
    connect (d->toolWidgetOpaqueOrTransparent,
             &kpToolWidgetOpaqueOrTransparent::isOpaqueChanged,
             this, &kpAbstractSelectionTool::slotIsOpaqueChanged);
    d->toolWidgetOpaqueOrTransparent->show ();

    /*virtual*/setSelectionBorderForHaventBegunDraw ();


    beginCreate ();
    beginMove ();
    beginResizeScale ();


    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpAbstractSelectionTool::end ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractSelectionTool<" << objectName () << ">::end()";
#endif

    if (document ()->selection ()) {
        pushOntoDocument ();
    }


    endCreate ();
    endMove ();
    endResizeScale ();


    // (should have been killed by cancelShape() or endDraw())
    Q_ASSERT (d->drawType == None);
    Q_ASSERT (!d->currentSelContentCommand);

    // d->dragAccepted
    // d->hadSelectionBeforeDraw

    // d->cancelledShapeButStillHoldingButtons


    Q_ASSERT (d->toolWidgetOpaqueOrTransparent);
    disconnect (d->toolWidgetOpaqueOrTransparent,
             &kpToolWidgetOpaqueOrTransparent::isOpaqueChanged,
             this, &kpAbstractSelectionTool::slotIsOpaqueChanged);
    d->toolWidgetOpaqueOrTransparent = nullptr;


    viewManager ()->unsetCursor ();

    ::AssertAllTimersInactive (d);
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpAbstractSelectionTool::reselect ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::reselect()";
#endif

    if (document ()->selection ()) {
        pushOntoDocument ();
    }
}

//---------------------------------------------------------------------

// protected virtual
kpAbstractSelectionTool::DrawType kpAbstractSelectionTool::calculateDrawTypeInsideSelection () const
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\t\tis move";
#endif
    return kpAbstractSelectionTool::Move;
}

//---------------------------------------------------------------------

// protected virtual
kpAbstractSelectionTool::DrawType kpAbstractSelectionTool::calculateDrawType () const
{
    kpAbstractSelection *sel = document ()->selection ();
    if (!sel) {
        return Create;
    }
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "\thas sel region rect=" << sel->boundingRect ();
#endif

    if (onSelectionResizeHandle () && !controlOrShiftPressed ()) {
        return ResizeScale;
    }

    if (sel->contains (currentPoint ())) {
        return /*virtual*/calculateDrawTypeInsideSelection ();
    }

    return Create;
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpAbstractSelectionTool::beginDraw ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::beginDraw() startPoint ()="
               << startPoint ()
               << " QCursor::pos() view startPoint="
               << viewUnderStartPoint ()->mapFromGlobal (QCursor::pos ());
#endif

    // endDraw() and cancelShape() should have taken care of these.
    ::AssertAllTimersInactive (d);

    // In case the cursor was wrong to start with
    // (forgot to call kpTool::somethingBelowTheCursorChanged()),
    // make sure it is correct during this operation.
    hover (currentPoint ());

    // Currently used only to end the current text
    if (hasBegunShape ())
    {
        endShape(currentPoint(),
                 kpPainter::normalizedRect(startPoint()/* TODO: wrong */, currentPoint()));
    }

    d->drawType = calculateDrawType ();
    d->dragAccepted = false;

    kpAbstractSelection *sel = document ()->selection ();
    d->hadSelectionBeforeDraw = bool (sel);

    operation (d->drawType, BeginDraw);
}

//---------------------------------------------------------------------


// public virtual [base kpTool]
void kpAbstractSelectionTool::hover (const QPoint &point)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::hover" << point;
#endif

    operation (calculateDrawType (), SetCursor);

    setUserShapePoints (point, KP_INVALID_POINT, false/*don't set size*/);
    if (document () && document ()->selection ())
    {
        setUserShapeSize (document ()->selection ()->width (),
                          document ()->selection ()->height ());
    }
    else
    {
        setUserShapeSize (KP_INVALID_SIZE);
    }

    QString mess = haventBegunDrawUserMessage ();
    if (mess != userMessage ()) {
        setUserMessage (mess);
    }
}

//---------------------------------------------------------------------


// public virtual [base kpTool]
void kpAbstractSelectionTool::draw (const QPoint &thisPoint, const QPoint & /*lastPoint*/,
                                    const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::draw (" << thisPoint
               << ",startPoint=" << startPoint ()
               << ",normalizedRect=" << normalizedRect << ")";
#else
    Q_UNUSED (thisPoint);
    Q_UNUSED (normalizedRect);
#endif


    // OPT: return when thisPoint == lastPoint () so that e.g. when creating
    //      Points sel, press modifiers doesn't add multiple points in same
    //      place


    operation (d->drawType, Draw);
}

//---------------------------------------------------------------------


// public virtual [base kpTool]
void kpAbstractSelectionTool::cancelShape ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::cancelShape() mouseButton="
              << mouseButton ();
#endif

    const DrawType oldDrawType = d->drawType;
    // kpTool::hasBegunDraw() returns false in this method so be consistent
    // and clear "drawType" before dispatching the operation() below.
    d->drawType = None;


    viewManager ()->setQueueUpdates ();
    {
        operation (oldDrawType, Cancel);


        if (d->currentSelContentCommand)
        {
        #if DEBUG_KP_TOOL_SELECTION
            qCDebug(kpLogTools) << "\t\tundo sel content";
        #endif
            d->currentSelContentCommand->unexecute ();
            delete d->currentSelContentCommand;
            d->currentSelContentCommand = nullptr;
        }


        /*virtual*/setSelectionBorderForHaventBegunDraw ();
    }
    viewManager ()->restoreQueueUpdates ();


    d->cancelledShapeButStillHoldingButtons = true;
    setUserMessage (i18n ("Let go of all the mouse buttons."));


    ::AssertAllTimersInactive (d);
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpAbstractSelectionTool::releasedAllButtons ()
{
    d->cancelledShapeButStillHoldingButtons = false;
    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// protected
void kpAbstractSelectionTool::popupRMBMenu ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "CALL - exec'ing menu";
#endif

    QMenu *pop = environ ()->selectionToolRMBMenu ();
    Q_ASSERT (pop);

    // Blocks until the menu closes.
    // WARNING: Enters event loop - may re-enter view/tool event handlers.
    pop->exec (QCursor::pos ());
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "calling somethingBelowTheCursorChanged()";
#endif

    // Cursor may have moved while the menu was up, triggering QMouseMoveEvents
    // for the menu -- but not the view -- so we may have missed cursor moves.
    // Update cursor position now.
    somethingBelowTheCursorChanged ();
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "DONE";
#endif
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpAbstractSelectionTool::endDraw (const QPoint & /*thisPoint*/,
        const QRect & /*normalizedRect*/)
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::endDraw()";
#endif

    const DrawType oldDrawType = d->drawType;
    // kpTool::hasBegunDraw() returns false in this method so be consistent
    // and clear "drawType" before dispatching the operation() below.
    d->drawType = None;


    viewManager ()->setQueueUpdates ();
    {
        operation (oldDrawType, EndDraw);

        /*virtual*/setSelectionBorderForHaventBegunDraw ();
    }
    viewManager ()->restoreQueueUpdates ();


    setUserMessage (haventBegunDrawUserMessage ());


    ::AssertAllTimersInactive (d);


    if (mouseButton () == 1/*right*/) {
        popupRMBMenu ();
    }


    // WARNING: Do not place any code after the popupRMBMenu() call
    //          (see the popupRMBMenu() API).
}

//---------------------------------------------------------------------

// protected virtual
QVariant kpAbstractSelectionTool::operation (DrawType drawType, Operation op,
        const QVariant &data1, const QVariant &data2)
{
    switch (drawType)
    {
    case None:
        // NOP.
        return {};

    case Create:
        return operationCreate (op, data1, data2);

    case Move:
        return operationMove (op, data1, data2);

    case ResizeScale:
        return operationResizeScale (op, data1, data2);

    default:
        Q_ASSERT (!"Unhandled draw type");
        return {};
    }
}

//---------------------------------------------------------------------



