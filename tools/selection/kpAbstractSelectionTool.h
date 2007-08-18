
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


#include <kpTool.h>

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
class kpAbstractSelectionTool : public kpTool
{
Q_OBJECT

public:
    kpAbstractSelectionTool (const QString &text, const QString &description,
        int key,
        kpToolSelectionEnvironment *environ, QObject *parent,
        const QString &name);
    virtual ~kpAbstractSelectionTool ();


    virtual bool careAboutModifierState () const { return true; }


//
// Drawing - Subclass Accessors
//

protected:
    friend struct kpAbstractSelectionToolPrivate;
    enum DrawType
    {
        None, Create, Move, SelectText, ResizeScale
    };


    DrawType drawType () const;
    bool hadSelectionBeforeDraw () const;
    

//
// Drawing
//

protected:
    // (overrides non-virtual method in kpTool)
    kpToolSelectionEnvironment *environ () const;

    // Returns whether a control or shift key is currently pressed.
    // Convenience method.
    bool controlOrShiftPressed () const;


protected:
    void pushOntoDocument ();


protected:
    virtual kpAbstractSelectionContentCommand *newGiveContentCommand () const = 0;

    // 
    // ASSUMPTION: There is a selection.
    void giveContentIfNeeded ();

    virtual QString nameOfCreateCommand () const = 0;
    void addNeedingContentCommand (kpCommand *cmd);


protected:
    // Sets the selection border mode when no drawing operation is active.
    //
    // Subclasses may wish to reimplement but should still call the base
    // implementation.  Reimplementations should wrap the whole
    // reimplementation in a kpViewManager::setQueueUpdates() block.
    virtual void setSelectionBorderForHaventBegunDraw ();
private:
    // (not const due to purely syntactic issue: it calls the non-const operation();
    //  it really acts like a const method though)
    QString haventBegunDrawUserMessage ();


public:
    virtual void begin ();
    virtual void end ();


public:
    virtual void reselect ();


//
// Drawing - Beginning a Drag
//

protected:
    // Called by calculateDrawType() to determine what type of draw operation
    // is begin started in response to a drag inside the bounding rectangle of
    // a selection.
    //
    // This implementation returns "Move".
    //
    // You are free to reimplement this and may choose to call this base
    // implementation or not.
    virtual DrawType calculateDrawTypeInsideSelection () const;
    
    // Called by beginDraw() to determine what type of draw operation is
    // being started.
    //
    // This implementation behaves according to the first rule that matches:
    //
    // 1. If the cursor is on top of a selection resize handle and no modifiers
    //    are held (i.e. not a smearing move operation), it returns "ResizeScale".
    //
    // 2. If the cursor is inside the bounding rectangle of a selection, it
    //    calls calculateDrawTypeInsideSelection().
    //
    // 3. Otherwise, it returns Create.
    //
    // You are free to reimplement this and may choose to call this base
    // implementation or not.
    virtual DrawType calculateDrawType () const;
public:
    virtual void beginDraw ();


public:
    virtual void hover (const QPoint &point);
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                       const QRect &normalizedRect);


public:
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    
    
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
    virtual void endDraw (const QPoint &thisPoint, const QRect &normalizedRect);


//
// Drawing - Operation Dispatching
//

protected:
    enum Operation
    {
        //
        // These may be called outside of a drawing operation so you cannot,
        // for instance, query drawType().
        //

        // Returns the message for the given drag mode and operation.
        HaventBegunDrawUserMessage,

        SetCursor,


        //
        // Called to start, to end, or inside, a drawing operation.
        //

        BeginDraw, Draw, Cancel, EndDraw
    };

    virtual QVariant operation (DrawType drawType, Operation op,
        const QVariant &data1 = QVariant (), const QVariant &data2 = QVariant ());


//
// Create
//

private:
    void initCreate ();
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
    // Subclasses must:
    // 1. Set the document's selection (which may not have previously existed)
    //    to the specified size.
    // 2. Update the status bar by calling kpTool::setUserShapePoints().
    //
    // <accidentalDragAdjustedPoint> = currentPoint() but is set to startPoint()
    //                                 if the mouse has not been moved much
    //                                 (6 manhatten length pixels from startPoint()
    //                                 within a short period of time (200ms).
    //                                 See m_createNOPTimer.
    // <normalizedRect> = as passed to kpTool::draw().
    //
    // If the drag has already begun (<dragHasBegun>), you must return "true".
    // If it has not, you should return whether you think the drag should be
    // started based on drag (usually, if <accidentalDragAdjustedPoint> is
    // not equal to startPoint()).  The return value will be fed into the
    // next call as <dragHasBegun>.
    virtual bool drawCreateMoreSelectionAndUpdateStatusBar (
        bool dragHasBegun,
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
    void initMove ();
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
    void initResizeScale ();
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
    // resizeScaleCalculateNewSelectionPosSize() calls us with what the
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
    virtual void keyPressEvent (QKeyEvent *e);


private:
    struct kpAbstractSelectionToolPrivate * const d;
};


#endif  // kpAbstractSelectionTool_H
