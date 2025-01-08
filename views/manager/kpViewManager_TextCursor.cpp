
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
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
   SPDX-FileCopyrightText: 2010 Tasuku Suzuki <stasuku@gmail.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_VIEW_MANAGER 0

#include "kpViewManager.h"
#include "kpViewManagerPrivate.h"

#include <QApplication>
#include <QTimer>
// #include <QInputContext>

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "kpDefs.h"
#include "layers/selections/text/kpTextSelection.h"
#include "layers/tempImage/kpTempImage.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"
#include "views/kpView.h"

// public
bool kpViewManager::textCursorEnabled() const
{
    return static_cast<bool>(d->textCursorBlinkTimer);
}

// public
void kpViewManager::setTextCursorEnabled(bool yes)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::setTextCursorEnabled(" << yes << ")";
#endif

    if (yes == textCursorEnabled()) {
        return;
    }

    delete d->textCursorBlinkTimer;
    d->textCursorBlinkTimer = nullptr;

    setFastUpdates();
    setQueueUpdates();
    {
        if (yes) {
            d->textCursorBlinkTimer = new QTimer(this);
            d->textCursorBlinkTimer->setSingleShot(true);
            connect(d->textCursorBlinkTimer, &QTimer::timeout, this, &kpViewManager::slotTextCursorBlink);

            d->textCursorBlinkState = true;
            slotTextCursorBlink();
        } else {
            d->textCursorBlinkState = false;
            updateTextCursor();
        }
    }
    restoreQueueUpdates();
    restoreFastUpdates();
}

// public
bool kpViewManager::textCursorBlinkState() const
{
    return d->textCursorBlinkState;
}

// public
void kpViewManager::setTextCursorBlinkState(bool on)
{
    if (on == d->textCursorBlinkState) {
        return;
    }

    d->textCursorBlinkState = on;

    updateTextCursor();
}

// public
int kpViewManager::textCursorRow() const
{
    return d->textCursorRow;
}

// public
int kpViewManager::textCursorCol() const
{
    return d->textCursorCol;
}

// public
void kpViewManager::setTextCursorPosition(int row, int col)
{
    if (row == d->textCursorRow && col == d->textCursorCol) {
        return;
    }

    setFastUpdates();
    setQueueUpdates();
    {
        // Clear the old cursor.
        d->textCursorBlinkState = false;
        updateTextCursor();

        d->textCursorRow = row;
        d->textCursorCol = col;

        // Render the new cursor.
        d->textCursorBlinkState = true;
        updateTextCursor();
    }
    restoreQueueUpdates();
    restoreFastUpdates();
}

// public
QRect kpViewManager::textCursorRect() const
{
    kpTextSelection *textSel = document()->textSelection();
    if (!textSel) {
        return {};
    }

    QPoint topLeft = textSel->pointForTextRowCol(d->textCursorRow, d->textCursorCol);
    if (topLeft == KP_INVALID_POINT) {
        // Text cursor row/col hasn't been specified yet?
        if (textSel->hasContent()) {
            return {};
        }

        // Empty text box should still display a cursor so that the user
        // knows where typed text will go.
        topLeft = textSel->textAreaRect().topLeft();
    }

    Q_ASSERT(topLeft != KP_INVALID_POINT);

    return {topLeft.x(), topLeft.y(), 1, textSel->textStyle().fontMetrics().height()};
}

// protected
void kpViewManager::updateTextCursor()
{
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::updateTextCursor()";
#endif

    const QRect r = textCursorRect();
    if (!r.isValid()) {
        return;
    }

    setFastUpdates();
    {
        // If !textCursorEnabled(), this will clear.
        updateViews(r);
    }
    restoreFastUpdates();
}

// protected slot
void kpViewManager::slotTextCursorBlink()
{
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::slotTextCursorBlink() cursorBlinkState=" << d->textCursorBlinkState;
#endif

    if (d->textCursorBlinkTimer) {
        // (single shot)
        d->textCursorBlinkTimer->start(QApplication::cursorFlashTime() / 2);
    }

    updateTextCursor();
    // TODO: Shouldn't this be done _before_ updating the text cursor
    //       because textCursorBlinkState() is supposed to reflect what
    //       updateTextCursor() just rendered, until the next timer tick?
    d->textCursorBlinkState = !d->textCursorBlinkState;
}
