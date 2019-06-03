
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


#ifndef KP_VIEW_MANAGER_H
#define KP_VIEW_MANAGER_H


#include <QObject>


class QCursor;
class QRegion;
class QRect;

class kpDocument;
class kpView;
class kpMainWindow;
class kpTempImage;


class kpViewManager : public QObject
{
Q_OBJECT

public:
    kpViewManager (kpMainWindow *mainWindow);
    ~kpViewManager () override;


private:
    kpDocument *document () const;


//
// Registering views
//

public:
    void registerView (kpView *view);
    void unregisterView (kpView *view);
    void unregisterAllViews ();


//
// View
//

public:
    kpView *viewUnderCursor (bool usingQt = false) const;

    //
    // QWidget::hasMouse() is unreliable:
    //
    // "bool QWidget::hasMouse () const
    //  ... See the "underMouse" property for details.
    //         .
    //         .
    //         .
    //  bool underMouse
    //  ... This value is not updated properly during drag and drop operations."
    //
    // i.e. it's possible that hasMouse() returns false in a mousePressEvent()!
    //
    // This hack needs to be called from kpView so that viewUnderCursor() works
    // as a reasonable replacement (although there is at least one case where
    // it still won't work - just after a fake drag onto the view).
    //
    void setViewUnderCursor (kpView *view);


public:
    // Returns whether at least 1 view has keyboard focus.
    // A pointer is not returned to such a view because more than one can
    // have focus at the same time (see QWidget::isActiveWindow()).
    bool hasAViewWithFocus () const;


//
// Mouse Cursors
//

public:
    void setCursor (const QCursor &cursor);
    void unsetCursor ();


//
// Temp Image
//

public:
    const kpTempImage *tempImage () const;
    void setTempImage (const kpTempImage &tempImage);
    void invalidateTempImage ();


//
// Selections
//

public:
    bool selectionBorderVisible () const;
    void setSelectionBorderVisible (bool yes = true);

    bool selectionBorderFinished () const;
    void setSelectionBorderFinished (bool yes = true);


//
// Text Cursor
//

public:
    // If you enable the text cursor, a timer will start ticking to update
    // the text cursor.  If no text selection is active, the update will
    // be a NOP but the timer will still tick and textCursorEnabled() will
    // still return true.
    bool textCursorEnabled () const;
    void setTextCursorEnabled (bool yes = true);

    bool textCursorBlinkState () const;
    void setTextCursorBlinkState (bool on = true);

    // By convention, a text box with no content (i.e. no text lines) should
    // have a cursor position of (row=0,col=0).  Some code assumes this.
    //
    // (no error checking is performed - the row and col are as per
    //  setTextCursorPosition() so may be out of the bounds of the
    //  text selection)
    int textCursorRow () const;
    int textCursorCol () const;
    // See kpToolText::beginDrawSelectText() for a correct use of this
    // method, satisfying the above convention.
    //
    // WARNING: If the previous row and column are invalid (e.g. you just
    //          called kpTextSelection::setTextLines() such that the previous
    //          row and column now point outside the lines), this will not
    //          be able to erase the cursor at the old position.
    //
    //          Always ensure that the text cursor position is valid.
    //          TODO: We need to check this in all source files.
    //                e.g. It's definitely wrong for kpToolTextBackspaceCommand.
    void setTextCursorPosition (int row, int col);

    // Returns the document rectangle where cursor would be placed, using
    // textCursorRow() and textCursorCol ().
    //
    // For a text selection without any content, this returns a rectangle
    // corresponding to the first text row and column.
    //
    // If there is no text selection or textCursorRow() or
    // textCursorCol() are invalid, it returns an empty rectangle.
    QRect textCursorRect () const;

protected:
    // If textCursorRect() is valid, updates all views at that rectangle.
    // The cursor blink state and timer are not affected at all.
    // TODO: This and other methods will happily execute even if
    //       !textCursorEnabled().  We should fix this.
    void updateTextCursor ();

protected slots:
    void slotTextCursorBlink ();


//
// View Updates
//

public:
    // Specifies whether to queue _all_ paint events
    // (generated by you or the window system), until the
    // corresponding call to restoreQueueUpdates().  Use this
    // before multiple, big, non-interactive changes to the
    // document to eliminate virtually all flicker.
    //
    // This is better than QWidget::setUpdatesEnabled() because
    // restoreQueueUpdates() automatically restores, for each view,
    // only the _regions_ that need to be repainted.
    //
    // You can nest blocks of setQueueUpdates()/restoreQueueUpdates().
    bool queueUpdates () const;
    void setQueueUpdates ();
    void restoreQueueUpdates ();

public:
    // Controls behaviour of updateViews():
    //
    // Slow: Let Qt buffer paint events via QWidget::update().
    //       Results in less flicker.  Paint events are probably merged
    //       so long-term efficiency is increased at the expense of
    //       reduced responsiveness (default).  Generally, the paint
    //       event happens a while later -- when you return to the event
    //       loop.
    // Fast: Force Qt to redraw immediately.  No paint events
    //       are merged so there is great potential for flicker,
    //       if used inappropriately.  Use this when the redraw
    //       area is small and responsiveness is critical.
    //       Continual use of this mode can result in
    //       unnecessary redraws and incredibly slugish performance.
    //
    // You can nest blocks of setFastUpdates()/restoreFastUpdates().
    bool fastUpdates () const;
    void setFastUpdates ();
    void restoreFastUpdates ();

public slots:
    void updateView (kpView *v);
    void updateView (kpView *v, const QRect &viewRect);
    void updateView (kpView *v, int x, int y, int w, int h);
    void updateView (kpView *v, const QRegion &viewRegion);

    void updateViewRectangleEdges (kpView *v, const QRect &viewRect);

    void updateViews (const QRect &docRect);


public slots:
    void adjustViewsToEnvironment ();

public slots:
    void setInputMethodEnabled (bool inputMethodEnabled);

private:
    struct kpViewManagerPrivate * const d;
};


#endif  // KP_VIEW_MANAGER_H
