
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


#include <kpToolText.h>

#include <qevent.h>
#include <qlist.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpDocument.h>
#include <kpTextSelection.h>
#include <kpToolTextBackspaceCommand.h>
#include <kpToolTextChangeStyleCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionEnvironment.h>
#include <kpToolTextDeleteCommand.h>
#include <kpToolTextEnterCommand.h>
#include <kpToolTextInsertCommand.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


// protected static
bool kpToolText::CursorIsOnWordChar (const QList <QString> &textLines,
    int cursorRow, int cursorCol)
{
    return (cursorRow >= 0 && cursorRow < (int) textLines.size () &&
            cursorCol >= 0 && cursorCol < (int) textLines [cursorRow].length () &&
            !textLines [cursorRow][cursorCol].isSpace ());
}


// protected static
bool kpToolText::CursorIsAtStart (const QList <QString> &,
    int cursorRow, int cursorCol)
{
    return (cursorRow == 0 && cursorCol == 0);
}

// protected static
bool kpToolText::CursorIsAtEnd (const QList <QString> &textLines,
    int cursorRow, int cursorCol)
{
    if (textLines.isEmpty ())
        return (cursorRow == 0 && cursorCol == 0);
        
    return (cursorRow == (int) textLines.size () - 1 &&
            cursorCol == (int) textLines [cursorRow].length ());
}


// protected static
void kpToolText::MoveCursorLeft (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty ())
        return;
        
    (*cursorCol)--;

    if (*cursorCol < 0)
    {
        (*cursorRow)--;
        if (*cursorRow < 0)
        {
            *cursorRow = 0;
            *cursorCol = 0;
        }
        else
            *cursorCol = textLines [*cursorRow].length ();
    }
}

// protected static
void kpToolText::MoveCursorRight (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty ())
        return;
        
    (*cursorCol)++;

    if (*cursorCol > (int) textLines [*cursorRow].length ())
    {
        (*cursorRow)++;
        if (*cursorRow > (int) textLines.size () - 1)
        {
            *cursorRow = textLines.size () - 1;
            *cursorCol = textLines [*cursorRow].length ();
        }
        else
            *cursorCol = 0;
    }
}


#define IS_ON_SPACE_OR_EOL() !CursorIsOnWordChar (textLines, *cursorRow, *cursorCol)

// protected static
int kpToolText::MoveCursorToWordStart (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty ())
        return 0;

    int numMoves = 0;

#define IS_ON_ANCHOR()                                            \
    (CursorIsOnWordChar (textLines, *cursorRow, *cursorCol) &&    \
         (cursorCol == 0 ||                                       \
             !CursorIsOnWordChar (textLines, *cursorRow, *cursorCol - 1)))
#define MOVE_CURSOR_LEFT()    \
    (MoveCursorLeft (textLines, cursorRow, cursorCol), ++numMoves)


    // (these comments will exclude the row=0,col=0 boundary case)

    if (IS_ON_ANCHOR ())
        MOVE_CURSOR_LEFT ();

    // --- now we're not on an anchor point (start of word) ---

    // End up on a letter...
    while (!(*cursorRow == 0 && *cursorCol == 0) &&
            (IS_ON_SPACE_OR_EOL ()))
    {
        MOVE_CURSOR_LEFT ();
    }

    // --- now we're on a letter ---

    // Find anchor point
    while (!(*cursorRow == 0 && *cursorCol == 0) && !IS_ON_ANCHOR ())
    {
        MOVE_CURSOR_LEFT ();
    }


#undef IS_ON_ANCHOR
#undef MOVE_CURSOR_LEFT

    return numMoves;
}

// protected static
int kpToolText::MoveCursorToNextWordStart (const QList <QString> &textLines,
    int *cursorRow, int *cursorCol)
{
    if (textLines.isEmpty ())
        return 0;
        
    int numMoves = 0;

#define IS_AT_END() CursorIsAtEnd (textLines, *cursorRow, *cursorCol)
#define MOVE_CURSOR_RIGHT()    \
    (MoveCursorRight (textLines, cursorRow, cursorCol), ++numMoves)


    // (these comments will exclude the last row,end col boundary case)

    // Find space
    while (!IS_AT_END () && !IS_ON_SPACE_OR_EOL ())
    {
        MOVE_CURSOR_RIGHT ();
    }

    // --- now we're on a space ---

    // Find letter
    while (!IS_AT_END () && IS_ON_SPACE_OR_EOL ())
    {
        MOVE_CURSOR_RIGHT ();
    }

    // --- now we're on a letter ---


#undef IS_AT_END
#undef MOVE_CURSOR_RIGHT

    return numMoves;
}

#undef IS_ON_SPACE_OR_EOL

