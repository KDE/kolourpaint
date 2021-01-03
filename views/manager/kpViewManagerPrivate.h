
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


#ifndef kpViewManagerPrivate_H
#define kpViewManagerPrivate_H


#include <QCursor>
#include <QList>


class kpMainWindow;
class QTimer;
class kpView;


struct kpViewManagerPrivate
{
    kpMainWindow *mainWindow;

    QList <kpView *> views;
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


#endif  // kpViewManagerPrivate_H
