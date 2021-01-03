
/*
   Copyright (c) 2010 Tasuku Suzuki <stasuku@gmail.com>
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

#include "layers/selections/text/kpPreeditText.h"

//---------------------------------------------------------------------

bool attributeLessThan (const QInputMethodEvent::Attribute &a1, const QInputMethodEvent::Attribute &a2)
{
    return a1.start < a2.start;
}

//---------------------------------------------------------------------

kpPreeditText::kpPreeditText ()
    : m_cursorPosition (0), m_cursorColor (Qt::transparent),
      m_selectionStart (0), m_selectionLength (0),
      m_position (-1, -1)
{
}

//---------------------------------------------------------------------

kpPreeditText::kpPreeditText (const QInputMethodEvent *event)
    : m_cursorPosition (0), m_cursorColor (Qt::transparent),
      m_selectionStart (0), m_selectionLength (0),
      m_position (-1, -1)
{
    m_preeditString = event->preeditString ();
    for (const auto &attr : event->attributes ())
    {
        switch (attr.type)
        {
        case QInputMethodEvent::TextFormat:
            m_textFormatList.append (attr);
            break;
        case QInputMethodEvent::Cursor:
            m_cursorPosition = attr.start;
            if (attr.length > 0)
            {
                m_cursorColor = attr.value.value<QColor> ();
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

bool kpPreeditText::isEmpty () const
{
    return m_preeditString.isEmpty ();
}

//---------------------------------------------------------------------

const QString &kpPreeditText::preeditString () const
{
    return m_preeditString;
}

//---------------------------------------------------------------------

int kpPreeditText::cursorPosition () const
{
    return m_cursorPosition;
}

//---------------------------------------------------------------------

const QColor &kpPreeditText::cursorColor () const
{
    return m_cursorColor;
}

//---------------------------------------------------------------------

int kpPreeditText::selectionStart () const
{
    return m_selectionStart;
}

//---------------------------------------------------------------------

int kpPreeditText::selectionLength () const
{
    return m_selectionLength;
}

//---------------------------------------------------------------------

const QList<QInputMethodEvent::Attribute> &kpPreeditText::textFormatList () const
{
    return m_textFormatList;
}

//---------------------------------------------------------------------

const QPoint &kpPreeditText::position () const
{
    return m_position;
}

//---------------------------------------------------------------------

void kpPreeditText::setPosition (const QPoint &position)
{
    m_position = position;
}

//---------------------------------------------------------------------
