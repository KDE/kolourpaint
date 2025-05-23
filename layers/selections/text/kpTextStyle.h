
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TEXT_STYLE_H
#define KP_TEXT_STYLE_H

#include <QString>

#include "imagelib/kpColor.h"

class QDataStream;
class QFont;
class QFontMetrics;

class kpTextStyle
{
public:
    kpTextStyle();
    kpTextStyle(const QString &fontFamily,
                int fontSize,
                bool isBold,
                bool isItalic,
                bool isUnderline,
                bool isStrikeThru,
                const kpColor &fcolor,
                const kpColor &bcolor,
                bool isBackgroundOpaque);
    ~kpTextStyle();

    friend QDataStream &operator<<(QDataStream &stream, const kpTextStyle &textStyle);
    friend QDataStream &operator>>(QDataStream &stream, kpTextStyle &textStyle);
    bool operator==(const kpTextStyle &rhs) const;
    bool operator!=(const kpTextStyle &rhs) const;

    QString fontFamily() const;
    void setFontFamily(const QString &f);

    int fontSize() const;
    void setFontSize(int s);

    bool isBold() const;
    void setBold(bool yes = true);

    bool isItalic() const;
    void setItalic(bool yes = true);

    bool isUnderline() const;
    void setUnderline(bool yes = true);

    bool isStrikeThru() const;
    void setStrikeThru(bool yes = true);

    kpColor foregroundColor() const;
    void setForegroundColor(const kpColor &fcolor);

    // Note: This is the _input_ backgroundColor
    kpColor backgroundColor() const;
    void setBackgroundColor(const kpColor &bcolor);

    bool isBackgroundOpaque() const;
    void setBackgroundOpaque(bool yes = true);

    bool isBackgroundTransparent() const;
    void setBackgroundTransparent(bool yes = true);

    QFont font() const;
    QFontMetrics fontMetrics() const;

private:
    QString m_fontFamily;
    int m_fontSize;
    bool m_isBold, m_isItalic, m_isUnderline, m_isStrikeThru;
    kpColor m_foregroundColor, m_backgroundColor;
    bool m_isBackgroundOpaque;
};

#endif // KP_TEXT_STYLE_H
