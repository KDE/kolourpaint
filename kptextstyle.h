
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

#ifndef __kp_text_style_h__
#define __kp_text_style_h__

#include <qstring.h>

#include <kpcolor.h>

class QDataStream;
class QFont;
class QFontMetrics;

class kpTextStyle
{
public:
    kpTextStyle ();
    kpTextStyle (const QString &fontFamily,
                 int fontSize,
                 bool isBold, bool isItalic,
                 bool isUnderline, bool isStrikeThru,
                 const kpColor &fcolor,
                 const kpColor &bcolor,
                 bool isBackgroundOpaque);
    ~kpTextStyle ();


    friend QDataStream &operator<< (QDataStream &stream, const kpTextStyle &textStyle);
    friend QDataStream &operator>> (QDataStream &stream, kpTextStyle &textStyle);
    bool operator== (const kpTextStyle &rhs) const;
    bool operator!= (const kpTextStyle &rhs) const;


    QString fontFamily () const;
    void setFontFamily (const QString &f);

    int fontSize () const;
    void setFontSize (int s);

    bool isBold () const;
    void setBold (bool yes = true);

    bool isItalic () const;
    void setItalic (bool yes = true);

    bool isUnderline () const;
    void setUnderline (bool yes = true);

    bool isStrikeThru () const;
    void setStrikeThru (bool yes = true);

    kpColor foregroundColor () const;
    void setForegroundColor (const kpColor &fcolor);

    // Note: This is the _input_ backgroundColor without applying any
    //       isBackground(Opaque|Transparent)() transformation.
    //       See effectiveBackgroundColor().
    kpColor backgroundColor () const;
    void setBackgroundColor (const kpColor &bcolor);

    bool isBackgroundOpaque () const;
    void setBackgroundOpaque (bool yes = true);

    bool isBackgroundTransparent () const;
    void setBackgroundTransparent (bool yes = true);


    // If isBackgroundOpaque(), returns backgroundColor().
    // Else, returns kpColor::transparent.
    kpColor effectiveBackgroundColor () const;

    QFont font () const;
    QFontMetrics fontMetrics () const;

private:
    QString m_fontFamily;
    int m_fontSize;
    bool m_isBold, m_isItalic, m_isUnderline, m_isStrikeThru;
    kpColor m_foregroundColor, m_backgroundColor;
    bool m_isBackgroundOpaque;
};

#endif  // __kp_text_style_h__
