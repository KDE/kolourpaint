
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


#include "kpToolTextInsertCommand.h"

#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

#include <QList>

//---------------------------------------------------------------------

kpToolTextInsertCommand::kpToolTextInsertCommand (const QString &name,
        int row, int col, const QString& newText,
        kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      m_row (row), m_col (col)
{
    viewManager ()->setTextCursorPosition (m_row, m_col);
    addText (newText);
}

//---------------------------------------------------------------------

// public
void kpToolTextInsertCommand::addText (const QString &moreText)
{
    if (moreText.isEmpty ()) {
        return;
    }

    QList <QString> textLines = textSelection ()->textLines ();
    const QString leftHalf = textLines [m_row].left (m_col);
    const QString rightHalf = textLines [m_row].mid (m_col);
    textLines [m_row] = leftHalf + moreText + rightHalf;
    textSelection ()->setTextLines (textLines);

    m_newText += moreText;
    m_col += moreText.length ();

    viewManager ()->setTextCursorPosition (m_row, m_col);
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextInsertCommand::size () const
{
    return static_cast<kpCommandSize::SizeType>
            (static_cast<unsigned int> (m_newText.length ()) * sizeof (QChar));
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpToolTextInsertCommand::execute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QString text = m_newText;
    m_newText.clear ();
    addText (text);
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpToolTextInsertCommand::unexecute ()
{
    viewManager ()->setTextCursorPosition (m_row, m_col);

    QList <QString> textLines = textSelection ()->textLines ();
    const QString leftHalf = textLines [m_row].left (m_col - m_newText.length ());
    const QString rightHalf = textLines [m_row].mid (m_col);
    textLines [m_row] = leftHalf + rightHalf;
    textSelection ()->setTextLines (textLines);

    m_col -= m_newText.length ();

    viewManager ()->setTextCursorPosition (m_row, m_col);
}

//---------------------------------------------------------------------
