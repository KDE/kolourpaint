
/*
   SPDX-FileCopyrightText: 2010 Tasuku Suzuki <stasuku@gmail.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpPreeditText_H
#define kpPreeditText_H

#include <QColor>
#include <QInputMethodEvent>
#include <QPoint>

class kpPreeditText
{
public:
    kpPreeditText();
    explicit kpPreeditText(const QInputMethodEvent *event);

    bool isEmpty() const;

    const QString &preeditString() const;
    int cursorPosition() const;
    bool cursorVisible() const;
    const QColor &cursorColor() const;
    int selectionStart() const;
    int selectionLength() const;
    const QList<QInputMethodEvent::Attribute> &textFormatList() const;

    const QPoint &position() const;
    void setPosition(const QPoint &position);

private:
    QString m_preeditString;
    int m_cursorPosition;
    QColor m_cursorColor;
    int m_selectionStart;
    int m_selectionLength;
    QList<QInputMethodEvent::Attribute> m_textFormatList;
    QPoint m_position;
};

#endif // kpPreeditText_H
