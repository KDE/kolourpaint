
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextBackspaceCommand.h"

#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

#include <QList>

kpToolTextBackspaceCommand::kpToolTextBackspaceCommand(const QString &name, int row, int col, Action action, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_row(row)
    , m_col(col)
    , m_numBackspaces(0)
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    if (action == AddBackspaceNow) {
        addBackspace();
    }
}

kpToolTextBackspaceCommand::~kpToolTextBackspaceCommand() = default;

// public
void kpToolTextBackspaceCommand::addBackspace()
{
    QList<QString> textLines = textSelection()->textLines();

    if (m_col > 0) {
        m_deletedText.prepend(textLines[m_row][m_col - 1]);

        textLines[m_row] = textLines[m_row].left(m_col - 1) + textLines[m_row].mid(m_col);
        m_col--;
    } else {
        if (m_row > 0) {
            int newCursorRow = m_row - 1;
            int newCursorCol = textLines[newCursorRow].length();

            m_deletedText.prepend(QLatin1Char('\n'));

            textLines[newCursorRow] += textLines[m_row];

            textLines.erase(textLines.begin() + m_row);

            m_row = newCursorRow;
            m_col = newCursorCol;
        }
    }

    textSelection()->setTextLines(textLines);

    viewManager()->setTextCursorPosition(m_row, m_col);

    m_numBackspaces++;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextBackspaceCommand::size() const
{
    return static_cast<kpCommandSize::SizeType>(static_cast<unsigned int>(m_deletedText.length()) * sizeof(QChar));
}

// public virtual [base kpCommand]
void kpToolTextBackspaceCommand::execute()
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    m_deletedText.clear();
    int oldNumBackspaces = m_numBackspaces;
    m_numBackspaces = 0;

    for (int i = 0; i < oldNumBackspaces; i++) {
        addBackspace();
    }
}

// public virtual [base kpCommand]
void kpToolTextBackspaceCommand::unexecute()
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    QList<QString> textLines = textSelection()->textLines();

    for (auto &&i : m_deletedText) {
        if (i == QLatin1Char('\n')) {
            const QString rightHalf = textLines[m_row].mid(m_col);

            textLines[m_row].truncate(m_col);
            textLines.insert(textLines.begin() + m_row + 1, rightHalf);

            m_row++;
            m_col = 0;
        } else {
            const QString leftHalf = textLines[m_row].left(m_col);
            const QString rightHalf = textLines[m_row].mid(m_col);

            textLines[m_row] = leftHalf + i + rightHalf;
            m_col++;
        }
    }

    m_deletedText.clear();

    textSelection()->setTextLines(textLines);

    viewManager()->setTextCursorPosition(m_row, m_col);
}
