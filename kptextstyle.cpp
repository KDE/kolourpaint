
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kptextstyle.h>

#include <qdatastream.h>
#include <qfont.h>
#include <qfontmetrics.h>


kpTextStyle::kpTextStyle ()
    : m_fontSize (0),
      m_isBold (false), m_isItalic (false),
      m_isUnderline (false), m_isStrikeThru (false),
      m_isBackgroundOpaque (true)
{
}

kpTextStyle::kpTextStyle (const QString &fontFamily,
                          int fontSize,
                          bool isBold, bool isItalic,
                          bool isUnderline, bool isStrikeThru,
                          const kpColor &fcolor, const kpColor &bcolor,
                          bool isBackgroundOpaque)
    : m_fontFamily (fontFamily),
      m_fontSize (fontSize),
      m_isBold (isBold), m_isItalic (isItalic),
      m_isUnderline (isUnderline), m_isStrikeThru (isStrikeThru),
      m_foregroundColor (fcolor), m_backgroundColor (bcolor),
      m_isBackgroundOpaque (isBackgroundOpaque)
{
}

kpTextStyle::~kpTextStyle ()
{
}


// friend
QDataStream &operator<< (QDataStream &stream, const kpTextStyle &textStyle)
{
    stream << textStyle.m_fontFamily;
    stream << textStyle.m_fontSize;

    stream << int (textStyle.m_isBold) << int (textStyle.m_isItalic)
           << int (textStyle.m_isUnderline) << int (textStyle.m_isStrikeThru);

    stream << textStyle.m_foregroundColor << textStyle.m_backgroundColor;

    stream << int (textStyle.m_isBackgroundOpaque);

    return stream;
}

// friend
QDataStream &operator>> (QDataStream &stream, kpTextStyle &textStyle)
{
    stream >> textStyle.m_fontFamily;
    stream >> textStyle.m_fontSize;

    int a, b, c, d;
    stream >> a >> b >> c >> d;
    textStyle.m_isBold = a;
    textStyle.m_isItalic = b;
    textStyle.m_isUnderline = c;
    textStyle.m_isStrikeThru = d;

    stream >> textStyle.m_foregroundColor >> textStyle.m_backgroundColor;

    int e;
    stream >> e;
    textStyle.m_isBackgroundOpaque = e;

    return stream;
}

// public
bool kpTextStyle::operator== (const kpTextStyle &rhs) const
{
    return (m_fontFamily == rhs.m_fontFamily &&
            m_fontSize == rhs.m_fontSize &&
            m_isBold == rhs.m_isBold &&
            m_isItalic == rhs.m_isItalic &&
            m_isUnderline == rhs.m_isUnderline &&
            m_isStrikeThru == rhs.m_isStrikeThru &&
            m_foregroundColor == rhs.m_foregroundColor &&
            m_backgroundColor == rhs.m_backgroundColor &&
            m_isBackgroundOpaque == rhs.m_isBackgroundOpaque);
}

// public
bool kpTextStyle::operator!= (const kpTextStyle &rhs) const
{
    return !(*this == rhs);
}


// public
QString kpTextStyle::fontFamily () const
{
    return m_fontFamily;
}

// public
void kpTextStyle::setFontFamily (const QString &f)
{
    m_fontFamily = f;
}


// public
int kpTextStyle::fontSize () const
{
    return m_fontSize;
}

// public
void kpTextStyle::setFontSize (int s)
{
    m_fontSize = s;
}


// public
bool kpTextStyle::isBold () const
{
    return m_isBold;
}

// public
void kpTextStyle::setBold (bool yes)
{
    m_isBold = yes;
}


// public
bool kpTextStyle::isItalic () const
{
    return m_isItalic;
}

// public
void kpTextStyle::setItalic (bool yes)
{
    m_isItalic = yes;
}


// public
bool kpTextStyle::isUnderline () const
{
    return m_isUnderline;
}

// public
void kpTextStyle::setUnderline (bool yes)
{
    m_isUnderline = yes;
}


// public
bool kpTextStyle::isStrikeThru () const
{
    return m_isStrikeThru;
}

// public
void kpTextStyle::setStrikeThru (bool yes)
{
    m_isStrikeThru = yes;
}


// public
kpColor kpTextStyle::foregroundColor () const
{
    return m_foregroundColor;
}

// public
void kpTextStyle::setForegroundColor (const kpColor &fcolor)
{
    m_foregroundColor = fcolor;
}


// public
kpColor kpTextStyle::backgroundColor () const
{
    return m_backgroundColor;
}

// public
void kpTextStyle::setBackgroundColor (const kpColor &bcolor)
{
    m_backgroundColor = bcolor;
}


// public
bool kpTextStyle::isBackgroundOpaque () const
{
    return m_isBackgroundOpaque;
}

// public
void kpTextStyle::setBackgroundOpaque (bool yes)
{
    m_isBackgroundOpaque = yes;
}


// public
bool kpTextStyle::isBackgroundTransparent () const
{
    return !m_isBackgroundOpaque;
}

// public
void kpTextStyle::setBackgroundTransparent (bool yes)
{
    m_isBackgroundOpaque = !yes;
}


// public
kpColor kpTextStyle::effectiveBackgroundColor () const
{
    if (isBackgroundOpaque ())
        return backgroundColor ();
    else
        return kpColor::transparent;
}


// public
QFont kpTextStyle::font () const
{
    QFont fnt (m_fontFamily, m_fontSize);
    fnt.setBold (m_isBold);
    fnt.setItalic (m_isItalic);
    fnt.setUnderline (m_isUnderline);
    fnt.setStrikeOut (m_isStrikeThru);

    return fnt;
}

// public
QFontMetrics kpTextStyle::fontMetrics () const
{
    return QFontMetrics (font ());
}
