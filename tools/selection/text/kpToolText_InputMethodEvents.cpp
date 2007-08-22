
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
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


// COMPAT: Need to update InputMethod support.
//
// This code needs lots of updating not just to Qt/KDE4 but also to changes
// in KolourPaint internals.
#if 0

void kpToolText::imStartEvent (QIMEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::imStartEvent() text='" << e->text ()
               << " cursorPos=" << e->cursorPos ()
               << " selectionLength=" << e->selectionLength ()
               << endl;
#endif

    kpTextSelection *textSel = document ()->textSelection ();
    if (hasBegunDraw() || !textSel)
    {
        e->ignore();
        return;
    }

    d->IMStartCursorRow = viewManager ()->textCursorRow ();
    d->IMStartCursorCol = viewManager ()->textCursorCol ();
    d->IMPreeditStr.clear ();
}

void kpToolText::imComposeEvent (QIMEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::imComposeEvent() text='" << e->text ()
               << " cursorPos=" << e->cursorPos ()
               << " selectionLength=" << e->selectionLength ()
               << endl;
#endif

    kpTextSelection *textSel = document ()->textSelection ();
    if (hasBegunDraw() || !textSel)
    {
        e->ignore();
        return;
    }

    // remove old preedit
    if (d->IMPreeditStr.length() > 0 )
    {
        // set cursor at the start input point
        viewManager ()->setTextCursorPosition (d->IMStartCursorRow, d->IMStartCursorCol);
        for (unsigned int i = 0; i < d->IMPreeditStr.length(); i++)
        {
            if (!d->deleteCommand)
            {
                if (hasBegunShape ())
                    endShape (currentPoint (), normalizedRect ());

                d->deleteCommand = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                    viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                    kpToolTextDeleteCommand::AddDeleteNow,
                    environ ()->commandEnvironment ());
                commandHistory ()->addCommand (d->deleteCommand, false/*no exec*/);
            }
            else
                d->deleteCommand->addDelete ();
        }
    }

    // insert new preedit
    d->IMPreeditStr = e->text();
    if (d->IMPreeditStr.length() > 0)
    {
        if (!d->insertCommand)
        {
            if (hasBegunShape ())
                endShape (currentPoint (), normalizedRect ());

            d->insertCommand = new kpToolTextInsertCommand (i18n ("Text: Write"),
                                                           viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                                                           d->IMPreeditStr,
                                                           environ ()->commandEnvironment ());
            commandHistory ()->addCommand (d->insertCommand, false/*no exec*/);
        }
        else
            d->insertCommand->addText (d->IMPreeditStr);
    }

    // set cursor pos
    if (d->IMStartCursorRow >= 0)
    {
        int row = d->IMStartCursorRow;
        int col = d->IMStartCursorCol + e->cursorPos () /* + e->selectionLength()*/;
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

    kpTextSelection *textSel = document ()->textSelection ();
    if (hasBegunDraw() || !textSel)
    {
        e->ignore();
        return;
    }

    // remove old preedit
    if (d->IMPreeditStr.length() > 0 )
    {
        // set cursor at the start input point
        viewManager ()->setTextCursorPosition (d->IMStartCursorRow, d->IMStartCursorCol);
        for (unsigned int i = 0; i < d->IMPreeditStr.length(); i++)
        {
            if (!d->deleteCommand)
            {
                if (hasBegunShape ())
                    endShape (currentPoint (), normalizedRect ());

                d->deleteCommand = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                    viewManager ()->textCursorRow (),
                    viewManager ()->textCursorCol (),
                    kpToolTextDeleteCommand::AddDeleteNow,
                    environ ()->commandEnvironment ());
                commandHistory ()->addCommand (d->deleteCommand, false/*no exec*/);
            }
            else
                d->deleteCommand->addDelete ();
        }
    }
    d->IMPreeditStr.clear ();

    // commit string
    QString inputStr = e->text();
    if (inputStr.length() > 0)
    {
        if (!d->insertCommand)
        {
            if (hasBegunShape ())
                endShape (currentPoint (), normalizedRect ());

            d->insertCommand = new kpToolTextInsertCommand (i18n ("Text: Write"),
                                                           viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                                                           inputStr,
                                                           environ ()->commandEnvironment ());
            commandHistory ()->addCommand (d->insertCommand, false/*no exec*/);
        }
        else
            d->insertCommand->addText (inputStr);
    }
}

#endif  // COMPAT
