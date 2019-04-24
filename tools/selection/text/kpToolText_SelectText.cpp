
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


#include "tools/selection/text/kpToolText.h"
#include "kpToolTextPrivate.h"

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"


// private
bool kpToolText::onSelectionToSelectText () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v) {
        return false;
    }

    return v->mouseOnSelectionToSelectText (currentViewPoint ());
}


// private
QString kpToolText::haventBegunDrawUserMessageSelectText () const
{
    return i18n ("Left click to change cursor position.");
}

// private
void kpToolText::setCursorSelectText ()
{
    viewManager ()->setCursor (Qt::IBeamCursor);
}


// private
void kpToolText::beginDrawSelectText ()
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\t\tis select cursor pos";
#endif
    kpTextSelection *textSel = document ()->textSelection ();
    Q_ASSERT (textSel);

    int newRow, newCol;

    if (textSel->hasContent ())
    {
        newRow = textSel->closestTextRowForPoint (currentPoint ());
        newCol = textSel->closestTextColForPoint (currentPoint ());
    }
    else
    {
        newRow = newCol = 0;
    }

#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\t\t\told: row=" << viewManager ()->textCursorRow ()
              << "col=" << viewManager ()->textCursorCol ();
    qCDebug(kpLogTools) << "\t\t\tnew: row=" << newRow << "col=" << newCol;
#endif
    viewManager ()->setTextCursorPosition (newRow, newCol);
}


// protected virtual
QVariant kpToolText::selectTextOperation (Operation op,
        const QVariant &data1, const QVariant &data2)
{
    (void) data1;
    (void) data2;

    
    switch (op)
    {
    case HaventBegunDrawUserMessage:
        return haventBegunDrawUserMessageSelectText ();

    case SetCursor:
        setCursorSelectText ();
        break;

    case BeginDraw:
        beginDrawSelectText ();
        break;

    case Draw:
        // Do nothing.
        break;

    case Cancel:
        // Not called.  REFACTOR: Change this?
        break;

    case EndDraw:
        // Do nothing.
        break;

    default:
        Q_ASSERT (!"Unhandled operation");
        break;
    }


    return {};
}
