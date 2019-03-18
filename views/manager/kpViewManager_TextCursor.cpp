
// TODO: This is bad design as it's easy to get out of sync with the selection.
//       e.g. You could have textCursorEnabled() but no text selection or
//       vice versa.  And the cursor could be outside of the selection.
//
//       In fact, our text commands momentarily violate these "invariants":
//
//       1. A text box with content must have the cursor somewhere on an
//          existing text line, possibly 1 position after the last character
//          on a line.
//
//       2. Special case: A content-less text box (i.e. no text lines) must
//          have the cursor at (0,0).
//
//       We don't assert-check them at the moment.  We should when we fix
//       the design so that the invariants are always maintained.

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


#define DEBUG_KP_VIEW_MANAGER 0


#include "kpViewManager.h"
#include "kpViewManagerPrivate.h"

#include <QApplication>
#include <QList>
#include <QTimer>
//#include <QInputContext>

#include "kpLogCategories.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "mainWindow/kpMainWindow.h"
#include "layers/tempImage/kpTempImage.h"
#include "layers/selections/text/kpTextSelection.h"
#include "tools/kpTool.h"
#include "views/kpView.h"


// public
bool kpViewManager::textCursorEnabled () const
{
    return static_cast<bool> (d->textCursorBlinkTimer);
}

// public
void kpViewManager::setTextCursorEnabled (bool yes)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::setTextCursorEnabled(" << yes << ")";
#endif

    if (yes == textCursorEnabled ()) {
        return;
    }

    delete d->textCursorBlinkTimer;
    d->textCursorBlinkTimer = nullptr;

    setFastUpdates ();
    setQueueUpdates ();
    {
        if (yes)
        {
            d->textCursorBlinkTimer = new QTimer (this);
            d->textCursorBlinkTimer->setSingleShot (true);
            connect (d->textCursorBlinkTimer, &QTimer::timeout, this, &kpViewManager::slotTextCursorBlink);

            d->textCursorBlinkState = true;
            slotTextCursorBlink ();
        }
        else
        {
            d->textCursorBlinkState = false;
            updateTextCursor ();
        }
    }
    restoreQueueUpdates ();
    restoreFastUpdates ();
}


// public
bool kpViewManager::textCursorBlinkState () const
{
    return d->textCursorBlinkState;
}

// public
void kpViewManager::setTextCursorBlinkState (bool on)
{
    if (on == d->textCursorBlinkState) {
        return;
    }

    d->textCursorBlinkState = on;

    updateTextCursor ();
}


// public
int kpViewManager::textCursorRow () const
{
    return d->textCursorRow;
}

// public
int kpViewManager::textCursorCol () const
{
    return d->textCursorCol;
}

// public
void kpViewManager::setTextCursorPosition (int row, int col)
{
    if (row == d->textCursorRow && col == d->textCursorCol) {
        return;
    }

    setFastUpdates ();
    setQueueUpdates ();
    {
        // Clear the old cursor.
        d->textCursorBlinkState = false;
        updateTextCursor ();

        d->textCursorRow = row;
        d->textCursorCol = col;

        // Render the new cursor.
        d->textCursorBlinkState = true;
        updateTextCursor ();
    }
    restoreQueueUpdates ();
    restoreFastUpdates ();
}


// public
QRect kpViewManager::textCursorRect () const
{
    kpTextSelection *textSel = document ()->textSelection ();
    if (!textSel) {
        return {};
    }

    QPoint topLeft = textSel->pointForTextRowCol (d->textCursorRow, d->textCursorCol);
    if (topLeft == KP_INVALID_POINT)
    {
        // Text cursor row/col hasn't been specified yet?
        if (textSel->hasContent ()) {
            return {};
        }

        // Empty text box should still display a cursor so that the user
        // knows where typed text will go.
        topLeft = textSel->textAreaRect ().topLeft ();
    }

    Q_ASSERT (topLeft != KP_INVALID_POINT);

    return  {topLeft.x (), topLeft.y (),
                1, textSel->textStyle ().fontMetrics ().height ()};
}


// protected
void kpViewManager::updateTextCursor ()
{
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::updateTextCursor()";
#endif

    const QRect r = textCursorRect ();
    if (!r.isValid ()) {
        return;
    }

    setFastUpdates ();
    {
        // If !textCursorEnabled(), this will clear.
        updateViews (r);
    }
    restoreFastUpdates ();
}

// protected slot
void kpViewManager::slotTextCursorBlink ()
{
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::slotTextCursorBlink() cursorBlinkState="
               << d->textCursorBlinkState;
#endif

    if (d->textCursorBlinkTimer)
    {
        // (single shot)
        d->textCursorBlinkTimer->start (QApplication::cursorFlashTime () / 2);
    }

    updateTextCursor ();
    // TODO: Shouldn't this be done _before_ updating the text cursor
    //       because textCursorBlinkState() is supposed to reflect what
    //       updateTextCursor() just rendered, until the next timer tick?
    d->textCursorBlinkState = !d->textCursorBlinkState;
}
