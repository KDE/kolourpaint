
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


#include "kpToolText.h"
#include "kpToolTextPrivate.h"
#include "kpLogCategories.h"

#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/text/kpToolTextBackspaceCommand.h"
#include "commands/tools/selection/text/kpToolTextDeleteCommand.h"
#include "commands/tools/selection/text/kpToolTextEnterCommand.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "views/manager/kpViewManager.h"

#include <KLocalizedString>

// private
void kpToolText::endTypingCommands ()
{
    d->insertCommand = nullptr;
    d->enterCommand = nullptr;

    d->backspaceCommand = nullptr;
    d->backspaceWordCommand = nullptr;

    d->deleteCommand = nullptr;
    d->deleteWordCommand = nullptr;
}


// private
void kpToolText::addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd)
{
    // TODO: why not endShapeInternal(); ditto for everywhere else in kpToolText*.cpp?
    if (hasBegunShape ())
    {
        endShape (currentPoint (), normalizedRect ());
    }

    giveContentIfNeeded ();

    *cmd = new kpToolTextBackspaceCommand (i18n ("Text: Backspace"),
        viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
        kpToolTextBackspaceCommand::DontAddBackspaceYet,
        environ ()->commandEnvironment ());
    addNeedingContentCommand (*cmd);
}

// private
void kpToolText::addNewDeleteCommand (kpToolTextDeleteCommand **cmd)
{
    if (hasBegunShape ())
    {
        endShape (currentPoint (), normalizedRect ());
    }

    giveContentIfNeeded ();

    *cmd = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
        viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
        kpToolTextDeleteCommand::DontAddDeleteYet,
        environ ()->commandEnvironment ());
    addNeedingContentCommand (*cmd);
}

// private
void kpToolText::addNewEnterCommand (kpToolTextEnterCommand **cmd)
{
    if (hasBegunShape ())
    {
        endShape (currentPoint (), normalizedRect ());
    }

    giveContentIfNeeded ();

    *cmd = new kpToolTextEnterCommand (i18n ("Text: New Line"),
        viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
        kpToolTextEnterCommand::DontAddEnterYet,
        environ ()->commandEnvironment ());
    addNeedingContentCommand (*cmd);
}

// private
void kpToolText::addNewInsertCommand (kpToolTextInsertCommand **cmd)
{
    if (hasBegunShape ())
    {
        endShape (currentPoint (), normalizedRect ());
    }

    giveContentIfNeeded ();

    *cmd = new kpToolTextInsertCommand (i18n ("Text: Write"),
        viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
        QString (),
        environ ()->commandEnvironment ());
    addNeedingContentCommand (*cmd);
}
