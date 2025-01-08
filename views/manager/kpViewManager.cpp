
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_VIEW_MANAGER 0

#include "views/manager/kpViewManager.h"
#include "kpViewManagerPrivate.h"

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "kpDefs.h"
#include "layers/selections/text/kpTextSelection.h"
#include "layers/tempImage/kpTempImage.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"
#include "views/kpView.h"

//---------------------------------------------------------------------

kpViewManager::kpViewManager(kpMainWindow *mainWindow)
    : d(new kpViewManagerPrivate())
{
    Q_ASSERT(mainWindow);

    d->mainWindow = mainWindow;

    // d->views
    d->viewUnderCursor = nullptr;

    // d->cursor

    d->tempImage = nullptr;

    d->selectionBorderVisible = false;
    d->selectionBorderFinished = false;

    d->textCursorBlinkTimer = nullptr;

    d->textCursorRow = -1;
    d->textCursorCol = -1;

    d->textCursorBlinkState = true;

    d->queueUpdatesCounter = d->fastUpdatesCounter = 0;

    d->inputMethodEnabled = false;
}

//---------------------------------------------------------------------

kpViewManager::~kpViewManager()
{
    unregisterAllViews();

    delete d->tempImage;
    delete d;
}

//---------------------------------------------------------------------

// private
kpDocument *kpViewManager::document() const
{
    return d->mainWindow->document();
}

//---------------------------------------------------------------------

// public
void kpViewManager::registerView(kpView *view)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::registerView (" << view << ")";
#endif
    Q_ASSERT(view);
    Q_ASSERT(!d->views.contains(view));

#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "\tadded view";
#endif
    view->setCursor(d->cursor);
    d->views.append(view);
}

//---------------------------------------------------------------------

// public
void kpViewManager::unregisterView(kpView *view)
{
    Q_ASSERT(view);
    Q_ASSERT(d->views.contains(view));

    if (view == d->viewUnderCursor) {
        d->viewUnderCursor = nullptr;
    }

    view->unsetCursor();
    d->views.removeAll(view);
}

//---------------------------------------------------------------------

// public
void kpViewManager::unregisterAllViews()
{
    d->views.clear();
}

//---------------------------------------------------------------------

// public
kpView *kpViewManager::viewUnderCursor(bool usingQt) const
{
    if (!usingQt) {
        Q_ASSERT(!d->viewUnderCursor || d->views.contains(d->viewUnderCursor));
        return d->viewUnderCursor;
    }

    for (kpView *view : std::as_const(d->views))
        if (view->underMouse())
            return view;

    return nullptr;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setViewUnderCursor(kpView *view)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::setViewUnderCursor (" << (view ? view->objectName() : "(none)") << ")"
                        << "  old=" << (d->viewUnderCursor ? d->viewUnderCursor->objectName() : "(none)");
#endif
    if (view == d->viewUnderCursor) {
        return;
    }

    d->viewUnderCursor = view;

    if (d->viewUnderCursor) {
        d->viewUnderCursor->setAttribute(Qt::WA_InputMethodEnabled, d->inputMethodEnabled);
    }

    if (!d->viewUnderCursor) {
        // Hide the brush if the mouse cursor just left the view.
        if (d->tempImage && d->tempImage->isBrush()) {
#if DEBUG_KP_VIEW_MANAGER && 1
            qCDebug(kpLogViews) << "\thiding brush pixmap since cursor left view";
#endif
            updateViews(d->tempImage->rect());
        }
    } else {
        if (d->mainWindow->tool()) {
#if DEBUG_KP_VIEW_MANAGER && 1
            qCDebug(kpLogViews) << "\tnotify tool that something changed below cursor";
#endif
            d->mainWindow->tool()->somethingBelowTheCursorChanged();
        }
    }
}

//---------------------------------------------------------------------

// public
bool kpViewManager::hasAViewWithFocus() const
{
    for (kpView *view : std::as_const(d->views))
        if (view->isActiveWindow())
            return true;

    return false;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setCursor(const QCursor &cursor)
{
    for (kpView *view : std::as_const(d->views))
        view->setCursor(cursor);

    d->cursor = cursor;
}

//---------------------------------------------------------------------

// public
void kpViewManager::unsetCursor()
{
    for (kpView *view : std::as_const(d->views))
        view->unsetCursor();

    d->cursor = QCursor();
}

//---------------------------------------------------------------------

// public
const kpTempImage *kpViewManager::tempImage() const
{
    return d->tempImage;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setTempImage(const kpTempImage &tempImage)
{
#if DEBUG_KP_VIEW_MANAGER
    qCDebug(kpLogViews) << "kpViewManager::setTempImage(isBrush=" << tempImage.isBrush() << ",topLeft=" << tempImage.topLeft()
                        << ",image.rect=" << tempImage.image().rect() << ")";
#endif

    QRect oldRect;

    if (d->tempImage) {
        oldRect = d->tempImage->rect();
        delete d->tempImage;
        d->tempImage = nullptr;
    }

    d->tempImage = new kpTempImage(tempImage);

    setQueueUpdates();
    {
        if (oldRect.isValid()) {
            updateViews(oldRect);
        }
        updateViews(d->tempImage->rect());
    }
    restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public
void kpViewManager::invalidateTempImage()
{
    if (!d->tempImage) {
        return;
    }

    QRect oldRect = d->tempImage->rect();

    delete d->tempImage;
    d->tempImage = nullptr;

    updateViews(oldRect);
}

//---------------------------------------------------------------------

// public
bool kpViewManager::selectionBorderVisible() const
{
    return d->selectionBorderVisible;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setSelectionBorderVisible(bool yes)
{
    if (d->selectionBorderVisible == yes) {
        return;
    }

    d->selectionBorderVisible = yes;

    if (document()->selection()) {
        updateViews(document()->selection()->boundingRect());
    }
}

//---------------------------------------------------------------------

// public
bool kpViewManager::selectionBorderFinished() const
{
    return d->selectionBorderFinished;
}

//---------------------------------------------------------------------

// public
void kpViewManager::setSelectionBorderFinished(bool yes)
{
    if (d->selectionBorderFinished == yes) {
        return;
    }

    d->selectionBorderFinished = yes;

    if (document()->selection()) {
        updateViews(document()->selection()->boundingRect());
    }
}

//---------------------------------------------------------------------

void kpViewManager::setInputMethodEnabled(bool inputMethodEnabled)
{
    d->inputMethodEnabled = inputMethodEnabled;
    if (d->viewUnderCursor) {
        d->viewUnderCursor->setAttribute(Qt::WA_InputMethodEnabled, inputMethodEnabled);
    }
}

//---------------------------------------------------------------------

#include "moc_kpViewManager.cpp"
