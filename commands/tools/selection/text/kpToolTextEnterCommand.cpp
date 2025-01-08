
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextEnterCommand.h"

#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

#include <QList>

kpToolTextEnterCommand::kpToolTextEnterCommand(const QString &name, int row, int col, Action action, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_row(row)
    , m_col(col)
    , m_numEnters(0)
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    if (action == AddEnterNow) {
        addEnter();
    }
}

kpToolTextEnterCommand::~kpToolTextEnterCommand() = default;

// public
void kpToolTextEnterCommand::addEnter()
{
    QList<QString> textLines = textSelection()->textLines();

    const QString rightHalf = textLines[m_row].mid(m_col);

    textLines[m_row].truncate(m_col);
    textLines.insert(textLines.begin() + m_row + 1, rightHalf);

    textSelection()->setTextLines(textLines);

    m_row++;
    m_col = 0;

    viewManager()->setTextCursorPosition(m_row, m_col);

    m_numEnters++;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextEnterCommand::size() const
{
    return 0;
}

// public virtual [base kpCommand]
void kpToolTextEnterCommand::execute()
{
    viewManager()->setTextCursorPosition(m_row, m_col);
    int oldNumEnters = m_numEnters;
    m_numEnters = 0;

    for (int i = 0; i < oldNumEnters; i++) {
        addEnter();
    }
}

// public virtual [base kpCommand]
void kpToolTextEnterCommand::unexecute()
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    QList<QString> textLines = textSelection()->textLines();

    for (int i = 0; i < m_numEnters; i++) {
        Q_ASSERT(m_col == 0);

        if (m_row <= 0) {
            break;
        }

        int newRow = m_row - 1;
        int newCol = textLines[newRow].length();

        textLines[newRow] += textLines[m_row];

        textLines.erase(textLines.begin() + m_row);

        m_row = newRow;
        m_col = newCol;
    }

    textSelection()->setTextLines(textLines);

    viewManager()->setTextCursorPosition(m_row, m_col);
}
