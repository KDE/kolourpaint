
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


#include <qstring.h>

#include <kpcommandhistory.h>
#include <kptextstyle.h>
#include <kptoolselection.h>


class QKeyEvent;

class kpColor;
class kpMainWindow;
class kpSelection;
class kpViewManager;

// (All defined below)
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

    virtual bool careAboutColorsSwapped () const { return true; }

protected:
    void setAllCommandPointersToZero ();
    
public:
    virtual void begin ();
    virtual void end ();

    bool hasBegunText () const;
    virtual bool hasBegunShape () const;
    virtual void cancelShape ();
    virtual void endShape (const QPoint &thisPoint, const QRect &normalizedRect);

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
        
protected:
    void addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd);
    void addNewDeleteCommand (kpToolTextDeleteCommand **cmd);
    
    // Prevents actions with single letter/number shortcuts from eating
    // keystrokes while a text selection is active.  This is important
    // because the Tool Box actions default to single letter/number
    // shortcuts.
    virtual bool event (QEvent *e);

    virtual void keyPressEvent (QKeyEvent *e);
    virtual void inputMethodEvent (QInputMethodEvent *e);

protected:
    bool shouldChangeTextStyle () const;
    void changeTextStyle (const QString &name,
                          const kpTextStyle &newTextStyle,
                          const kpTextStyle &oldTextStyle);

protected slots:
    virtual void slotIsOpaqueChanged ();
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


class kpToolTextChangeStyleCommand : public kpNamedCommand
{
public:
    kpToolTextChangeStyleCommand (const QString &name,
        const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextChangeStyleCommand ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    kpTextStyle m_newTextStyle, m_oldTextStyle;
};

class kpToolTextInsertCommand : public kpNamedCommand
{
public:
    kpToolTextInsertCommand (const QString &name,
        int row, int col, QString newText,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextInsertCommand ();

    void addText (const QString &moreText);

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    QString m_newText;
};

class kpToolTextEnterCommand : public kpNamedCommand
{
public:
    enum Action
    {
        DontAddEnterYet,
        AddEnterNow
    };
    
    kpToolTextEnterCommand (const QString &name,
        int row, int col, Action action,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextEnterCommand ();

    void addEnter ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    int m_numEnters;
};

class kpToolTextBackspaceCommand : public kpNamedCommand
{
public:
    enum Action
    {
        DontAddBackspaceYet,
        AddBackspaceNow
    };
    
    kpToolTextBackspaceCommand (const QString &name,
        int row, int col, Action action,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextBackspaceCommand ();

    void addBackspace ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    int m_numBackspaces;
    QString m_deletedText;
};

class kpToolTextDeleteCommand : public kpNamedCommand
{
public:
    enum Action
    {
        DontAddDeleteYet,
        AddDeleteNow
    };
    
    kpToolTextDeleteCommand (const QString &name,
        int row, int col, Action action,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextDeleteCommand ();

    void addDelete ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    int m_numDeletes;
    QString m_deletedText;
};


#endif  // KP_TOOL_TEXT_H
