
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


#include "kpToolTextEnterCommand.h"

#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

#include <QList>


kpToolTextEnterCommand::kpToolTextEnterCommand (const QString &name,
    int row, int col, Action action,
    kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      m_row (row), m_col (col),
      m_numEnters (0)
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    if (action == AddEnterNow) {
        addEnter ();
    }
}

kpToolTextEnterCommand::~kpToolTextEnterCommand () = default;


// public
void kpToolTextEnterCommand::addEnter ()
{
    QList <QString> textLines = textSelection ()->textLines ();

    const QString rightHalf = textLines [m_row].mid (m_col);

    textLines [m_row].truncate (m_col);
    textLines.insert (textLines.begin () + m_row + 1, rightHalf);

    textSelection ()->setTextLines (textLines);

    m_row++;
    m_col = 0;

    viewManager ()->setTextCursorPosition (m_row, m_col);

    m_numEnters++;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextEnterCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolTextEnterCommand::execute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    int oldNumEnters = m_numEnters;
    m_numEnters = 0;

    for (int i = 0; i < oldNumEnters; i++) {
        addEnter ();
    }
}

// public virtual [base kpCommand]
void kpToolTextEnterCommand::unexecute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QList <QString> textLines = textSelection ()->textLines ();

    for (int i = 0; i < m_numEnters; i++)
    {
        Q_ASSERT (m_col == 0);

        if (m_row <= 0) {
            break;
        }

        int newRow = m_row - 1;
        int newCol = textLines [newRow].length ();

        textLines [newRow] += textLines [m_row];

        textLines.erase (textLines.begin () + m_row);

        m_row = newRow;
        m_col = newCol;
    }

    textSelection ()->setTextLines (textLines);

    viewManager ()->setTextCursorPosition (m_row, m_col);
}

