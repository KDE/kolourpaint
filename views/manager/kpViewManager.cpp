
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


#define DEBUG_KP_VIEW_MANAGER 0


#include <kpViewManager.h>
#include <kpViewManagerPrivate.h>

#include <qapplication.h>
#include <qlist.h>
#include <qtimer.h>

#include <kdebug.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpMainWindow.h>
#include <kpTempImage.h>
#include <kpTextSelection.h>
#include <kpTool.h>
#include <kpView.h>

//---------------------------------------------------------------------

kpViewManager::kpViewManager (kpMainWindow *mainWindow)
    : d (new kpViewManagerPrivate ())
{
    Q_ASSERT (mainWindow);

    d->mainWindow = mainWindow;

    // d->views
    d->viewUnderCursor = 0;

    // d->cursor

    d->tempImage = 0;

    d->selectionBorderVisible = false;
    d->selectionBorderFinished = false;


    d->textCursorBlinkTimer = 0;

    d->textCursorRow = -1;
    d->textCursorCol = -1;

    d->textCursorBlinkState = true;


    d->queueUpdatesCounter = d->fastUpdatesCounter = 0;

    d->inputMethodEnabled = false;
}

//---------------------------------------------------------------------

kpViewManager::~kpViewManager ()
{
    unregisterAllViews ();

    delete d->tempImage;
    delete d;
}

//---------------------------------------------------------------------

// private
kpDocument *kpViewManager::document () const
{
    return d->mainWindow->document ();
}

//---------------------------------------------------------------------

// public
void kpViewManager::registerView (kpView *view)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    kDebug () << "kpViewManager::registerView (" << view << ")";
#endif
    Q_ASSERT (view);
    Q_ASSERT (!d->views.contains (view));

#if DEBUG_KP_VIEW_MANAGER && 1
    kDebug () << "\tadded view";
#endif
    view->setCursor (d->cursor);
    d->views.append (view);
}

//---------------------------------------------------------------------

// public
void kpViewManager::unregisterView (kpView *view)
{
    Q_ASSERT (view);
    Q_ASSERT (d->views.contains (view));

    if (view == d->viewUnderCursor)
        d->viewUnderCursor = 0;

    view->unsetCursor ();
    d->views.removeAll (view);
}

//---------------------------------------------------------------------

// public
void kpViewManager::unregisterAllViews ()
{
    d->views.clear ();
}

//---------------------------------------------------------------------

// public
kpView *kpViewManager::viewUnderCursor (bool usingQt) const
{
    if (!usingQt)
    {
        Q_ASSERT (!d->viewUnderCursor || d->views.contains (d->viewUnderCursor));
        return d->viewUnderCursor;
    }
    else
    {
        for (QLinkedList <kpView *>::const_iterator it = d->views.begin ();
             it != d->views.end ();
             ++it)
        {
            if ((*it)->underMouse ())
                return (*it);
        }

        return 0;
    }
}

//---------------------------------------------------------------------

// public
void kpViewManager::setViewUnderCursor (kpView *view)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    kDebug () << "kpViewManager::setViewUnderCursor ("
               << (view ? view->objectName () : "(none)") << ")"
               << "  old=" << (d->viewUnderCursor ? d->viewUnderCursor->objectName () : "(none)")
               << endl;
#endif
    if (view == d->viewUnderCursor)
        return;

    d->viewUnderCursor = view;

    if (d->viewUnderCursor)
        d->viewUnderCursor->setAttribute (Qt::WA_InputMethodEnabled, d->inputMethodEnabled);

    if (!d->viewUnderCursor)
    {
        // Hide the brush if the mouse cursor just left the view.
        if (d->tempImage && d->tempImage->isBrush ())
        {
        #if DEBUG_KP_VIEW_MANAGER && 1
            kDebug () << "\thiding brush pixmap since cursor left view";
        #endif
            updateViews (d->tempImage->rect ());
        }
    }
    else
    {
        if (d->mainWindow->tool ())
        {
        #if DEBUG_KP_VIEW_MANAGER && 1
            kDebug () << "\tnotify tool that something changed below cursor";
        #endif
            d->mainWindow->tool ()->somethingBelowTheCursorChanged ();
        }
    }
}

//---------------------------------------------------------------------

// public
bool kpViewManager::hasAViewWithFocus () const
{
    for (QLinkedList <kpView *>::const_iterator it = d->views.begin ();
         it != d->views.end ();
         ++it)
    {
        if ((*it)->isActiveWindow ())
            return true;
    }

    return false;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setCursor (const QCursor &cursor)
{
    for (QLinkedList <kpView *>::const_iterator it = d->views.begin ();
         it != d->views.end ();
         ++it)
    {
        (*it)->setCursor (cursor);
    }

    d->cursor = cursor;
}

//---------------------------------------------------------------------

// public
void kpViewManager::unsetCursor ()
{
    for (QLinkedList <kpView *>::const_iterator it = d->views.begin ();
         it != d->views.end ();
         ++it)
    {
        (*it)->unsetCursor ();
    }

    d->cursor = QCursor ();
}

//---------------------------------------------------------------------

// public
const kpTempImage *kpViewManager::tempImage () const
{
    return d->tempImage;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setTempImage (const kpTempImage &tempImage)
{
#if DEBUG_KP_VIEW_MANAGER
    kDebug () << "kpViewManager::setTempImage(isBrush="
               << tempImage.isBrush ()
               << ",topLeft=" << tempImage.topLeft ()
               << ",image.rect=" << tempImage.image ().rect ()
               << ")" << endl;
#endif

    QRect oldRect;

    if (d->tempImage)
    {
        oldRect = d->tempImage->rect ();
        delete d->tempImage;
        d->tempImage = 0;
    }

    d->tempImage = new kpTempImage (tempImage);

    setQueueUpdates ();
    {
        if (oldRect.isValid ())
            updateViews (oldRect);
        updateViews (d->tempImage->rect ());
    }
    restoreQueueUpdates ();
}

//---------------------------------------------------------------------

// public
void kpViewManager::invalidateTempImage ()
{
    if (!d->tempImage)
        return;

    QRect oldRect = d->tempImage->rect ();

    delete d->tempImage;
    d->tempImage = 0;

    updateViews (oldRect);
}

//---------------------------------------------------------------------

// public
bool kpViewManager::selectionBorderVisible () const
{
    return d->selectionBorderVisible;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setSelectionBorderVisible (bool yes)
{
    if (d->selectionBorderVisible == yes)
        return;

    d->selectionBorderVisible = yes;

    if (document ()->selection ())
        updateViews (document ()->selection ()->boundingRect ());
}

//---------------------------------------------------------------------

// public
bool kpViewManager::selectionBorderFinished () const
{
    return d->selectionBorderFinished;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setSelectionBorderFinished (bool yes)
{
    if (d->selectionBorderFinished == yes)
        return;

    d->selectionBorderFinished = yes;

    if (document ()->selection ())
        updateViews (document ()->selection ()->boundingRect ());
}

//---------------------------------------------------------------------

void kpViewManager::setInputMethodEnabled (bool inputMethodEnabled)
{
    d->inputMethodEnabled = inputMethodEnabled;
    if (d->viewUnderCursor)
        d->viewUnderCursor->setAttribute (Qt::WA_InputMethodEnabled, inputMethodEnabled);
}

//---------------------------------------------------------------------

#include <kpViewManager.moc>
