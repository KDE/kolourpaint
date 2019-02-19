
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

#include <QEvent>
#include <QList>

#include "kpLogCategories.h"
#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "commands/tools/selection/text/kpToolTextBackspaceCommand.h"
#include "commands/tools/selection/text/kpToolTextChangeStyleCommand.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/text/kpToolTextDeleteCommand.h"
#include "commands/tools/selection/text/kpToolTextEnterCommand.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

// protected
void kpToolText::handleUpKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tup pressed";
#endif

    if (hasBegunShape ()) {
        endShape (currentPoint (), normalizedRect ());
    }

    if (!textLines.isEmpty () && cursorRow > 0)
    {
        cursorRow--;
        cursorCol = qMin (cursorCol, textLines [cursorRow].length ());
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }

    e->accept ();
}

// protected
void kpToolText::handleDownKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tdown pressed";
#endif

    if (hasBegunShape ()) {
        endShape (currentPoint (), normalizedRect ());
    }

    if (!textLines.isEmpty () && cursorRow < textLines.size () - 1)
    {
        cursorRow++;
        cursorCol = qMin (cursorCol, textLines [cursorRow].length ());
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }

    e->accept ();
}

// protected
void kpToolText::handleLeftKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tleft pressed";
#endif

    if (hasBegunShape ()) {
        endShape (currentPoint (), normalizedRect ());
    }

    if (!textLines.isEmpty ())
    {
        if ((e->modifiers () & Qt::ControlModifier) == 0)
        {
        #if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove single char";
        #endif
    
            MoveCursorLeft (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }
        else
        {
        #if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove to start of word";
        #endif
    
            MoveCursorToWordStart (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }
    }

    e->accept ();
}

// protected
void kpToolText::handleRightKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tright pressed";
#endif

    if (hasBegunShape ()) {
        endShape (currentPoint (), normalizedRect ());
    }

    if (!textLines.isEmpty ())
    {
        if ((e->modifiers () & Qt::ControlModifier) == 0)
        {
        #if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove single char";
        #endif
    
            MoveCursorRight (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }
        else
        {
        #if DEBUG_KP_TOOL_TEXT
            qCDebug(kpLogTools) << "\tmove to start of next word";
        #endif
    
            MoveCursorToNextWordStart (textLines, &cursorRow, &cursorCol);
            viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
        }
    }

    e->accept ();
}


// protected
void kpToolText::handleHomeKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\thome pressed";
#endif

    if (hasBegunShape ()) {
        endShape (currentPoint (), normalizedRect ());
    }

    if (!textLines.isEmpty ())
    {
        if (e->modifiers () & Qt::ControlModifier) {
            cursorRow = 0;
        }
    
        cursorCol = 0;
    
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }

    e->accept ();
}

// protected
void kpToolText::handleEndKeyPress (QKeyEvent *e,
    const QList <QString> &textLines, int cursorRow, int cursorCol)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "\tend pressed";
#endif

    if (hasBegunShape ()) {
        endShape (currentPoint (), normalizedRect ());
    }

    if (!textLines.isEmpty ())
    {
        if (e->modifiers () & Qt::ControlModifier) {
            cursorRow = textLines.size () - 1;
        }
    
        cursorCol = textLines [cursorRow].length ();
    
        viewManager ()->setTextCursorPosition (cursorRow, cursorCol);
    }

    e->accept ();
}
