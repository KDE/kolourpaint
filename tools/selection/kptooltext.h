
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#include <kptoolselection.h>


class QKeyEvent;

class kpColor;
class kpMainWindow;
class kpSelection;
class kpTextStyle;
class kpViewManager;

class kpToolTextInsertCommand;
class kpToolTextEnterCommand;
class kpToolTextBackspaceCommand;
class kpToolTextDeleteCommand;


class kpToolText : public kpToolSelection
{
Q_OBJECT

public:
    kpToolText (kpMainWindow *mainWindow);
    virtual ~kpToolText ();

private:
    bool onSelectionToSelectText () const;

protected:
    virtual QString haventBegunDrawUserMessageOnResizeHandle () const;
    virtual QString haventBegunDrawUserMessageInsideSelection () const;
    virtual QString haventBegunDrawUserMessageOutsideSelection () const;


    //
    // Command Handling
    //
    
protected:
    /**
     * Indicates that no current text editing command is active.
     * You must call this when ending the current command (e.g. changing
     * from backspacing to inserting text).
     *
     * It achieves this by zero'ing out all the m_.+Command pointers.
     * It does not delete the pointers as they should be owned by the
     * commandHistory().
     */
    void setAllCommandPointersToZero ();
    
    
    /**
     * Ends the current text editing command by eventually calling
     * setAllCommandPointersToZero(), returns a new
     * kpToolTextBackspaceCommand and adds it to the commandHistory().
     *
     * @param cmd A Pointer to one of the m_backspace.*Command pointers.
     *            On function return, the pointed-to m_backspace.*Command
     *            pointer will point to a new kpToolTextBackspaceCommand.
     */
    void addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd);
    
    /**
     * Ends the current text editing command by eventually calling
     * setAllCommandPointersToZero(), returns a new
     * kpToolTextDeleteCommand and adds it to the commandHistory().
     *
     * @param cmd A Pointer to one of the m_delete.*Command pointers. On
     *            function return, the pointed-to m_delete.*Command pointer
     *            will point to a new kpToolTextDeleteCommand.
     */
    void addNewDeleteCommand (kpToolTextDeleteCommand **cmd);
    
    
public:
    virtual void begin ();
    virtual void end ();

    bool hasBegunText () const;
    virtual bool hasBegunShape () const;

protected:
    virtual kpToolSelection::DragType beginDrawInsideSelection ();
    virtual QCursor cursorInsideSelection () const;
    virtual void setSelectionBorderForHaventBegunDraw ();


public:
    virtual void cancelShape ();
protected:
    virtual QString nonSmearMoveCommandName () const;
public:
    virtual void endShape (const QPoint &thisPoint, const QRect &normalizedRect);


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
    static bool cursorIsOnWordChar (const QList <QString> &textLines,
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
    static bool cursorIsAtStart (const QList <QString> &textLines,
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
    static bool cursorIsAtEnd (const QList <QString> &textLines,
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
    static void moveCursorLeft (const QList <QString> &textLines,
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
    static void moveCursorRight (const QList <QString> &textLines,
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
    static int moveCursorToWordStart (const QList <QString> &textLines,
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
    static int moveCursorToNextWordStart (const QList <QString> &textLines,
        int *cursorRow, int *cursorCol);
        

    //
    // Key Press Handling
    //
    
protected:
    // Prevents actions with single letter/number shortcuts from eating
    // keystrokes while a text selection is active.  This is important
    // because the Tool Box actions default to single letter/number
    // shortcuts.
    virtual bool viewEvent (QEvent *e);


    /**
     * Moves the text cursor up one character.  Accepts the key event @p e.
     *
     * If there was an active text editing command, it ends it first.
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
     * If there was an active text editing command, it ends it first.
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
     * If there was an active text editing command, it ends it first.
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
     * If there was an active text editing command, it ends it first.
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
     * If there was an active text editing command, it ends it first.
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
     * If there was an active text editing command, it ends it first.
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

        
    /**
     * Backspaces and if the active text editing command is not
     * m_backspaceCommand, it calls addNewBackspaceCommand() on
     * m_backspaceCommand first.
     *
     * If CTRL is held, it backspaces to the start of the active word
     * and if the current text editing command was not
     * m_backspaceWordCommand, it calls addNewBackspaceCommand() on
     * m_backspaceWordCommand first.
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
     * m_deleteCommand, it calls addNewDeleteCommand() on
     * m_deleteCommand first.
     *
     * If CTRL is held, it delets to the start of the next word
     * and if the active text editing command was not
     * m_deleteWordCommand, it calls addNewDeleteCommand() on
     * m_deleteWordCommand first.
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
     * m_enterCommand, it ends the command, constructs m_enterCommand adding
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
     * m_insertCommand, it ends the command, constructs m_insertCommand
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
    
        
protected:
    /**
     * Handles key press events.
     *
     * If the user is currently drawing/resizing something or if the
     * document doesn't have a text selection, it passes control to the
     * otherwise overridden kpToolSelection::keyPressEvent().
     *
     * Else, for a recognised key it calls handle.*Press().  If a
     * recognised key was not pressed, it assumes that one or more text
     * characters was typed, and calls handleTextTyped().  If none of the
     * handle.*() methods call e->accept(), it passes control to the
     * otherwise overridden kpToolSelection::keyPressEvent().
     *
     * @param e Mutable key event information.
     *
     * Re-implemented from kpToolSelection.
     */

    virtual void keyPressEvent (QKeyEvent *e);
    
    
    //
    // Input Method Text Entry
    //
    
    virtual void inputMethodEvent (QInputMethodEvent *e);

    
    //
    // User Changing Text Style Elements
    //

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
    virtual void slotIsOpaqueChanged ();
    
    
protected:
    /**
     * Asks kpTool to call slotColorsSwapped() when the foreground and
     * background color are swapped.
     *
     * Re-implemented from kpTool.
     */
    virtual bool careAboutColorsSwapped () const { return true; }

protected slots:
    virtual void slotColorsSwapped (const kpColor &newForegroundColor,
                                    const kpColor &newBackgroundColor);
                                    
    virtual void slotForegroundColorChanged (const kpColor &color);
    virtual void slotBackgroundColorChanged (const kpColor &color);
    virtual void slotColorSimilarityChanged (double, int);

public slots:
    void slotFontFamilyChanged (const QString &fontFamily, const QString &oldFontFamily);
    void slotFontSizeChanged (int fontSize, int oldFontSize);
    void slotBoldChanged (bool isBold);
    void slotItalicChanged (bool isItalic);
    void slotUnderlineChanged (bool isUnderline);
    void slotStrikeThruChanged (bool isStrikeThru);

    
protected:
    kpToolTextInsertCommand *m_insertCommand;
    kpToolTextEnterCommand *m_enterCommand;
    kpToolTextBackspaceCommand *m_backspaceCommand, *m_backspaceWordCommand;
    kpToolTextDeleteCommand *m_deleteCommand, *m_deleteWordCommand;

    bool m_isIMStarted;
    int m_IMStartCursorRow;
    int m_IMStartCursorCol;
    QString m_IMPreeditStr;
};


#endif  // KP_TOOL_TEXT_H
