
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "layers/selections/text/kpTextStyle.h"

#include <QDataStream>
#include <QFont>
#include <QFontMetrics>

kpTextStyle::kpTextStyle()
    : m_fontSize(0)
    , m_isBold(false)
    , m_isItalic(false)
    , m_isUnderline(false)
    , m_isStrikeThru(false)
    , m_isBackgroundOpaque(true)
{
}

kpTextStyle::kpTextStyle(const QString &fontFamily,
                         int fontSize,
                         bool isBold,
                         bool isItalic,
                         bool isUnderline,
                         bool isStrikeThru,
                         const kpColor &fcolor,
                         const kpColor &bcolor,
                         bool isBackgroundOpaque)
    : m_fontFamily(fontFamily)
    , m_fontSize(fontSize)
    , m_isBold(isBold)
    , m_isItalic(isItalic)
    , m_isUnderline(isUnderline)
    , m_isStrikeThru(isStrikeThru)
    , m_foregroundColor(fcolor)
    , m_backgroundColor(bcolor)
    , m_isBackgroundOpaque(isBackgroundOpaque)
{
}

kpTextStyle::~kpTextStyle() = default;

// friend
QDataStream &operator<<(QDataStream &stream, const kpTextStyle &textStyle)
{
    stream << textStyle.m_fontFamily;
    stream << textStyle.m_fontSize;

    stream << int(textStyle.m_isBold) << int(textStyle.m_isItalic) << int(textStyle.m_isUnderline) << int(textStyle.m_isStrikeThru);

    stream << textStyle.m_foregroundColor << textStyle.m_backgroundColor;

    stream << int(textStyle.m_isBackgroundOpaque);

    return stream;
}

// friend
QDataStream &operator>>(QDataStream &stream, kpTextStyle &textStyle)
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
bool kpTextStyle::operator==(const kpTextStyle &rhs) const
{
    return (m_fontFamily == rhs.m_fontFamily && m_fontSize == rhs.m_fontSize && m_isBold == rhs.m_isBold && m_isItalic == rhs.m_isItalic
            && m_isUnderline == rhs.m_isUnderline && m_isStrikeThru == rhs.m_isStrikeThru && m_foregroundColor == rhs.m_foregroundColor
            && m_backgroundColor == rhs.m_backgroundColor && m_isBackgroundOpaque == rhs.m_isBackgroundOpaque);
}

// public
bool kpTextStyle::operator!=(const kpTextStyle &rhs) const
{
    return !(*this == rhs);
}

// public
QString kpTextStyle::fontFamily() const
{
    return m_fontFamily;
}

// public
void kpTextStyle::setFontFamily(const QString &f)
{
    m_fontFamily = f;
}

// public
int kpTextStyle::fontSize() const
{
    return m_fontSize;
}

// public
void kpTextStyle::setFontSize(int s)
{
    m_fontSize = s;
}

// public
bool kpTextStyle::isBold() const
{
    return m_isBold;
}

// public
void kpTextStyle::setBold(bool yes)
{
    m_isBold = yes;
}

// public
bool kpTextStyle::isItalic() const
{
    return m_isItalic;
}

// public
void kpTextStyle::setItalic(bool yes)
{
    m_isItalic = yes;
}

// public
bool kpTextStyle::isUnderline() const
{
    return m_isUnderline;
}

// public
void kpTextStyle::setUnderline(bool yes)
{
    m_isUnderline = yes;
}

// public
bool kpTextStyle::isStrikeThru() const
{
    return m_isStrikeThru;
}

// public
void kpTextStyle::setStrikeThru(bool yes)
{
    m_isStrikeThru = yes;
}

// public
kpColor kpTextStyle::foregroundColor() const
{
    return m_foregroundColor;
}

// public
void kpTextStyle::setForegroundColor(const kpColor &fcolor)
{
    m_foregroundColor = fcolor;
}

// public
kpColor kpTextStyle::backgroundColor() const
{
    return m_backgroundColor;
}

// public
void kpTextStyle::setBackgroundColor(const kpColor &bcolor)
{
    m_backgroundColor = bcolor;
}

// public
bool kpTextStyle::isBackgroundOpaque() const
{
    return m_isBackgroundOpaque;
}

// public
void kpTextStyle::setBackgroundOpaque(bool yes)
{
    m_isBackgroundOpaque = yes;
}

// public
bool kpTextStyle::isBackgroundTransparent() const
{
    return !m_isBackgroundOpaque;
}

// public
void kpTextStyle::setBackgroundTransparent(bool yes)
{
    m_isBackgroundOpaque = !yes;
}

// public
QFont kpTextStyle::font() const
{
    QFont fnt(m_fontFamily, m_fontSize);
    fnt.setBold(m_isBold);
    fnt.setItalic(m_isItalic);
    fnt.setUnderline(m_isUnderline);
    fnt.setStrikeOut(m_isStrikeThru);

    return fnt;
}

// public
QFontMetrics kpTextStyle::fontMetrics() const
{
    return QFontMetrics(font());
}
