
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
#include <kpToolTextBackspaceCommand.h>
#include <kpToolTextChangeStyleCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolTextDeleteCommand.h>
#include <kpToolTextEnterCommand.h>
#include <kpToolTextInsertCommand.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpview.h>
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


// private
bool kpToolText::onSelectionToSelectText () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v)
        return 0;

    return v->mouseOnSelectionToSelectText (m_currentViewPoint);
}


// protected virtual [base kpToolSelection]
QString kpToolText::haventBegunDrawUserMessageOnResizeHandle () const
{
    return i18n ("Left drag to resize text box.");
}

// protected virtual [base kpToolSelection]
QString kpToolText::haventBegunDrawUserMessageInsideSelection () const
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
        return i18n ("Left click to change cursor position.");
    else
        return i18n ("Left drag to move text box.");
}

// protected virtual [base kpToolSelection]
QString kpToolText::haventBegunDrawUserMessageOutsideSelection () const
{
    return i18n ("Left drag to create text box.");
}


//
// Command Handling
//


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


// protected
void kpToolText::addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd)
{
    if (hasBegunShape ())
    {
        endShape (m_currentPoint,
            kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
    }
    
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
    {
        endShape (m_currentPoint,
            kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
    }

    *cmd = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                kpToolTextDeleteCommand::DontAddDeleteYet,
                mainWindow ());
    commandHistory ()->addCommand (*cmd, false/*no exec*/);
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


// protected virtual [base kpToolSelection]
kpToolSelection::DragType kpToolText::beginDrawInsideSelection ()
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
    {
        // This path is a bit dangerous since we don't call the base
        // implementation.
        //
        // However, we are doing something unusual here in that we aren't
        // drag-moving the selection - therefore it makes sense to not
        // call the base.
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\t\tis select cursor pos" << endl;
    #endif

        viewManager ()->setTextCursorPosition (
            document ()->selection ()->textRowForPoint (m_currentPoint),
            document ()->selection ()->textColForPoint (m_currentPoint));

        return kpToolSelection::SelectText;
    }
                
    return kpToolSelection::beginDrawInsideSelection ();
}

// protected virtual [base kpToolSelection]
QCursor kpToolText::cursorInsideSelection () const
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
        return Qt::IBeamCursor;

    return kpToolSelection::cursorInsideSelection ();
}


// private
int kpToolText::calcClickCreateDimension (int mouseStart, int mouseEnd,
    int preferredMin, int smallestMin,
    int docSize)
{
    Q_ASSERT (preferredMin >= smallestMin);
    Q_ASSERT (docSize > 0);
    
    // Get reasonable width/height for a text box.
    int ret = preferredMin;

    // X or Y increasing?
    if (mouseEnd >= mouseStart)
    {
        // Text box extends past document width/height?
        if (mouseStart + ret - 1 >= docSize)
        {
            // Cap width/height to not extend past but not below smallest
            // possible selection width/height
            ret = qMax (smallestMin, docSize - mouseStart);
        }
    }
    // X or Y decreasing
    else
    {
        // Text box extends past document start?
        // TODO: I doubt this code can be invoked for a click.
        //       Maybe very tricky interplay with accidental drag detection?
        if (mouseStart - ret + 1 < 0)
        {
            // Cap width/height to not extend past but not below smallest
            // possible selection width/height.
            ret = qMax (smallestMin, mouseStart + 1);
        }
    }

    return ret;
}

// private
bool kpToolText::shouldCreate (const kpTextStyle &textStyle,
        int *minimumWidthOut, int *minimumHeightOut)
{
    if (m_dragHasBegun || m_currentPoint != m_startPoint)
    {
    #if DEBUG_KP_TOOL_TEXT && 1
        kDebug () << "\tdrag creating text box" << endl;
    #endif
        *minimumWidthOut = kpSelection::minimumWidthForTextStyle (textStyle);
        *minimumHeightOut = kpSelection::minimumHeightForTextStyle (textStyle);
        return true/*do create text box*/;
    }

    
    //
    // User is possibly clicking to create a text box.
    //
    // Create a text box of reasonable ("preferred minimum") size.
    //
    // However, if it turns out that this is just the start of the drag,
    // we will be called again but the above code will execute instead,
    // ignoring this resonable size.
    //

#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "\tclick creating text box" << endl;
#endif

    // (Click creating text box with RMB would not be obvious
    //  since RMB menu most likely hides text box immediately
    //  afterwards)
    if (m_mouseButton == 1)
        return false/*do not create text box*/;


    // Calculate suggested width.
    *minimumWidthOut = calcClickCreateDimension (
        m_startPoint.x (),
            m_currentPoint.x (),
        kpSelection::preferredMinimumWidthForTextStyle (textStyle),
            kpSelection::minimumWidthForTextStyle (textStyle),
        document ()->width ());

    // Calculate suggested height.
    *minimumHeightOut = calcClickCreateDimension (
        m_startPoint.y (),
            m_currentPoint.y (),
        kpSelection::preferredMinimumHeightForTextStyle (textStyle),
            kpSelection::minimumHeightForTextStyle (textStyle),
        document ()->height ());


    return true/*do create text box*/;
}

// protected virtual [base kpToolSelection]
void kpToolText::createMoreSelectionAndUpdateStatusBar (QPoint thisPoint,
        QRect normalizedRect)
{
    const kpTextStyle textStyle = mainWindow ()->textStyle ();


    //
    // Calculate Text Box Rectangle.
    //

    // (will set both variables)
    int minimumWidth = 0, minimumHeight = 0;
    if (!shouldCreate (textStyle, &minimumWidth, &minimumHeight))
        return;


    // Make sure the dragged out rectangle is of the minimum width we just
    // calculated.
    if (normalizedRect.width () < minimumWidth)
    {
        if (thisPoint.x () >= m_startPoint.x ())
            normalizedRect.setWidth (minimumWidth);
        else
            normalizedRect.setX (normalizedRect.right () - minimumWidth + 1);
    }

    // Make sure the dragged out rectangle is of the minimum height we just
    // calculated.
    if (normalizedRect.height () < minimumHeight)
    {
        if (thisPoint.y () >= m_startPoint.y ())
            normalizedRect.setHeight (minimumHeight);
        else
            normalizedRect.setY (normalizedRect.bottom () - minimumHeight + 1);
    }

#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "\t\tnormalizedRect=" << normalizedRect
                << " kpSelection::preferredMinimumSize="
                    << QSize (minimumWidth, minimumHeight)
                << endl;
#endif


    //
    // Construct and Deploy Text Box.
    //
    
    // Create empty text box.
    QList <QString> textLines;
    textLines.append (QString ());
    kpSelection sel (normalizedRect, textLines, textStyle);

    // Create command containing text box.
    if (!m_currentCreateTextCommand)
    {
        m_currentCreateTextCommand = new kpToolSelectionCreateCommand (
            i18n ("Text: Create Box"),
            sel,
            mainWindow ());
    }
    else
        m_currentCreateTextCommand->setFromSelection (sel);

    // Render.
    viewManager ()->setTextCursorPosition (0, 0);
    document ()->setSelection (sel);


    //
    // Update Status Bar.
    //
    
    QPoint actualEndPoint = KP_INVALID_POINT;
    if (m_startPoint == normalizedRect.topLeft ())
        actualEndPoint = normalizedRect.bottomRight ();
    else if (m_startPoint == normalizedRect.bottomRight ())
        actualEndPoint = normalizedRect.topLeft ();
    else if (m_startPoint == normalizedRect.topRight ())
        actualEndPoint = normalizedRect.bottomLeft ();
    else if (m_startPoint == normalizedRect.bottomLeft ())
        actualEndPoint = normalizedRect.topRight ();

    setUserShapePoints (m_startPoint, actualEndPoint);
}

// protected virtual [base kpToolSelection]
void kpToolText::setSelectionBorderForHaventBegunDraw ()
{
    kpToolSelection::setSelectionBorderForHaventBegunDraw ();
    viewManager ()->setTextCursorEnabled (true);
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
    
// protected virtual [base kpToolSelection]
QString kpToolText::nonSmearMoveCommandName () const
{
    return i18n ("Text: Move Box");
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


//
// Text Cursor Calculations (all static, no mutations)
//


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


//
// Key Press Handling
//


// protected virtual [base kpTool]
bool kpToolText::viewEvent (QEvent *e)
{
    const bool isShortcutOverrideEvent =
        (e->type () == QEvent::ShortcutOverride);
    const bool haveTextSelection =
        (document ()->selection () && document ()->selection ()->isText ());

#if DEBUG_KP_TOOL_TEXT && 0
    kDebug () << "kpToolText::viewEvent() type=" << e->type ()
              << " isShortcutOverrideEvent=" << isShortcutOverrideEvent
              << " haveTextSel=" << haveTextSelection
              << endl;
#endif

    if (!isShortcutOverrideEvent || !haveTextSelection)
        return kpToolSelection::viewEvent (e);

    QKeyEvent *ke = static_cast <QKeyEvent *> (e);
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::viewEvent() key=" << ke->key ()
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


// protected
void kpToolText::handleUpKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
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

// protected
void kpToolText::handleDownKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
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

// protected
void kpToolText::handleLeftKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tleft pressed" << endl;
#endif

    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

    if ((e->modifiers () & Qt::ControlModifier) == 0)
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

    e->accept ();
}

// protected
void kpToolText::handleRightKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tright pressed" << endl;
#endif

    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

    if ((e->modifiers () & Qt::ControlModifier) == 0)
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

    e->accept ();
}


// protected
void kpToolText::handleHomeKeyPress (QKeyEvent *e,
    const QList <QString> & /*textLines*/, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\thome pressed" << endl;
#endif

    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

    if (e->modifiers () & Qt::ControlModifier)
        cursorRow = 0;

    cursorCol = 0;

    viewManager ()->setTextCursorPosition (cursorRow, cursorCol);

    e->accept ();
}

// protected
void kpToolText::handleEndKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tend pressed" << endl;
#endif

    if (hasBegunShape ())
        endShape (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

    if (e->modifiers () & Qt::ControlModifier)
        cursorRow = textLines.size () - 1;

    cursorCol = textLines [cursorRow].length ();

    viewManager ()->setTextCursorPosition (cursorRow, cursorCol);

    e->accept ();
}


// protected
void kpToolText::handleBackspaceKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tbackspace pressed" << endl;
#endif

    if ((e->modifiers () & Qt::ControlModifier) == 0)
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

// protected
void kpToolText::handleDeleteKeyPress (QKeyEvent *e,
    const QList <QString> & textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tdelete pressed" << endl;
#endif

    if ((e->modifiers () & Qt::ControlModifier) == 0)
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


// protected
void kpToolText::handleEnterKeyPress (QKeyEvent *e,
    const QList <QString> & /*textLines*/, int cursorRow, int cursorCol)
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
            cursorRow, cursorCol,
            kpToolTextEnterCommand::AddEnterNow,
            mainWindow ());
        commandHistory ()->addCommand (m_enterCommand, false/*no exec*/);
    }
    else
        m_enterCommand->addEnter ();

    e->accept ();
}


// protected
void kpToolText::handleTextTyped (QKeyEvent *e,
    const QList <QString> & /*textLines*/, int cursorRow, int cursorCol)
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
                cursorRow, cursorCol,
                usableText,
                mainWindow ());
            commandHistory ()->addCommand (m_insertCommand, false/*no exec*/);
        }
        else
            m_insertCommand->addText (usableText);

        e->accept ();
    }
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


    kpSelection * const sel = document ()->selection ();

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


    // (All handle.+()'s require this info)
    const QList <QString> textLines = sel->textLines ();
    const int cursorRow = viewManager ()->textCursorRow ();
    const int cursorCol = viewManager ()->textCursorCol ();


    // TODO: KTextEdit::keyPressEvent() uses KStdAccel instead of hardcoding; same fix for kpTool?
    switch (e->key ())
    {
    case Qt::Key_Up:
        handleUpKeyPress (e, textLines, cursorRow, cursorCol);
        break;

    case Qt::Key_Down:
        handleDownKeyPress (e, textLines, cursorRow, cursorCol);
        break;

    case Qt::Key_Left:
        handleLeftKeyPress (e, textLines, cursorRow, cursorCol);
        break;

    case Qt::Key_Right:
        handleRightKeyPress (e, textLines, cursorRow, cursorCol);
        break;


    case Qt::Key_Home:
        handleHomeKeyPress (e, textLines, cursorRow, cursorCol);
        break;

    case Qt::Key_End:
        handleEndKeyPress (e, textLines, cursorRow, cursorCol);
        break;


    case Qt::Key_Backspace:
        handleBackspaceKeyPress (e, textLines, cursorRow, cursorCol);
        break;

    case Qt::Key_Delete:
        handleDeleteKeyPress (e, textLines, cursorRow, cursorCol);
        break;


    case Qt::Key_Enter:
    case Qt::Key_Return:
        handleEnterKeyPress (e, textLines, cursorRow, cursorCol);
        break;


    default:
        handleTextTyped (e, textLines, cursorRow, cursorCol);
        break;
    }


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


//
// Input Method Text Entry
//


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


//
// User Changing Text Style Elements
//


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

    // Figure out old text style.
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

    // Figure out old text style.
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

    // Figure out old text style.
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

    // Figure out old text style.
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

    // Figure out old text style.
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

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setStrikeThru (!isStrikeThru);

    changeTextStyle (i18n ("Text: Strike Through"),
                     newTextStyle,
                     oldTextStyle);
}


#include <kptooltext.moc>
