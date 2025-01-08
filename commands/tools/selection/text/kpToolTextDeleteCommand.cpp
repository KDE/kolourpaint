
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "kpToolTextDeleteCommand.h"

#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

#include <QList>

kpToolTextDeleteCommand::kpToolTextDeleteCommand(const QString &name, int row, int col, Action action, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_row(row)
    , m_col(col)
    , m_numDeletes(0)
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    if (action == AddDeleteNow) {
        addDelete();
    }
}

kpToolTextDeleteCommand::~kpToolTextDeleteCommand() = default;

// public
void kpToolTextDeleteCommand::addDelete()
{
    QList<QString> textLines = textSelection()->textLines();

    if (m_col < static_cast<int>(textLines[m_row].length())) {
        m_deletedText.prepend(textLines[m_row][m_col]);

        textLines[m_row] = textLines[m_row].left(m_col) + textLines[m_row].mid(m_col + 1);
    } else {
        if (m_row < static_cast<int>(textLines.size() - 1)) {
            m_deletedText.prepend(QLatin1Char('\n'));

            textLines[m_row] += textLines[m_row + 1];
            textLines.erase(textLines.begin() + m_row + 1);
        }
    }

    textSelection()->setTextLines(textLines);

    viewManager()->setTextCursorPosition(m_row, m_col);

    m_numDeletes++;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextDeleteCommand::size() const
{
    return static_cast<kpCommandSize::SizeType>(static_cast<unsigned int>(m_deletedText.length()) * sizeof(QChar));
}

// public virtual [base kpCommand]
void kpToolTextDeleteCommand::execute()
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    m_deletedText.clear();
    int oldNumDeletes = m_numDeletes;
    m_numDeletes = 0;

    for (int i = 0; i < oldNumDeletes; i++) {
        addDelete();
    }
}

// public virtual [base kpCommand]
void kpToolTextDeleteCommand::unexecute()
{
    viewManager()->setTextCursorPosition(m_row, m_col);

    QList<QString> textLines = textSelection()->textLines();

    for (auto &&i : m_deletedText) {
        if (i == QLatin1Char('\n')) {
            const QString rightHalf = textLines[m_row].mid(m_col);

            textLines[m_row].truncate(m_col);
            textLines.insert(textLines.begin() + m_row + 1, rightHalf);
        } else {
            const QString leftHalf = textLines[m_row].left(m_col);
            const QString rightHalf = textLines[m_row].mid(m_col);

            textLines[m_row] = leftHalf + i + rightHalf;
        }
    }

    m_deletedText.clear();

    textSelection()->setTextLines(textLines);

    viewManager()->setTextCursorPosition(m_row, m_col);
}
