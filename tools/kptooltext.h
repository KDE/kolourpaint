
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#ifndef __kptooltext_h__
#define __kptooltext_h__

#include <qpoint.h>
#include <qstring.h>
#include <qvaluevector.h>

#include <kptool.h>


class kpMainWindow;
class kpToolWidgetOpaqueOrTransparent;


class kpTextStyle
{
public:
    kpTextStyle ()
        : m_fontSize (0),
          m_isBold (false), m_isItalic (false),
          m_isUnderline (false), m_isStrikeThru (false)
    {
    }

    kpTextStyle (const QString &fontFamily,
                 int fontSize,
                 bool isBold, bool isItalic,
                 bool isUnderline, bool isStrikeThru)
        : m_fontFamily (fontFamily),
          m_fontSize (fontSize),
          m_isBold (isBold), m_isItalic (isItalic),
          m_isUnderline (isUnderline), m_isStrikeThru (isStrikeThru)
    {
    }

    ~kpTextStyle ()
    {
    }


    QString fontFamily () const { return m_fontFamily; }
    void setFontFamily (const QString &f) { m_fontFamily = f; }

    int fontSize () const { return m_fontSize; }
    void setFontSize (int s) { m_fontSize = s; }

    bool isBold () const { return m_isBold; }
    void setBold (bool yes = true) { m_isBold = yes; }

    bool isItalic () const { return m_isItalic; }
    void setItalic (bool yes = true) { m_isItalic = yes; }

    bool isUnderline () const { return m_isUnderline; }
    void setUnderline (bool yes = true) { m_isUnderline = yes; }

    bool isStrikeThru () const { return m_isStrikeThru; }
    void setStrikeThru (bool yes = true) { m_isStrikeThru = yes; }

private:
    QString m_fontFamily;
    int m_fontSize;
    bool m_isBold, m_isItalic, m_isUnderline, m_isStrikeThru;
};


class kpToolText : public kpTool
{
Q_OBJECT

public:
    kpToolText (kpMainWindow *);
    virtual ~kpToolText ();

    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void cancelShape ();
    virtual void endDraw (const QPoint &thisPoint, const QRect &);

    bool hasDecidedTextTopLeft () const;
    QPoint textTopLeft () const;

protected:
    virtual void keyPressEvent (QKeyEvent *e);

public slots:
    void slotFontFamilyChanged (const QString &fontFamily);
    void slotFontSizeChanged (int fontSize);
    void slotBoldChanged (bool isBold);
    void slotItalicChanged (bool isItalic);
    void slotUnderlineChanged (bool isUnderline);
    void slotStrikeThruChanged (bool isStrikeThru);

private slots:
    void slotIsOpaqueChanged ();

private:
    bool m_hasDecidedTextTopLeft;
    QPoint m_textTopLeft;
    QValueVector <QString> m_textLines;
    kpTextStyle m_textStyle;
    int m_cursorRow, m_cursorCol;
    bool m_cursorOn;
    QPixmap m_textPixmap;
    kpToolWidgetOpaqueOrTransparent *m_toolWidgetOpaqueOrTransparent;
};

#endif  // __kptooltext_h__

