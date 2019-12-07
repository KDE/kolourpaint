
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


#ifndef kpAbstractSelectionTool_H
#define kpAbstractSelectionTool_H


#include "tools/kpTool.h"

#include <QVariant>


class QKeyEvent;
class QPoint;
class QRect;

class kpAbstractSelection;
class kpAbstractSelectionContentCommand;
class kpCommand;
class kpToolSelectionEnvironment;


// The abstract base for all selection tools.
//
//
// This provides methods to:
//
//   1. handle selection commands and the command history.
//
//   2. implement the kpTool drawing methods.
//
//   3. implement the "Create", "Move" and "Resize/Scale" selection
//      draw types.
//
// "Drags" that consist of a single click generally have no effect in
// order to prevent accidentally creating a 1x1 selection, doing a NOP move
// or doing a NOP resize/scale.  However, doing a bigger drag and then
// dragging the mouse back to create a 1x1 selection or the NOP
// effects are allowed and are recorded in the command history.
//
// Additionally, the "Create" draw type is fitted with "accidental drag
// detection" which will not create a selection in response in small and
// quick drags.
//
//
// The internal architecture is as follows and is of interest for subclasses
// and for changing the existing implementation of the above selection
// draw types:
//
// beginDraw() initiates the action by determining the current draw type
// by calling the virtual calculateDrawType().  Later, all of this class'
// implementations of kpTool drawing methods (e.g. beginDraw(), draw(),
// endShape() etc.) call the virtual operation(), which is passed:
//
//   1. the current draw type (e.g. "Create" or "Move")
//
//   2. the operation, corresponding to the calling method (e.g.
//      "BeginDraw" of called by beginDraw()).
//
// [Note: the documentation in these source files sometimes uses "operation"
//        where "draw type" is meant and vice-versa]
//
// Note that these kpTool drawing methods do some other work before and after
// calling operation().
//
// The default implementation of operation() is to dispatch the operation
// to draw-type-specific methods e.g. createOperation() handles all
// operations for the "Create" draw type.  createOperation() will then
// call the method that corresponds to the operation e.g. beginDrawCreate()
// corresponds to the "BeginDraw" operation.
//
// For each draw type, all methods are grouped in a single source file
// e.g. kpAbstractSelectionTool_Create.cpp.
//
// To introduce a custom draw type, not implemented by code in this
// class (e.g. "SelectText"), you must:
//
//   1. Add it to "enum DrawType" below.
//
//   2. Override calculateDrawType() to determine the situations in which
//      the new draw type is active.
//
//   3. Override operation() to catch all situations in which the new draw
//      type is being used, and to implement the appropriate logic.
//
class kpAbstractSelectionTool : public kpTool
{
Q_OBJECT

public:
    kpAbstractSelectionTool (const QString &text, const QString &description,
        int key,
        kpToolSelectionEnvironment *environ, QObject *parent,
        const QString &name);

    ~kpAbstractSelectionTool () override;


    // Inform kpTool to call draw() when CTRL, SHIFT and friends are
    // pressed.  CTRL is used for copying, instead of moving, the
    // selection.  SHIFT is used for sweeping.
    bool careAboutModifierState () const override { return true; }


//
// Drawing - Subclass Accessors
//

protected:
    friend struct kpAbstractSelectionToolPrivate;
    enum DrawType
    {
        None, Create, Move, SelectText, ResizeScale
    };


    // The return value is not "None" during a drawing operation.
    //
    // The returned value is set by beginDraw(), after being determined
    // by calculateDrawType().  It is cleared in cancelShape() and endDraw().
    DrawType drawType () const;

    bool hadSelectionBeforeDraw () const;


//
// Drawing
//

protected:
    // (overrides non-virtual method in kpTool)
    kpToolSelectionEnvironment *environ () const;

    // Returns whether a CTRL or SHIFT key is currently pressed.
    // Convenience method.
    bool controlOrShiftPressed () const;


protected:
    // Deselects the current selection:
    //
    //   1. If it has no content, it is simply deleted.
    //   2. If it has content, it pushes it onto the document, adding the
    //      necessary commands to the command history.
    //
    // ASSUMPTIONS:
    //   1. There is a current selection.
    //   2. You have not called giveContentIfNeeded() nor
    //      addNeedingContentCommand() on the current selection.
    void pushOntoDocument ();


//
// The command lifecycle is as follows:
//
// 1. Ensure that the document has a selection, with or without content.
//
// 2. Call giveContentIfNeeded().
//
// 3. Create the command.
//
// 4. Process user input, mutate the selection directly and update the
//    command with the user's input.
//
// 5. When the drawing operation is complete, call addNeedingContentCommand()
//    with the command created in Step 3.
//
protected:
    // Returns a new instance of the give-the-selection-content command
    // that matches the current selection type.  The command should not be
    // executed by this method.
    virtual kpAbstractSelectionContentCommand *newGiveContentCommand () const = 0;

    // Before changing a selection (e.g. moving or resizing), you must
    // ensure that it has content.  Call this method to ensure that.
    //
    // If the selection has no content, this calls newGiveContentCommand()
    // and executes it.  If the selection already has content, this does
    // nothing.
    //
    // ASSUMPTION: There is a selection.
    void giveContentIfNeeded ();

    // The name that should be given to command that is constructed in
    // response to a drag that creates a new selection.
    virtual QString nameOfCreateCommand () const = 0;

    // Add a command to the command history.
    // The command is not executed.
    //
    // If the prior call to giveContentIfNeeded() created content, this
    // will, in line with KolourPaint selection convention:
    //
    //   1. Adds a selection border creation command (this is a bit clever
    //      and may overwrite the last "Undo" command instead -- see
    //      kpCommandHistory::addCreateSelectionCommand()).
    //
    //   2. Group the content command created by giveContentIfNeeded()
    //      with <cmd>, as a kpMacroCommand also named <cmd>.
    //
    // ASSUMPTION: giveContentIfNeeded() must have been called before
    //             creating <cmd>.
    void addNeedingContentCommand (kpCommand *cmd);


protected:
    // Sets the selection border mode when no drawing operation is active.
    //
    // Subclasses may wish to reimplement but should still call the base
    // implementation.  Reimplementations should wrap the whole
    // reimplementation in a kpViewManager::setQueueUpdates() block.
    virtual void setSelectionBorderForHaventBegunDraw ();
private:
    // Returns the statusbar message from when no draw operation is in
    // progress.  Calls operation() with "HaventBegunDrawUserMessage".
    //
    // (not const due to purely syntactic issue: it calls the non-const
    //  operation(); it really acts like a const method though)
    QString haventBegunDrawUserMessage ();


public:
    void begin () override;
    void end () override;


public:
    void reselect () override;


//
// Drawing - Beginning a Drag
//

protected:
    // Called by calculateDrawType() to determine what type of draw type
    // is being started in response to a drag inside the bounding rectangle of
    // a selection.
    //
    // This implementation returns "Move".
    //
    // You are free to reimplement this and may choose to call this base
    // implementation or not.
    virtual DrawType calculateDrawTypeInsideSelection () const;

    // Called by beginDraw() to determine what type of draw type is
    // being started.  The returned draw type is passed to operation().
    //
    // This implementation behaves according to the first rule that matches:
    //
    // 1. If the cursor is on top of a selection resize handle and no modifiers
    //    are held (i.e. not a smearing move draw type), it returns "ResizeScale".
    //
    // 2. If the cursor is inside the bounding rectangle of a selection, it
    //    calls calculateDrawTypeInsideSelection().
    //
    // 3. Otherwise, it returns "Create".
    //
    // You are free to reimplement this and may choose to call this base
    // implementation or not.  Reimplementing allows you to support new
    // draw types for different types of selections (e.g. kpToolText
    // supports "SelectText").  It also allows you to make certain
    // drags (e.g. dragging in the middle of a selection) do nothing by
    // returning "None" instead of calling the base implementation.
    virtual DrawType calculateDrawType () const;
public:
    void beginDraw () override;


//
// Drawing - Mouse Movement
//

public:
    void hover (const QPoint &point) override;
    void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                       const QRect &normalizedRect) override;


//
// Drawing - Ending a Drag
//

public:
    void cancelShape () override;
    void releasedAllButtons () override;


protected:
    // Displays the right-mouse-button-triggered selection menu, re-entering
    // the event loop and blocking until the menu closes.
    //
    // This menu is a subset of the main window's Edit and Selection menus.
    //
    // WARNING: This may cause a re-entry of view/tool event handlers.
    //          If you are calling this from a view/tool event handler,
    //          either make all your handlers re-entrant or do not put any
    //          code in your handler after the call.
    void popupRMBMenu ();
public:
    void endDraw (const QPoint &thisPoint, const QRect &normalizedRect) override;


//
// Drawing - Operation Dispatching
//

protected:
    enum Operation
    {
        //
        // These may be called outside of a drawing operation where
        // drawType() will return None.
        //

        // Returns the message for the given draw type and operation.
        HaventBegunDrawUserMessage,

        SetCursor,


        //
        // Called to start, to end, or inside, a drawing operation.
        //

        BeginDraw, Draw, Cancel, EndDraw
    };

    // (See the class API Doc for a description).
    virtual QVariant operation (DrawType drawType, Operation op,
        const QVariant &data1 = QVariant (), const QVariant &data2 = QVariant ());


//
// Create
//

private:
    // Called by constructor to initialize the "Create" draw type.
    void initCreate ();
    // Called by destructor to uninitialize the "Create" draw type.
    void uninitCreate ();


private:
    void beginCreate ();
    void endCreate ();


protected:
    virtual QString haventBegunDrawUserMessageCreate () const = 0;
private:
    void setCursorCreate ();


protected:
    // Sets the selection border mode when beginning to drag to create a
    // selection.
    //
    // Subclasses may wish to reimplement but should still call the base
    // implementation.  Reimplementations should wrap the whole
    // reimplementation in a kpViewManager::setQueueUpdates() block.
    virtual void setSelectionBorderForBeginDrawCreate ();
private:
    void beginDrawCreate ();


protected:
    //
    // If the drag has already been substantial enough to be considered as a
    // non-NOP drag (<dragAccepted>), you must return "true".
    //
    // If it has not, you should return whether you think the drag should
    // be started.  This criteria usually includes "if
    // <accidentalDragAdjustedPoint> is not equal to startPoint()".
    //
    // If you are returning true, you must:
    //
    //   1. Set the document's selection (which may not have previously
    //      existed) to the specified size.
    //
    //   2. Update the status bar by calling kpTool::setUserShapePoints().
    //
    // If you return false, you are still permitted to do the above,
    // although it would be unusual (kpToolText does the above to allow a
    // single click -- with no dragging -- to create a new text box).
    //
    // The return value will be fed into the next call as <dragAccepted>.
    //
    // Arguments:
    //
    //   1. <accidentalDragAdjustedPoint>:
    //      This is the same as currentPoint() but is set to startPoint()
    //      if the mouse has not been moved much (6 manhatten length pixels
    //      from startPoint() within a short period of time (200ms)).
    //      This provides the accidental drag detection, referred to in the
    //      class' API Doc.
    //
    //   2. <normalizedRect>:
    //      This is as passed to kpTool::draw().
    //
    virtual bool drawCreateMoreSelectionAndUpdateStatusBar (
        bool dragAccepted,
        const QPoint &accidentalDragAdjustedPoint,
        const QRect &normalizedRect) = 0;
    void drawCreate (const QPoint &thisPoint, const QRect &normalizedRect);
private slots:
    void delayedDrawCreate ();


private:
    void cancelCreate ();
    void endDrawCreate ();


private:
    QVariant operationCreate (Operation op,
        const QVariant &data1, const QVariant &data2);


//
// Move
//

private:
    // Called by constructor to initialize the "Move" draw type.
    void initMove ();
    // Called by destructor to uninitialize the "Move" draw type.
    void uninitMove ();


private:
    void beginMove ();
    void endMove ();


protected:
    virtual QString haventBegunDrawUserMessageMove () const = 0;
private:
    void setCursorMove ();


protected:
    // Sets the selection border mode when beginning to drag to move a
    // selection.
    //
    // Subclasses may wish to reimplement but should still call the base
    // implementation.  Reimplementations should wrap the whole
    // reimplementation in a kpViewManager::setQueueUpdates() block.
    virtual void setSelectionBorderForBeginDrawMove ();
private:
    void beginDrawMove ();
private slots:
    void slotRMBMoveUpdateGUI ();


private:
    void drawMove (const QPoint &thisPoint, const QRect &normalizedRect);


private:
    void cancelMove ();
protected:
    // Returns what the name of the command that moves -- but does not smear
    // (not holding SHIFT) -- the selection, should be.
    virtual QString nonSmearMoveCommandName () const;
private:
    void endDrawMove ();


private:
    QVariant operationMove (Operation op,
        const QVariant &data1, const QVariant &data2);


//
// Resize/Scale
//

private:
    int onSelectionResizeHandle () const;


private:
    // Called by constructor to initialize the "Resize/Scale" draw type.
    void initResizeScale ();
    // Called by destructor to uninitialize the "Resize/Scale" draw type.
    void uninitResizeScale ();


private:
    void beginResizeScale ();
    void endResizeScale ();


protected:
    virtual QString haventBegunDrawUserMessageResizeScale () const = 0;
private:
    void setCursorResizeScale ();


protected:
    // Sets the selection border mode when beginning to drag to resize or
    // scale a selection.
    //
    // Subclasses may wish to reimplement but should still call the base
    // implementation.  Reimplementations should wrap the whole
    // reimplementation in a kpViewManager::setQueueUpdates() block.
    virtual void setSelectionBorderForBeginDrawResizeScale ();
private:
    void beginDrawResizeScale ();


private:
    // drawResizeScaleCalculateNewSelectionPosSize() calls us with what the
    // <newWidth>x<newHeight> should be, but before any aspect maintenance
    // operations.
    //
    // <horizontalGripDragged> specifies whether a horizontal grip is being
    // dragged.  <verticalGripDragged> specifies whether a vertical grip is
    // being dragged.
    //
    // The selection before any resizing/scaling (before the sequence of
    // drags, where the mouse has been held down) is <originalSelection>.
    //
    // The method should output its attempt at maintaining the aspect ratio.
    // We say "attempt" because it is constrained by the minimum allowed
    // size of the selection.
    void drawResizeScaleTryKeepAspect (int newWidth, int newHeight,
        bool horizontalGripDragged, bool verticalGripDragged,
        const kpAbstractSelection &originalSelection,
        int *newWidthOut, int *newHeightOut);

    void drawResizeScaleCalculateNewSelectionPosSize (
        const kpAbstractSelection &originalSelection,
        int *newX, int *newY,
        int *newWidth, int *newHeight);

    void drawResizeScale (const QPoint &thisPoint, const QRect &normalizedRect);


private:
    void cancelResizeScale ();
    void endDrawResizeScale ();


private:
    QVariant operationResizeScale (Operation op,
        const QVariant &data1, const QVariant &data2);


//
// User Setting Selection Options
//

protected slots:
    virtual void slotIsOpaqueChanged (bool isOpaque) = 0;


//
// Keyboard Events
//

protected:
    // Reimplemented to trap Esc presses for deselecting the selection.
    // All other keypresses are passed to the base implementation.
    void keyPressEvent (QKeyEvent *e) override;


private:
    struct kpAbstractSelectionToolPrivate * const d;
};


#endif  // kpAbstractSelectionTool_H
