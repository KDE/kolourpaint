
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

#define DEBUG_KP_TOOL_TEXT 1


#include <kpToolText.h>
#include <kpToolTextPrivate.h>

#include <KDebug>

#include <kpDocument.h>
#include <kpTextSelection.h>
#include <kpView.h>
#include <kpViewManager.h>


// private
bool kpToolText::onSelectionToSelectText () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v)
        return 0;

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
    // This path is a bit dangerous since we don't call the base
    // implementation.
    //
    // However, we are doing something unusual here in that we aren't
    // drag-moving the selection - therefore it makes sense to not
    // call the base.
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "\t\tis select cursor pos" << endl;
#endif

    Q_ASSERT (document ()->textSelection ());
    viewManager ()->setTextCursorPosition (
        document ()->textSelection ()->closestTextRowForPoint (currentPoint ()),
        document ()->textSelection ()->closestTextColForPoint (currentPoint ()));
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
        Q_ASSERT (!"Unexpected call");
        break;

    case EndDraw:
        // Do nothing.
        break;

    default:
        Q_ASSERT (!"Unhandled operation");
        break;
    }


    return QVariant ();
}
