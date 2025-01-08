
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpViewManagerPrivate_H
#define kpViewManagerPrivate_H

#include <QCursor>
#include <QList>

class kpMainWindow;
class kpTempImage;
class QTimer;
class kpView;

struct kpViewManagerPrivate {
    kpMainWindow *mainWindow;

    QList<kpView *> views;
    kpView *viewUnderCursor;

    QCursor cursor;

    kpTempImage *tempImage;

    bool selectionBorderVisible;
    bool selectionBorderFinished;

    //
    // Text Cursor
    //

    // Exists only if the text cursor is enabled.  0 otherwise.
    QTimer *textCursorBlinkTimer;

    // (undefined if there is not currently a text selection)
    int textCursorRow, textCursorCol;

    // (undefined if there is not currently a text selection)
    bool textCursorBlinkState;

    //
    // View Updates
    //

    int queueUpdatesCounter, fastUpdatesCounter;

    //
    // Input Method
    //

    bool inputMethodEnabled;
};

#endif // kpViewManagerPrivate_H
