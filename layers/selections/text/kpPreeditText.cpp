
/*
   SPDX-FileCopyrightText: 2010 Tasuku Suzuki <stasuku@gmail.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "layers/selections/text/kpPreeditText.h"

//---------------------------------------------------------------------

bool attributeLessThan(const QInputMethodEvent::Attribute &a1, const QInputMethodEvent::Attribute &a2)
{
    return a1.start < a2.start;
}

//---------------------------------------------------------------------

kpPreeditText::kpPreeditText()
    : m_cursorPosition(0)
    , m_cursorColor(Qt::transparent)
    , m_selectionStart(0)
    , m_selectionLength(0)
    , m_position(-1, -1)
{
}

//---------------------------------------------------------------------

kpPreeditText::kpPreeditText(const QInputMethodEvent *event)
    : m_cursorPosition(0)
    , m_cursorColor(Qt::transparent)
    , m_selectionStart(0)
    , m_selectionLength(0)
    , m_position(-1, -1)
{
    m_preeditString = event->preeditString();
    for (const auto &attr : event->attributes()) {
        switch (attr.type) {
        case QInputMethodEvent::TextFormat:
            m_textFormatList.append(attr);
            break;
        case QInputMethodEvent::Cursor:
            m_cursorPosition = attr.start;
            if (attr.length > 0) {
                m_cursorColor = attr.value.value<QColor>();
            }
            break;
        case QInputMethodEvent::Selection:
            m_selectionStart = attr.start;
            m_selectionLength = attr.length;
            break;
        default:
            break;
        }
    }
    std::sort(m_textFormatList.begin(), m_textFormatList.end(), attributeLessThan);
}

//---------------------------------------------------------------------

bool kpPreeditText::isEmpty() const
{
    return m_preeditString.isEmpty();
}

//---------------------------------------------------------------------

const QString &kpPreeditText::preeditString() const
{
    return m_preeditString;
}

//---------------------------------------------------------------------

int kpPreeditText::cursorPosition() const
{
    return m_cursorPosition;
}

//---------------------------------------------------------------------

const QColor &kpPreeditText::cursorColor() const
{
    return m_cursorColor;
}

//---------------------------------------------------------------------

int kpPreeditText::selectionStart() const
{
    return m_selectionStart;
}

//---------------------------------------------------------------------

int kpPreeditText::selectionLength() const
{
    return m_selectionLength;
}

//---------------------------------------------------------------------

const QList<QInputMethodEvent::Attribute> &kpPreeditText::textFormatList() const
{
    return m_textFormatList;
}

//---------------------------------------------------------------------

const QPoint &kpPreeditText::position() const
{
    return m_position;
}

//---------------------------------------------------------------------

void kpPreeditText::setPosition(const QPoint &position)
{
    m_position = position;
}

//---------------------------------------------------------------------
