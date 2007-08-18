
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

#define DEBUG_KP_TOOL_TEXT 0


#include <kpToolText.h>
#include <kpToolTextPrivate.h>

#include <qevent.h>
#include <qlist.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpBug.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpTextSelection.h>
#include <kpToolTextBackspaceCommand.h>
#include <kpToolTextChangeStyleCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionEnvironment.h>
#include <kpToolTextDeleteCommand.h>
#include <kpToolTextEnterCommand.h>
#include <kpToolTextInsertCommand.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


// protected virtual [base kpTool]
bool kpToolText::viewEvent (QEvent *e)
{
    const bool isShortcutOverrideEvent =
        (e->type () == QEvent::ShortcutOverride);
    const bool haveTextSelection = document ()->textSelection ();

#if DEBUG_KP_TOOL_TEXT && 0
    kDebug () << "kpToolText::viewEvent() type=" << e->type ()
              << " isShortcutOverrideEvent=" << isShortcutOverrideEvent
              << " haveTextSel=" << haveTextSelection
              << endl;
#endif

    if (!isShortcutOverrideEvent || !haveTextSelection)
        return kpAbstractSelectionTool::viewEvent (e);

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
        kDebug () << "\tcan't be shortcut - safe to not react";
    #endif
    }
    // Normal letter (w/ or w/o shift, keypad button ok)?
    // TODO: don't like this check
    else if ((ke->modifiers () &
                (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) == 0 &&
            ke->key () < 0x100 /*QChar (ke->key ()).isPrint () - unfortunately F1 is printable too...*/)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tis text - grab";
    #endif
        e->accept ();
    }
    else
    {
        // Strickly speaking, we should grab stuff like the arrow keys
        // and enter.  In any case, should be done down in kpTool (as that
        // uses arrow keys too).
    }

    return kpAbstractSelectionTool::event (e);
}


// protected
void kpToolText::handleUpKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tup pressed";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), normalizedRect ());

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
    kDebug () << "\tdown pressed";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), normalizedRect ());

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
    kDebug () << "\tleft pressed";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), normalizedRect ());

    if ((e->modifiers () & Qt::ControlModifier) == 0)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tmove single char";
    #endif

        MoveCursorLeft (textLines, &cursorRow, &cursorCol);
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }
    else
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tmove to start of word";
    #endif

        MoveCursorToWordStart (textLines, &cursorRow, &cursorCol);
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }

    e->accept ();
}

// protected
void kpToolText::handleRightKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tright pressed";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), normalizedRect ());

    if ((e->modifiers () & Qt::ControlModifier) == 0)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tmove single char";
    #endif

        MoveCursorRight (textLines, &cursorRow, &cursorCol);
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }
    else
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tmove to start of next word";
    #endif

        MoveCursorToNextWordStart (textLines, &cursorRow, &cursorCol);
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }

    e->accept ();
}


// protected
void kpToolText::handleHomeKeyPress (QKeyEvent *e,
    const QList <QString> & /*textLines*/, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\thome pressed";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), normalizedRect ());

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
    kDebug () << "\tend pressed";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), normalizedRect ());

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
    kDebug () << "\tbackspace pressed";
#endif

    if ((e->modifiers () & Qt::ControlModifier) == 0)
    {
        if (!d->backspaceCommand)
            addNewBackspaceCommand (&d->backspaceCommand);

        d->backspaceCommand->addBackspace ();
    }
    else
    {
        if (!d->backspaceWordCommand)
            addNewBackspaceCommand (&d->backspaceWordCommand);

        const int numMoves = MoveCursorToWordStart (textLines,
            &cursorRow, &cursorCol);

        viewManager ()->setQueueUpdates ();
        {
            for (int i = 0; i < numMoves; i++)
                d->backspaceWordCommand->addBackspace ();
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
    kDebug () << "\tdelete pressed";
#endif

    if ((e->modifiers () & Qt::ControlModifier) == 0)
    {
        if (!d->deleteCommand)
            addNewDeleteCommand (&d->deleteCommand);

        d->deleteCommand->addDelete ();
    }
    else
    {
        if (!d->deleteWordCommand)
            addNewDeleteCommand (&d->deleteWordCommand);

        // We don't want to know the cursor pos of the next word start
        // as delete should keep cursor in same pos.
        int cursorRowThrowAway = cursorRow,
            cursorColThrowAway = cursorCol;
        const int numMoves = MoveCursorToNextWordStart (textLines,
            &cursorRowThrowAway, &cursorColThrowAway);

        viewManager ()->setQueueUpdates ();
        {
            for (int i = 0; i < numMoves; i++)
                d->deleteWordCommand->addDelete ();
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
    const QList <QString> & /*textLines*/, int /*cursorRow*/, int /*cursorCol*/)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tenter pressed";
#endif
    if (!d->enterCommand)
        addNewEnterCommand (&d->enterCommand);
        
    d->enterCommand->addEnter ();

    e->accept ();
}


// protected
void kpToolText::handleTextTyped (QKeyEvent *e,
    const QList <QString> & /*textLines*/, int /*cursorRow*/, int /*cursorCol*/)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\ttext='" << e->text () << "'";
#endif
    QString usableText;
    for (int i = 0; i < (int) e->text ().length (); i++)
    {
        if (e->text ().at (i).isPrint ())
            usableText += e->text ().at (i);
    }
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\tusableText='" << usableText << "'";
#endif

    if (usableText.length () > 0)
    {
        if (!d->insertCommand)
            addNewInsertCommand (&d->insertCommand);
            
        d->insertCommand->addText (usableText);

        e->accept ();
    }
}


// protected virtual [base kpAbstractSelectionTool]
void kpToolText::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::keyPressEvent(e->text='" << e->text () << "')";
#endif


    e->ignore ();


    if (hasBegunDraw ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\talready began draw with mouse - passing on event to kpTool";
    #endif
        kpAbstractSelectionTool::keyPressEvent (e);
        return;
    }


    kpTextSelection * const textSel = document ()->textSelection ();

    if (!textSel)
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tno text sel - passing on event to kpTool";
    #endif
        //if (hasBegunShape ())
        //    endShape (currentPoint (), normalizedRect ());

        kpAbstractSelectionTool::keyPressEvent (e);
        return;
    }


    // (All handle.+()'s require this info)
    const QList <QString> textLines = textSel->textLines ();
    const int cursorRow = viewManager ()->textCursorRow ();
    const int cursorCol = viewManager ()->textCursorCol ();


    // TODO: KTextEdit::keyPressEvent() uses KStandardShortcut instead of hardcoding; same fix for kpTool?
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
                   << "') - passing on event to kpAbstractSelectionTool"
                   << endl;
    #endif
        //if (hasBegunShape ())
        //    endShape (currentPoint (), normalizedRect ());

        kpAbstractSelectionTool::keyPressEvent (e);
        return;
    }
}
