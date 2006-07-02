
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

#define DEBUG_KP_TOOL_TEXT 0


#include <kptooltext.h>

#include <qevent.h>
#include <qlist.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpviewmanager.h>


kpToolText::kpToolText (kpMainWindow *mainWindow)
    : kpToolSelection (Text,
                       i18n ("Text"), i18n ("Writes text"),
                       Qt::Key_T,
                       mainWindow, "tool_text"),
      m_isIMStarted (false),
      m_IMStartCursorRow (0),
      m_IMStartCursorCol (0)
{
}

kpToolText::~kpToolText ()
{
}


// protected
void kpToolText::setAllCommandPointersToZero ()
{
    m_insertCommand = 0;
    m_enterCommand = 0;
    
    m_backspaceCommand = 0;
    m_backspaceWordCommand = 0;
    
    m_deleteCommand = 0;
    m_deleteWordCommand = 0;
}


// public virtual [base kpToolSelection]
void kpToolText::begin ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::begin()" << endl;
#endif

    mainWindow ()->enableTextToolBarActions (true);
    viewManager ()->setTextCursorEnabled (true);

    setAllCommandPointersToZero ();
    
    kpToolSelection::begin ();
}

// public virtual [base kpToolSelection]
void kpToolText::end ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::end()" << endl;
#endif

    kpToolSelection::end ();

    viewManager ()->setTextCursorEnabled (false);
    mainWindow ()->enableTextToolBarActions (false);
}


// public
bool kpToolText::hasBegunText () const
{
    return (m_insertCommand ||
            m_enterCommand ||
            m_backspaceCommand ||
            m_backspaceWordCommand ||
            m_deleteCommand ||
            m_deleteWordCommand);
}

// public virtual [base kpTool]
bool kpToolText::hasBegunShape () const
{
    return (hasBegunDraw () || hasBegunText ());
}


// public virtual [base kpToolSelection]
void kpToolText::cancelShape ()
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::cancelShape()" << endl;
#endif

    if (m_dragType != Unknown)
        kpToolSelection::cancelShape ();
    else if (hasBegunText ())
    {
        setAllCommandPointersToZero ();

        commandHistory ()->undo ();
    }
    else
        kpToolSelection::cancelShape ();
}

// public virtual [base kpTool]
void kpToolText::endShape (const QPoint &thisPoint, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::endShape()" << endl;
#endif

    if (m_dragType != Unknown)
        kpToolSelection::endDraw (thisPoint, normalizedRect);
    else if (hasBegunText ())
        setAllCommandPointersToZero ();
    else
        kpToolSelection::endDraw (thisPoint, normalizedRect);
}

    
// protected static
bool kpToolText::cursorIsOnWordChar (const QList <QString> &textLines,
    int cursorRow, int cursorCol)
{
    return (cursorRow >= 0 && cursorRow < (int) textLines.size () &&
            cursorCol >= 0 && cursorCol < (int) textLines [cursorRow].length () &&
            !textLines [cursorRow][cursorCol].isSpace ());
}


// protected static
bool kpToolText::cursorIsAtStart (const QList <QString> &,
    int cursorRow, int cursorCol)
{
    return (cursorRow == 0 && cursorCol == 0);
}

// protected static
bool kpToolText::cursorIsAtEnd (const QList <QString> &textLines,
    int cursorRow, int cursorCol)
{
    return (cursorRow == (int) textLines.size () - 1 &&
            cursorCol == (int) textLines [cursorRow].length ());
}


// protected static
void kpToolText::moveCursorLeft (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    (*cursorCol)--;

    if (*cursorCol < 0)
    {
        (*cursorRow)--;
        if (*cursorRow < 0)
        {
            *cursorRow = 0;
            *cursorCol = 0;
        }
        else
            *cursorCol = textLines [*cursorRow].length ();
    }
}

// protected static
void kpToolText::moveCursorRight (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    (*cursorCol)++;                                            
                                                            
    if (*cursorCol > (int) textLines [*cursorRow].length ())
    {                                                       
        (*cursorRow)++;                                        
        if (*cursorRow > (int) textLines.size () - 1)        
        {                                                   
            *cursorRow = textLines.size () - 1;              
            *cursorCol = textLines [*cursorRow].length ();    
        }                                                   
        else                                                
            *cursorCol = 0;                                  
    }                                                       
}


#define IS_ON_SPACE_OR_EOL() !cursorIsOnWordChar (textLines, *cursorRow, *cursorCol)
        
// protected static
int kpToolText::moveCursorToWordStart (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    int numMoves = 0;
    
#define IS_ON_ANCHOR()                                            \
    (cursorIsOnWordChar (textLines, *cursorRow, *cursorCol) &&    \
         (cursorCol == 0 ||                                       \
             !cursorIsOnWordChar (textLines, *cursorRow, *cursorCol - 1)))
#define MOVE_CURSOR_LEFT()    \
    (moveCursorLeft (textLines, cursorRow, cursorCol), ++numMoves)
    
                        
    // (these comments will exclude the row=0,col=0 boundary case)

    if (IS_ON_ANCHOR ())
        MOVE_CURSOR_LEFT ();

    // --- now we're not on an anchor point (start of word) ---

    // End up on a letter...
    while (!(*cursorRow == 0 && *cursorCol == 0) &&
            (IS_ON_SPACE_OR_EOL ()))
    {
        MOVE_CURSOR_LEFT ();
    }

    // --- now we're on a letter ---

    // Find anchor point
    while (!(*cursorRow == 0 && *cursorCol == 0) && !IS_ON_ANCHOR ())
    {
        MOVE_CURSOR_LEFT ();
    }

    
#undef IS_ON_ANCHOR
#undef MOVE_CURSOR_LEFT

    return numMoves;
}

// protected static
int kpToolText::moveCursorToNextWordStart (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    int numMoves = 0;
    
#define IS_AT_END() cursorIsAtEnd (textLines, *cursorRow, *cursorCol)
#define MOVE_CURSOR_RIGHT()    \
    (moveCursorRight (textLines, cursorRow, cursorCol), ++numMoves)

    
    // (these comments will exclude the last row,end col boundary case)

    // Find space
    while (!IS_AT_END () && !IS_ON_SPACE_OR_EOL ())
    {
        MOVE_CURSOR_RIGHT ();
    }

    // --- now we're on a space ---

    // Find letter
    while (!IS_AT_END () && IS_ON_SPACE_OR_EOL ())
    {
        MOVE_CURSOR_RIGHT ();
    }

    // --- now we're on a letter ---

    
#undef IS_AT_END
#undef MOVE_CURSOR_RIGHT

    return numMoves;
}

#undef IS_ON_SPACE_OR_EOL


// protected
void kpToolText::addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd)
{
    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
    
    *cmd = new kpToolTextBackspaceCommand (i18n ("Text: Backspace"),
                viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                kpToolTextBackspaceCommand::DontAddBackspaceYet,
                mainWindow ());
    commandHistory ()->addCommand (*cmd, false/*no exec*/);
}

// protected
void kpToolText::addNewDeleteCommand (kpToolTextDeleteCommand **cmd)
{
    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

    *cmd = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                kpToolTextDeleteCommand::DontAddDeleteYet,
                mainWindow ());
    commandHistory ()->addCommand (*cmd, false/*no exec*/);
}


// protected virtual [base QWidget]
bool kpToolText::event (QEvent *e)
{
    const bool isShortcutOverrideEvent =
        (e->type () == QEvent::ShortcutOverride);
    const bool haveTextSelection =
        (document ()->selection () && document ()->selection ()->isText ());

#if DEBUG_KP_TOOL_TEXT && 0
    kDebug () << "kpToolText::event() type=" << e->type ()
              << " isShortcutOverrideEvent=" << isShortcutOverrideEvent
              << " haveTextSel=" << haveTextSelection
              << endl;
#endif

    if (!isShortcutOverrideEvent || !haveTextSelection)
        return kpToolSelection::event (e);

    QKeyEvent *ke = static_cast <QKeyEvent *> (e);
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::event() key=" << ke->key ()
              << " modifiers=" << ke->modifiers ()
              << " QChar.isPrint()=" << QChar (ke->key ()).isPrint ()
              << endl;
#endif

    // Can't be shortcut?
    if (ke->key () == 0 && ke->key () == Qt::Key_unknown)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tcan't be shortcut - safe to not react" << endl;
    #endif
    }
    // Normal letter (w/ or w/o shift, keypad button ok)?
    // TODO: don't like this check
    else if ((ke->modifiers () &
                (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) == 0 &&
            ke->key () < 0x100 /*QChar (ke->key ()).isPrint () - unfortunately F1 is printable too...*/)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tis text - grab" << endl;
    #endif
        e->accept ();
    }
    else
    {
        // Strickly speaking, we should grab stuff like the arrow keys
        // and enter.  In any case, should be done down in kpTool (as that
        // uses arrow keys too).
    }

    return kpToolSelection::event (e);
}

// protected virtual [base kpTool]
void kpToolText::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::keyPressEvent(e->text='" << e->text () << "')" << endl;
#endif


    e->ignore ();


    if (hasBegunDraw ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\talready began draw with mouse - passing on event to kpTool" << endl;
    #endif
        kpToolSelection::keyPressEvent (e);
        return;
    }


    kpSelection *sel = document ()->selection ();

    if (!sel || !sel->isText ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tno text sel - passing on event to kpTool" << endl;
    #endif
        //if (hasBegunShape ())
        //    endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        kpToolSelection::keyPressEvent (e);
        return;
    }


    const QList <QString> textLines = sel->textLines ();
    int cursorRow = viewManager ()->textCursorRow ();
    int cursorCol = viewManager ()->textCursorCol ();


    if (e->key () == Qt::Key_Enter || e->key () == Qt::Key_Return)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tenter pressed" << endl;
    #endif
        if (!m_enterCommand)
        {
            // TODO: why not endShapeInternal(); ditto for everywhere else in this file?
            if (hasBegunShape ())
                endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

            m_enterCommand = new kpToolTextEnterCommand (i18n ("Text: New Line"),
                viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                kpToolTextEnterCommand::AddEnterNow,
                mainWindow ());
            commandHistory ()->addCommand (m_enterCommand, false/*no exec*/);
        }
        else
            m_enterCommand->addEnter ();

        e->accept ();
    }
    else if (e->key () == Qt::Key_Backspace)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tbackspace pressed" << endl;
    #endif

        if ((e->state () & Qt::ControlModifier) == 0)
        {
            if (!m_backspaceCommand)
                addNewBackspaceCommand (&m_backspaceCommand);

            m_backspaceCommand->addBackspace ();
        }
        else
        {
            if (!m_backspaceWordCommand)
                addNewBackspaceCommand (&m_backspaceWordCommand);
                
            const int numMoves = moveCursorToWordStart (textLines,
                &cursorRow, &cursorCol);
                
            viewManager ()->setQueueUpdates ();
            {
                for (int i = 0; i < numMoves; i++)
                    m_backspaceWordCommand->addBackspace ();
            }
            viewManager ()->restoreQueueUpdates ();
                
            Q_ASSERT (cursorRow == viewManager ()->textCursorRow ());
            Q_ASSERT (cursorCol == viewManager ()->textCursorCol ());
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Delete)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tdelete pressed" << endl;
    #endif

        if ((e->state () & Qt::ControlModifier) == 0)
        {
            if (!m_deleteCommand)
                addNewDeleteCommand (&m_deleteCommand);
                
            m_deleteCommand->addDelete ();
        }
        else
        {
            if (!m_deleteWordCommand)
                addNewDeleteCommand (&m_deleteWordCommand);
                
            // We don't want to know the cursor pos of the next word start
            // as delete should keep cursor in same pos.
            int cursorRowThrowAway = cursorRow,
                cursorColThrowAway = cursorCol;
            const int numMoves = moveCursorToNextWordStart (textLines,
                &cursorRowThrowAway, &cursorColThrowAway);
                
            viewManager ()->setQueueUpdates ();
            {
                for (int i = 0; i < numMoves; i++)
                    m_deleteWordCommand->addDelete ();
            }
            viewManager ()->restoreQueueUpdates ();
                
            // Assert unchanged as delete should keep cursor in same pos.
            Q_ASSERT (cursorRow == viewManager ()->textCursorRow ());
            Q_ASSERT (cursorCol == viewManager ()->textCursorCol ());
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Up)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tup pressed" << endl;
    #endif

        if (hasBegunShape ())
            endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if (cursorRow > 0)
        {
            cursorRow--;
            cursorCol = qMin (cursorCol, (int) textLines [cursorRow].length ());
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Down)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tdown pressed" << endl;
    #endif

        if (hasBegunShape ())
            endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if (cursorRow < (int) textLines.size () - 1)
        {
            cursorRow++;
            cursorCol = qMin (cursorCol, (int) textLines [cursorRow].length ());
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Left)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tleft pressed" << endl;
    #endif

        if (hasBegunShape ())
            endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if ((e->state () & Qt::ControlModifier) == 0)
        {
        #if DEBUG_KP_TOOL_TEXT
            kDebug () << "\tmove single char" << endl;
        #endif

            moveCursorLeft (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }
        else
        {
        #if DEBUG_KP_TOOL_TEXT
            kDebug () << "\tmove to start of word" << endl;
        #endif

            moveCursorToWordStart (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }

    #undef MOVE_CURSOR_LEFT

        e->accept ();

    }
    else if (e->key () == Qt::Key_Right)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tright pressed" << endl;
    #endif

        if (hasBegunShape ())
            endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if ((e->state () & Qt::ControlModifier) == 0)
        {
        #if DEBUG_KP_TOOL_TEXT
            kDebug () << "\tmove single char" << endl;
        #endif

            moveCursorRight (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }
        else
        {
        #if DEBUG_KP_TOOL_TEXT
            kDebug () << "\tmove to start of next word" << endl;
        #endif

            moveCursorToNextWordStart (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);

        }

    #undef MOVE_CURSOR_RIGHT

        e->accept ();
    }
    else if (e->key () == Qt::Key_Home)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\thome pressed" << endl;
    #endif

        if (hasBegunShape ())
            endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if (e->state () & Qt::ControlModifier)
            cursorRow = 0;

        cursorCol = 0;

        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);

        e->accept ();
    }
    else if (e->key () == Qt::Key_End)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tend pressed" << endl;
    #endif

        if (hasBegunShape ())
            endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if (e->state () & Qt::ControlModifier)
            cursorRow = textLines.size () - 1;

        cursorCol = textLines [cursorRow].length ();

        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);

        e->accept ();
    }
    else
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\ttext='" << e->text () << "'" << endl;
    #endif
        QString usableText;
        for (int i = 0; i < (int) e->text ().length (); i++)
        {
            if (e->text ().at (i).isPrint ())
                usableText += e->text ().at (i);
        }
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tusableText='" << usableText << "'" << endl;
    #endif

        if (usableText.length () > 0)
        {
            if (!m_insertCommand)
            {
                if (hasBegunShape ())
                    endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

                m_insertCommand = new kpToolTextInsertCommand (i18n ("Text: Write"),
                    viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                    usableText,
                    mainWindow ());
                commandHistory ()->addCommand (m_insertCommand, false/*no exec*/);
            }
            else
                m_insertCommand->addText (usableText);

            e->accept ();
        }
    }
#undef IS_SPACE


    if (!e->isAccepted ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tkey processing did not accept (text was '"
                   << e->text ()
                   << "') - passing on event to kpToolSelection"
                   << endl;
    #endif
        //if (hasBegunShape ())
        //    endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        kpToolSelection::keyPressEvent (e);
        return;
    }
}

// COMPAT
#if 1
void kpToolText::inputMethodEvent (QInputMethodEvent *e)
{
    (void) e;
}
#else
void kpToolText::imStartEvent (QIMEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::imStartEvent() text='" << e->text ()
               << " cursorPos=" << e->cursorPos ()
               << " selectionLength=" << e->selectionLength ()
               << endl;
#endif

    kpSelection *sel = document ()->selection ();
    if (hasBegunDraw() || !sel || !sel->isText ())
    {
        e->ignore();
        return;
    }

    m_IMStartCursorRow = viewManager ()->textCursorRow ();
    m_IMStartCursorCol = viewManager ()->textCursorCol ();
    m_IMPreeditStr.clear ();
}

void kpToolText::imComposeEvent (QIMEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::imComposeEvent() text='" << e->text ()
               << " cursorPos=" << e->cursorPos ()
               << " selectionLength=" << e->selectionLength ()
               << endl;
#endif

    kpSelection *sel = document ()->selection ();
    if (hasBegunDraw() || !sel || !sel->isText ())
    {
        e->ignore();
        return;
    }

    // remove old preedit
    if (m_IMPreeditStr.length() > 0 )
    {
        // set cursor at the start input point
        viewManager ()->setTextCursorPosition (m_IMStartCursorRow, m_IMStartCursorCol);
        for (unsigned int i = 0; i < m_IMPreeditStr.length(); i++)
        {
            if (!m_deleteCommand)
            {
                if (hasBegunShape ())
                    endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
                
                m_deleteCommand = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                    viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                    kpToolTextDeleteCommand::AddDeleteNow,
                    mainWindow ());
                commandHistory ()->addCommand (m_deleteCommand, false/*no exec*/);
            }
            else
                m_deleteCommand->addDelete ();
        }
    }
    
    // insert new preedit
    m_IMPreeditStr = e->text();
    if (m_IMPreeditStr.length() > 0)
    {
        if (!m_insertCommand)
        {
            if (hasBegunShape ())
                endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
            
            m_insertCommand = new kpToolTextInsertCommand (i18n ("Text: Write"),
                                                           viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                                                           m_IMPreeditStr,
                                                           mainWindow ());
            commandHistory ()->addCommand (m_insertCommand, false/*no exec*/);
        }
        else
            m_insertCommand->addText (m_IMPreeditStr);
    }

    // set cursor pos
    if (m_IMStartCursorRow >= 0)
    {
        int row = m_IMStartCursorRow;
        int col = m_IMStartCursorCol + e->cursorPos () /* + e->selectionLength()*/;
        viewManager ()->setTextCursorPosition (row, col, true /* update MicroFocusHint */);
    }
}

void kpToolText::imEndEvent (QIMEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::imEndEvent() text='" << e->text ()
               << " cursorPos=" << e->cursorPos ()
               << " selectionLength=" << e->selectionLength ()
               << endl;
#endif

    kpSelection *sel = document ()->selection ();
    if (hasBegunDraw() || !sel || !sel->isText ())
    {
        e->ignore();
        return;
    }

    // remove old preedit
    if (m_IMPreeditStr.length() > 0 )
    {
        // set cursor at the start input point
        viewManager ()->setTextCursorPosition (m_IMStartCursorRow, m_IMStartCursorCol);
        for (unsigned int i = 0; i < m_IMPreeditStr.length(); i++)
        {
            if (!m_deleteCommand)
            {
                if (hasBegunShape ())
                    endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
                
                m_deleteCommand = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                    viewManager ()->textCursorRow (),
                    viewManager ()->textCursorCol (),
                    kpToolTextDeleteCommand::AddDeleteNow,
                    mainWindow ());
                commandHistory ()->addCommand (m_deleteCommand, false/*no exec*/);
            }
            else
                m_deleteCommand->addDelete ();
        }
    }
    m_IMPreeditStr.clear ();

    // commit string
    QString inputStr = e->text();
    if (inputStr.length() > 0)
    {
        if (!m_insertCommand)
        {
            if (hasBegunShape ())
                endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
            
            m_insertCommand = new kpToolTextInsertCommand (i18n ("Text: Write"),
                                                           viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                                                           inputStr,
                                                           mainWindow ());
            commandHistory ()->addCommand (m_insertCommand, false/*no exec*/);
        }
        else
            m_insertCommand->addText (inputStr);
    }
}
#endif

// protected
bool kpToolText::shouldChangeTextStyle () const
{
    if (mainWindow ()->settingTextStyle ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\trecursion - abort setting text style: "
                   << mainWindow ()->settingTextStyle ()
                   << endl;
    #endif
        return false;
    }

    if (!document ()->selection () ||
        !document ()->selection ()->isText ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tno text selection - abort setting text style" << endl;
    #endif
        return false;
    }

    return true;
}

// protected
void kpToolText::changeTextStyle (const QString &name,
                                  const kpTextStyle &newTextStyle,
                                  const kpTextStyle &oldTextStyle)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::changeTextStyle(" << name << ")" << endl;
#endif

    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

    commandHistory ()->addCommand (
        new kpToolTextChangeStyleCommand (
            name,
            newTextStyle,
            oldTextStyle,
            mainWindow ()));
}


// protected slot virtual [base kpToolSelection]
void kpToolText::slotIsOpaqueChanged ()
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotIsOpaqueChanged()" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBackgroundOpaque (!m_toolWidgetOpaqueOrTransparent->isOpaque ());

    changeTextStyle (newTextStyle.isBackgroundOpaque () ?
                         i18n ("Text: Opaque Background") :
                         i18n ("Text: Transparent Background"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpTool]
void kpToolText::slotColorsSwapped (const kpColor &newForegroundColor,
                                    const kpColor &newBackgroundColor)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotColorsSwapped()" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setForegroundColor (newBackgroundColor);
    oldTextStyle.setBackgroundColor (newForegroundColor);

    changeTextStyle (i18n ("Text: Swap Colors"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpTool]
void kpToolText::slotForegroundColorChanged (const kpColor & /*color*/)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotForegroundColorChanged()" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setForegroundColor (oldForegroundColor ());

    changeTextStyle (i18n ("Text: Foreground Color"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpToolSelection]
void kpToolText::slotBackgroundColorChanged (const kpColor & /*color*/)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotBackgroundColorChanged()" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBackgroundColor (oldBackgroundColor ());

    changeTextStyle (i18n ("Text: Background Color"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpToolSelection]
void kpToolText::slotColorSimilarityChanged (double, int)
{
    // --- don't pass on event to kpToolSelection which would have set the
    //     SelectionTransparency - not relevant to the Text Tool ---
}


// public slot
void kpToolText::slotFontFamilyChanged (const QString &fontFamily,
                                        const QString &oldFontFamily)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotFontFamilyChanged() new="
               << fontFamily
               << " old="
               << oldFontFamily
               << endl;
#else
    (void) fontFamily;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setFontFamily (oldFontFamily);

    changeTextStyle (i18n ("Text: Font"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotFontSizeChanged (int fontSize, int oldFontSize)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotFontSizeChanged() new="
               << fontSize
               << " old="
               << oldFontSize
               << endl;
#else
    (void) fontSize;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setFontSize (oldFontSize);

    changeTextStyle (i18n ("Text: Font Size"),
                     newTextStyle,
                     oldTextStyle);
}


// public slot
void kpToolText::slotBoldChanged (bool isBold)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotBoldChanged(" << isBold << ")" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBold (!isBold);

    changeTextStyle (i18n ("Text: Bold"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotItalicChanged (bool isItalic)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotItalicChanged(" << isItalic << ")" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setItalic (!isItalic);

    changeTextStyle (i18n ("Text: Italic"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotUnderlineChanged (bool isUnderline)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotUnderlineChanged(" << isUnderline << ")" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setUnderline (!isUnderline);

    changeTextStyle (i18n ("Text: Underline"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotStrikeThruChanged (bool isStrikeThru)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotStrikeThruChanged(" << isStrikeThru << ")" << endl;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = mainWindow ()->textStyle ();
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setStrikeThru (!isStrikeThru);

    changeTextStyle (i18n ("Text: Strike Through"),
                     newTextStyle,
                     oldTextStyle);
}


/*
 * kpToolTextChangeStyleCommand
 */

kpToolTextChangeStyleCommand::kpToolTextChangeStyleCommand (const QString &name,
    const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle,
    kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_newTextStyle (newTextStyle),
      m_oldTextStyle (oldTextStyle)
{
}

kpToolTextChangeStyleCommand::~kpToolTextChangeStyleCommand ()
{
}


// public virtual [base kpCommand]
int kpToolTextChangeStyleCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolTextChangeStyleCommand::execute ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolTextChangeStyleCommand::execute()"
               << " font=" << m_newTextStyle.fontFamily ()
               << " fontSize=" << m_newTextStyle.fontSize ()
               << " isBold=" << m_newTextStyle.isBold ()
               << " isItalic=" << m_newTextStyle.isItalic ()
               << " isUnderline=" << m_newTextStyle.isUnderline ()
               << " isStrikeThru=" << m_newTextStyle.isStrikeThru ()
               << endl;
#endif

    m_mainWindow->setTextStyle (m_newTextStyle);
    if (selection ())
        selection ()->setTextStyle (m_newTextStyle);
    else
        kError () << "kpToolTextChangeStyleCommand::execute() without sel" << endl;
}

// public virtual [base kpCommand]
void kpToolTextChangeStyleCommand::unexecute ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolTextChangeStyleCommand::unexecute()"
               << " font=" << m_newTextStyle.fontFamily ()
               << " fontSize=" << m_newTextStyle.fontSize ()
               << " isBold=" << m_newTextStyle.isBold ()
               << " isItalic=" << m_newTextStyle.isItalic ()
               << " isUnderline=" << m_newTextStyle.isUnderline ()
               << " isStrikeThru=" << m_newTextStyle.isStrikeThru ()
               << endl;
#endif

    m_mainWindow->setTextStyle (m_oldTextStyle);
    if (selection ())
        selection ()->setTextStyle (m_oldTextStyle);
    else
        kError () << "kpToolTextChangeStyleCommand::unexecute() without sel" << endl;
}


/*
 * kpToolTextInsertCommand
 */

kpToolTextInsertCommand::kpToolTextInsertCommand (const QString &name,
    int row, int col, QString newText,
    kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_row (row), m_col (col)
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    addText (newText);
}

kpToolTextInsertCommand::~kpToolTextInsertCommand ()
{
}


// public
void kpToolTextInsertCommand::addText (const QString &moreText)
{
    if (moreText.isEmpty ())
        return;

    QList <QString> textLines = selection ()->textLines ();
    const QString leftHalf = textLines [m_row].left (m_col);
    const QString rightHalf = textLines [m_row].mid (m_col);
    textLines [m_row] = leftHalf + moreText + rightHalf;
    selection ()->setTextLines (textLines);

    m_newText += moreText;
    m_col += moreText.length ();

    viewManager ()->setTextCursorPosition (m_row, m_col);
}


// public virtual [base kpCommand]
int kpToolTextInsertCommand::size () const
{
    return m_newText.length () * sizeof (QChar);
}


// public virtual [base kpCommand]
void kpToolTextInsertCommand::execute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QString text = m_newText;
    m_newText.clear ();
    addText (text);
}

// public virtual [base kpCommand]
void kpToolTextInsertCommand::unexecute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QList <QString> textLines = selection ()->textLines ();
    const QString leftHalf = textLines [m_row].left (m_col - m_newText.length ());
    const QString rightHalf = textLines [m_row].mid (m_col);
    textLines [m_row] = leftHalf + rightHalf;
    selection ()->setTextLines (textLines);

    m_col -= m_newText.length ();

    viewManager ()->setTextCursorPosition (m_row, m_col);
}


/*
 * kpToolTextEnterCommand
 */

kpToolTextEnterCommand::kpToolTextEnterCommand (const QString &name,
    int row, int col, Action action,
    kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_row (row), m_col (col),
      m_numEnters (0)
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    
    if (action == AddEnterNow)
        addEnter ();
}

kpToolTextEnterCommand::~kpToolTextEnterCommand ()
{
}


// public
void kpToolTextEnterCommand::addEnter ()
{
    QList <QString> textLines = selection ()->textLines ();

    const QString rightHalf = textLines [m_row].mid (m_col);

    textLines [m_row].truncate (m_col);
    textLines.insert (textLines.begin () + m_row + 1, rightHalf);

    selection ()->setTextLines (textLines);

    m_row++;
    m_col = 0;

    viewManager ()->setTextCursorPosition (m_row, m_col);

    m_numEnters++;
}


// public virtual [base kpCommand]
int kpToolTextEnterCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolTextEnterCommand::execute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    int oldNumEnters = m_numEnters;
    m_numEnters = 0;

    for (int i = 0; i < oldNumEnters; i++)
        addEnter ();
}

// public virtual [base kpCommand]
void kpToolTextEnterCommand::unexecute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QList <QString> textLines = selection ()->textLines ();

    for (int i = 0; i < m_numEnters; i++)
    {
        if (m_col != 0)
        {
            kError () << "kpToolTextEnterCommand::unexecute() col=" << m_col << endl;
            break;
        }

        if (m_row <= 0)
            break;

        int newRow = m_row - 1;
        int newCol = textLines [newRow].length ();

        textLines [newRow] += textLines [m_row];

        textLines.erase (textLines.begin () + m_row);

        m_row = newRow;
        m_col = newCol;
    }

    selection ()->setTextLines (textLines);

    viewManager ()->setTextCursorPosition (m_row, m_col);
}


/*
 * kpToolTextBackspaceCommand
 */

kpToolTextBackspaceCommand::kpToolTextBackspaceCommand (const QString &name,
    int row, int col, Action action,
    kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_row (row), m_col (col),
      m_numBackspaces (0)
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    
    if (action == AddBackspaceNow)
        addBackspace ();
}

kpToolTextBackspaceCommand::~kpToolTextBackspaceCommand ()
{
}


// public
void kpToolTextBackspaceCommand::addBackspace ()
{
    QList <QString> textLines = selection ()->textLines ();

    if (m_col > 0)
    {
        m_deletedText.prepend (textLines [m_row][m_col - 1]);

        textLines [m_row] = textLines [m_row].left (m_col - 1) +
                            textLines [m_row].mid (m_col);
        m_col--;
    }
    else
    {
        if (m_row > 0)
        {
            int newCursorRow = m_row - 1;
            int newCursorCol = textLines [newCursorRow].length ();

            m_deletedText.prepend ('\n');

            textLines [newCursorRow] += textLines [m_row];

            textLines.erase (textLines.begin () + m_row);

            m_row = newCursorRow;
            m_col = newCursorCol;
        }
    }

    selection ()->setTextLines (textLines);

    viewManager ()->setTextCursorPosition (m_row, m_col);

    m_numBackspaces++;
}


// public virtual [base kpCommand]
int kpToolTextBackspaceCommand::size () const
{
    return m_deletedText.length () * sizeof (QChar);
}


// public virtual [base kpCommand]
void kpToolTextBackspaceCommand::execute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    m_deletedText.clear ();
    int oldNumBackspaces = m_numBackspaces;
    m_numBackspaces = 0;

    for (int i = 0; i < oldNumBackspaces; i++)
        addBackspace ();
}

// public virtual [base kpCommand]
void kpToolTextBackspaceCommand::unexecute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QList <QString> textLines = selection ()->textLines ();

    for (int i = 0; i < (int) m_deletedText.length (); i++)
    {
        if (m_deletedText [i] == '\n')
        {
            const QString rightHalf = textLines [m_row].mid (m_col);

            textLines [m_row].truncate (m_col);
            textLines.insert (textLines.begin () + m_row + 1, rightHalf);

            m_row++;
            m_col = 0;
        }
        else
        {
            const QString leftHalf = textLines [m_row].left (m_col);
            const QString rightHalf = textLines [m_row].mid (m_col);

            textLines [m_row] = leftHalf + m_deletedText [i] + rightHalf;
            m_col++;
        }
    }

    m_deletedText.clear ();

    selection ()->setTextLines (textLines);

    viewManager ()->setTextCursorPosition (m_row, m_col);
}


/*
 * kpToolTextDeleteCommand
 */

kpToolTextDeleteCommand::kpToolTextDeleteCommand (const QString &name,
    int row, int col, Action action,
    kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_row (row), m_col (col),
      m_numDeletes (0)
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    
    if (action == AddDeleteNow)
        addDelete ();
}

kpToolTextDeleteCommand::~kpToolTextDeleteCommand ()
{
}


// public
void kpToolTextDeleteCommand::addDelete ()
{
    QList <QString> textLines = selection ()->textLines ();

    if (m_col < (int) textLines [m_row].length ())
    {
        m_deletedText.prepend (textLines [m_row][m_col]);

        textLines [m_row] = textLines [m_row].left (m_col) +
                            textLines [m_row].mid (m_col + 1);
    }
    else
    {
        if (m_row < (int) textLines.size () - 1)
        {
            m_deletedText.prepend ('\n');

            textLines [m_row] += textLines [m_row + 1];
            textLines.erase (textLines.begin () + m_row + 1);
        }
    }

    selection ()->setTextLines (textLines);

    viewManager ()->setTextCursorPosition (m_row, m_col);

    m_numDeletes++;
}


// public virtual [base kpCommand]
int kpToolTextDeleteCommand::size () const
{
    return m_deletedText.length () * sizeof (QChar);
}


// public virtual [base kpCommand]
void kpToolTextDeleteCommand::execute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    m_deletedText.clear ();
    int oldNumDeletes = m_numDeletes;
    m_numDeletes = 0;

    for (int i = 0; i < oldNumDeletes; i++)
        addDelete ();
}

// public virtual [base kpCommand]
void kpToolTextDeleteCommand::unexecute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QList <QString> textLines = selection ()->textLines ();

    for (int i = 0; i < (int) m_deletedText.length (); i++)
    {
        if (m_deletedText [i] == '\n')
        {
            const QString rightHalf = textLines [m_row].mid (m_col);

            textLines [m_row].truncate (m_col);
            textLines.insert (textLines.begin () + m_row + 1, rightHalf);
        }
        else
        {
            const QString leftHalf = textLines [m_row].left (m_col);
            const QString rightHalf = textLines [m_row].mid (m_col);

            textLines [m_row] = leftHalf + m_deletedText [i] + rightHalf;
        }
    }

    m_deletedText.clear ();

    selection ()->setTextLines (textLines);

    viewManager ()->setTextCursorPosition (m_row, m_col);
}


#include <kptooltext.moc>
