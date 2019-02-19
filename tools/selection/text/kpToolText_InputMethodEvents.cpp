
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
   Copyright (c) 2010 Tasuku Suzuki <stasuku@gmail.com>
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


#include "tools/selection/text/kpToolText.h"
#include "kpToolTextPrivate.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"

#include <QEvent>

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

//---------------------------------------------------------------------

void kpToolText::inputMethodEvent (QInputMethodEvent *e)
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "kpToolText::inputMethodEvent() preeditString='" << e->preeditString ()
               << "commitString = " << e->commitString ()
               << " replacementStart=" << e->replacementStart ()
               << " replacementLength=" << e->replacementLength ();
#endif
    kpTextSelection *textSel = document ()->textSelection ();
    if (hasBegunDraw() || !textSel)
    {
        e->ignore();
        return;
    }

    kpPreeditText previous = textSel->preeditText ();
    kpPreeditText next (e);

    int textCursorRow = viewManager ()->textCursorRow ();
    int textCursorCol = viewManager ()->textCursorCol ();
    if (!next.isEmpty ())
    {
        if (previous.position().x () < 0 && previous.position().y () < 0)
        {
            next.setPosition (QPoint(textCursorCol, textCursorRow));
        }
        else
        {
            next.setPosition(previous.position ());
        }
    }
    textSel->setPreeditText (next);
    textCursorCol = textCursorCol - previous.cursorPosition () + next.cursorPosition ();
    viewManager ()->setTextCursorPosition (textCursorRow, textCursorCol);

    QString commitString = e->commitString ();
    if (!commitString.isEmpty ())
    {
        // commit string
        if (!d->insertCommand) {
            addNewInsertCommand (&d->insertCommand);
        }

        d->insertCommand->addText (commitString);
    }
    e->accept ();
}

//---------------------------------------------------------------------
