
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


#ifndef KP_TOOL_TEXT_H
#define KP_TOOL_TEXT_H


#include "tools/selection/kpAbstractSelectionTool.h"


class QKeyEvent;

class kpColor;
class kpTextStyle;

class kpToolTextInsertCommand;
class kpToolTextEnterCommand;
class kpToolTextBackspaceCommand;
class kpToolTextDeleteCommand;


//
// kpAbstractSelectionTool considers a drawing operation to be a mouse
// drag that creates, moves or resize/scales a selection.
//
// kpToolText also considers any such drawing operation and alternatively,
// any active text command (e.g. inserting text), to be the "current
// shape".  kpTool's shape methods (e.g. hasBegunShape() and endShape())
// have been overloaded to ensure that they operate on whatever operation
// (drawing or text) is active.
//
// It is not possible to have a drawing command and text command active
// simultaneously.  However, it is possible to have neither active.
//
// Text commands do not end until a different kind of key is pressed or
// a drawing command commences.  For instance, if you were to
// type a character of text, a kpToolTextInsertCommand would be added to
// the command history but stays active so that future typed characters
// would simply be added to this command.  As soon as the user presses
// a different kind of key (e.g. arrow key, backspace) or drags the mouse,
// the command is no longer kept active.
//
//
// kpToolText implements a new drawing type, "SelectText", for moving the
// text cursor to the clicked location.
//
//
// As an exception to the standard kpAbstractSelectionTool behavior,
// a single click -- with no dragging -- can be used to create a new text
// box.
//
class kpToolText : public kpAbstractSelectionTool
{
Q_OBJECT

public:
    kpToolText (kpToolSelectionEnvironment *environ, QObject *parent);
    ~kpToolText () override;


//
// Text Command Handling
//

private:
    /**
     * Indicates that no current text editing command is active.
     * You must call this, via endShape(), when ending the current command
     * (e.g. changing from backspacing to inserting text
     *  e.g.2. changing from moving/resizing to inserting text).
     *
     * It achieves this by zero'ing out all the d->.+Command pointers.
     * It does not delete the pointers as they should be owned by the
     * commandHistory().
     *
     * If you call this excessively, you will break up commands into many
     * smaller commands e.g. a text insertion command with 10 characters
     * might be split into 10 text insertion commands, each containing 1
     * character.
     */
    void endTypingCommands ();


    /**
     * Ends the current text editing command by eventually calling
     * endTypingCommands(), returns a new
     * kpToolTextBackspaceCommand and adds it to the commandHistory().
     *
     * @param cmd A Pointer to one of the d->backspace.*Command pointers.
     *            On function return, the pointed-to d->backspace.*Command
     *            pointer will point to a new kpToolTextBackspaceCommand.
     */
    void addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd);

    /**
     * Ends the current text editing command by eventually calling
     * endTypingCommands(), returns a new
     * kpToolTextDeleteCommand and adds it to the commandHistory().
     *
     * @param cmd A Pointer to one of the d->delete.*Command pointers. On
     *            function return, the pointed-to d->delete.*Command pointer
     *            will point to a new kpToolTextDeleteCommand.
     */
    void addNewDeleteCommand (kpToolTextDeleteCommand **cmd);

    void addNewEnterCommand (kpToolTextEnterCommand **cmd);

    void addNewInsertCommand (kpToolTextInsertCommand **cmd);


//
// Drawing
//

protected:
    kpAbstractSelectionContentCommand *newGiveContentCommand () const override;

    QString nameOfCreateCommand () const override;


protected:
    void setSelectionBorderForHaventBegunDraw () override;


public:
    void begin () override;
    void end () override;


public:
    bool hasBegunText () const;
    bool hasBegunShape () const override;


//
// Drawing - Beginning a Drag
//

protected:
    DrawType calculateDrawTypeInsideSelection () const override;


public:
    void cancelShape () override;


public:
    void endShape (const QPoint &thisPoint, const QRect &normalizedRect) override;


//
// Drawing - Operation Dispatching
//

protected:
    QVariant operation (DrawType drawType, Operation op,
        const QVariant &data1 = QVariant (), const QVariant &data2 = QVariant ()) override;


//
// Create
//

protected:
    QString haventBegunDrawUserMessageCreate () const override;


protected:
    void setSelectionBorderForBeginDrawCreate () override;


private:
    // Returns the suggested width/height of a click-created text box:
    //
    // <mouseStart> = starting X/Y of the entire drag
    // <mouseEnd> = current ending X/Y of the entire drag
    //
    // <preferredMin> = the preferred minimum width/height of the selection
    // <smallestMin> = the legal minimum width/height of the selection
    //
    // <docSize> = the width/height of the document
    int calcClickCreateDimension (int mouseStart, int mouseEnd,
        int preferredMin, int smallestMin,
        int docSize);
    bool shouldCreate (
        bool drawAcceptedAsDrag,
        const QPoint &accidentalDragAdjustedPoint,
        const kpTextStyle &textStyle,
        int *minimumWidthOut, int *minimumHeightOut,
        bool *newDragHasBegun);
protected:
    bool drawCreateMoreSelectionAndUpdateStatusBar (
        bool drawAcceptedAsDrag,
        const QPoint &accidentalDragAdjustedPoint,
        const QRect &normalizedRectIn) override;


//
// Move
//

protected:
    QString haventBegunDrawUserMessageMove () const override;


protected:
    void setSelectionBorderForBeginDrawMove () override;


protected:
    QString nonSmearMoveCommandName () const override;


//
// Resize/Scale
//

protected:
    QString haventBegunDrawUserMessageResizeScale () const override;


protected:
    void setSelectionBorderForBeginDrawResizeScale () override;


//
// Select Text
//

private:
    bool onSelectionToSelectText () const;


private:
    QString haventBegunDrawUserMessageSelectText () const;

    void setCursorSelectText ();


private:
    void beginDrawSelectText ();


protected:
    virtual QVariant selectTextOperation (Operation op,
        const QVariant &data1 = QVariant (), const QVariant &data2 = QVariant ());


//
// User Changing Text Style Elements
//

protected:
    bool shouldChangeTextStyle () const;

    /**
     * Adds a kpToolTextChangeStyleCommand to commandHistory().
     *
     * Call this when an element of the text style changes (e.g. user
     * changes font size, boldness, color etc.).
     *
     * @param name Name of the command in the command history.
     * @param newTextStyle The new and current text style.
     * @param oldTextStyle The old and previous text style.
     *
     * You should only call this if shouldChangeTextStyle() returns true.
     */
    void changeTextStyle (const QString &name,
                          const kpTextStyle &newTextStyle,
                          const kpTextStyle &oldTextStyle);

protected slots:
    void slotIsOpaqueChanged (bool isOpaque) override;


protected:
    /**
     * Asks kpTool to call slotColorsSwapped() when the foreground and
     * background color are swapped.
     *
     * Re-implemented from kpTool.
     */
    bool careAboutColorsSwapped () const override { return true; }

protected slots:
    void slotColorsSwapped (const kpColor &newForegroundColor,
                                    const kpColor &newBackgroundColor) override;

    void slotForegroundColorChanged (const kpColor &color) override;
    void slotBackgroundColorChanged (const kpColor &color) override;
    void slotColorSimilarityChanged (double, int) override;

public slots:
    void slotFontFamilyChanged (const QString &fontFamily, const QString &oldFontFamily);
    void slotFontSizeChanged (int fontSize, int oldFontSize);
    void slotBoldChanged (bool isBold);
    void slotItalicChanged (bool isItalic);
    void slotUnderlineChanged (bool isUnderline);
    void slotStrikeThruChanged (bool isStrikeThru);


//
// Text Cursor Calculations (all static, no mutations)
//

protected:
    /**
     * @param textLines One or more lines of text.
     * @param cursorRow Current row of the cursor.
     * @param cursorCol Current column of the cursor.
     *
     * @returns whether the cursor is currently on a word character
     *          (not a space).
     */
    static bool CursorIsOnWordChar (const QList <QString> &textLines,
        int cursorRow, int cursorCol);


    /**
     * @param textLines One or more lines of text.
     * @param cursorRow Current row of the cursor.
     * @param cursorCol Current column of the cursor.
     *
     * @returns whether the given cursor position is at the start of
     *          textLines (on the first character of the first line)
     *          i.e. when moveCursorLeft() won't do anything.
     */
    static bool CursorIsAtStart (const QList <QString> &textLines,
        int cursorRow, int cursorCol);

    /**
     * @param textLines One or more lines of text.
     * @param cursorRow Current row of the cursor.
     * @param cursorCol Current column of the cursor.
     *
     * @returns whether the given cursor position is at the end of
     *          textLines (after the last character of the last line)
     *          i.e. when moveCursorRight() won't do anything.
     */
    static bool CursorIsAtEnd (const QList <QString> &textLines,
        int cursorRow, int cursorCol);


    /**
     * Moves the given cursor position one character to the left, if
     * this is possible (i.e. if not cursorIsAtStart()).  This may move the
     * cursor one line up.
     *
     * @param textLines One or more lines of text.
     * @param cursorRow Value-result parameter, initially containing
     *                  the current row of the cursor and modified on
     *                  return to indicate the new row.
     * @param cursorCol Value-result parameter, initially containing
     *                  the current column of the cursor and modified on
     *                  return to indicate the new column.
     */
    static void MoveCursorLeft (const QList <QString> &textLines,
        int *cursorRow, int *cursorCol);

    /**
     * Moves the given cursor position one character to the right, if
     * this is possible (i.e. if not cursorIsAtEnd()).  This may move the
     * cursor one line down.
     *
     * @param textLines One or more lines of text.
     * @param cursorRow Value-result parameter, initially containing
     *                  the current row of the cursor and modified on
     *                  return to indicate the new row.
     * @param cursorCol Value-result parameter, initially containing
     *                  the current column of the cursor and modified on
     *                  return to indicate the new column.
     */
    static void MoveCursorRight (const QList <QString> &textLines,
        int *cursorRow, int *cursorCol);


    /**
     * Moves the row and column values, representing the current cursor
     * location, to the start of the current word.  If the cursor is
     * on a space, it will move to the start of the previous word.
     *
     * This is normally used for a CTRL+Left or CTRL+Backspace behaviour.
     *
     * @param textLines One or more lines of text.
     * @param cursorRow Value-result parameter, initially containing
     *                  the current row of the cursor and modified on
     *                  return to indicate the new row.
     * @param cursorCol Value-result parameter, initially containing
     *                  the current column of the cursor and modified on
     *                  return to indicate the new column.
     *
     * @returns the number of times, it <b>attempted</b> to move left.
     *          Note: Attempting to moving left when cursorIsAtStart()
     *                may still be counted as a move.
     */
    static int MoveCursorToWordStart (const QList <QString> &textLines,
        int *cursorRow, int *cursorCol);

    /**
     * Moves the row and column values, representing the current cursor
     * location, to the start of the next word.  This is regardless of
     * whether the cursor starts on a space or not.
     *
     * This is normally used for a CTRL+Right or CTRL+Delete behaviour.
     *
     * @param textLines One or more lines of text.
     * @param cursorRow Value-result parameter, initially containing
     *                  the current row of the cursor and modified on
     *                  return to indicate the new row.
     * @param cursorCol Value-result parameter, initially containing
     *                  the current column of the cursor and modified on
     *                  return to indicate the new column.
     *
     * @returns the number of times, it <b>attempted</b> to move right.
     *          Note: Attempting to moving right when cursorIsAtEnd()
     *                may still be counted as a move.
     */
    static int MoveCursorToNextWordStart (const QList <QString> &textLines,
        int *cursorRow, int *cursorCol);


//
// Keyboard Events - Handling Arrow Keys
//
// These methods always:
//
// 1. Before doing anything, end the current shape (e.g. a text editing command or
//    selection move command).  This is done for 2 reasons:
//
//    a) The user has interrupted the current command e.g. if I were to
//       type some text, press an arrow key and then type text again, the two
//       periods of text typing should be separate commands due to the arrow
//       key interruption.
//
//    b) To simplify the code by avoiding interference with the current command
//       esp. since commands do not expect the cursor to move in the middle.
//
// 2. Accept the event as it is always intended for the method.  This is even
//    if the operation could not complete e.g. an attempt to move the cursor
//    left when it is already at column 0.
//
protected:
    /**
     * Moves the text cursor up one character.  Accepts the key event @p e.
     *
     * If there was an active text editing or selection command, it ends it first.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleUpKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);

    /**
     * Moves the text cursor down one character.  Accepts the key event @p e.
     *
     * If there was an active text editing or selection command, it ends it first.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleDownKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);

    /**
     * Moves the text cursor left one character or if CTRL is held, one
     * word.  Accepts the key event @p e.
     *
     * If there was an active text editing or selection command, it ends it first.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleLeftKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);

    /**
     * Moves the text cursor right one character or if CTRL is held, one
     * word.  Accepts the key event @p e.
     *
     * If there was an active text editing or selection command, it ends it first.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleRightKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);


    /**
     * Moves the text cursor to the start of the line and if CTRL is held,
     * to the first line.  Accepts the key event @p e.
     *
     * If there was an active text editing or selection command, it ends it first.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleHomeKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);

    /**
     * Moves the text cursor to after the last character of the current
     * text line or if CTRL is held, after the last character of the last
     * line.  Accepts the key event @p e.
     *
     * If there was an active text editing or selection command, it ends it first.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleEndKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);


//
// Keyboard Events - Handling Typing Keys
//
// For each method, if the respective event was actually intended for the
// method:
//
// 1. If the event will not be a NOP:
//
//        If the current command is not the same as what this method would produce,
//        it starts a new one, ending the current one (using the addNew*Command()
//        methods).  If the current command is the same, it simply extends the
//        current command e.g. if the current command was a kpToolTextInsertCommand
//        and the user typed another character of text, that character would just be
//        added to that command.
//
// 2. Accept the event.  This is even if the operation could not complete e.g.
//    an attempt backspace when the cursor is at column 0.
//
// If the event was not intended for the method (e.g. pressing CTRL, Caps Lock
// or any other key that does not produce text, in handleTextTyped()), nothing
// happens.
//
protected:
    /**
     * Backspaces and if the active text editing command is not
     * d->backspaceCommand, it calls addNewBackspaceCommand() on
     * d->backspaceCommand first.
     *
     * If CTRL is held, it backspaces to the start of the active word
     * and if the current text editing command was not
     * d->backspaceWordCommand, it calls addNewBackspaceCommand() on
     * d->backspaceWordCommand first.
     *
     * In this way, Backspace and CTRL+Backspace are separate entries
     * in the commandHistory().
     *
     * Accepts the key event @p e.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleBackspaceKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);

    /**
     * Deletes and if the active text editing command is not
     * d->deleteCommand, it calls addNewDeleteCommand() on
     * d->deleteCommand first.
     *
     * If CTRL is held, it deletes to the start of the next word
     * and if the active text editing command was not
     * d->deleteWordCommand, it calls addNewDeleteCommand() on
     * d->deleteWordCommand first.
     *
     * In this way, Delete and CTRL+Delete are separate entries
     * in the commandHistory().
     *
     * Accepts the key event @p e.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleDeleteKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);


    /**
     * Enters and if the active text editing command is not
     * d->enterCommand, it ends the command, constructs d->enterCommand adding
     * it to commandHistory(), first.
     *
     * Accepts the key event @p e.
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleEnterKeyPress (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);


    /**
     * Inserts the printable characters of e->text() and accepts the key
     * event @p e.  If the active text editing command is not
     * d->insertCommand, it ends the command, constructs d->insertCommand
     * adding it to commandHistory(), first.
     *
     * If e->text() does not contain any printable characters, it does not
     * do anything.  As a result, it would not accept the key event @e.
     * This printability restriction prevents control characters from being
     * typed when they should be trapped by a keyPressEvent() that is lower
     * in the call chain (for e.g. CTRL+Z for Undo).
     *
     * @param e Mutable key event information.
     * @param textLines One or more lines of text (convenience parameter).
     * @param cursorRow Current row of the cursor (convenience parameter).
     * @param cursorCol Current column of the cursor (convenience parameter).
     *
     * Called by keyPressEvent().
     */
    void handleTextTyped (QKeyEvent *e,
        const QList <QString> &textLines, int cursorRow, int cursorCol);


//
// Keyboard Events
//

protected:
    // Prevents actions with single letter/number shortcuts from eating
    // keystrokes while a text selection is active.  This is important
    // because the Tool Box actions default to single letter/number
    // shortcuts.
    bool viewEvent (QEvent *e) override;

    /**
     * Handles key press events.
     *
     * If the user is currently drawing/resizing something or if the
     * document doesn't have a text selection, it passes control to the
     * otherwise overridden kpAbstractSelectionTool::keyPressEvent().
     *
     * Else, for a recognised key it calls handle.*Press().  If a
     * recognised key was not pressed, it assumes that one or more text
     * characters was typed, and calls handleTextTyped().  If none of the
     * handle.*() methods call e->accept(), it passes control to the
     * otherwise overridden kpAbstractSelectionTool::keyPressEvent().
     *
     * @param e Mutable key event information.
     *
     * Re-implemented from kpAbstractSelectionTool.
     */

    void keyPressEvent (QKeyEvent *e) override;


//
// Input Method Text Entry
//

protected:
    void inputMethodEvent (QInputMethodEvent *e) override;


private:
    struct kpToolTextPrivate * const d;
};


#endif  // KP_TOOL_TEXT_H
