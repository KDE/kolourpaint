
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

#ifndef __kp_squeezed_text_label_h__
#define __kp_squeezed_text_label_h__

#include <qlabel.h>
#include <qstring.h>


// KSqueezedTextLabel done properly - squeeze at the end of the string,
// not the middle.
class kpSqueezedTextLabel : public QLabel
{
Q_OBJECT

public:
    kpSqueezedTextLabel (QWidget *parent, const char *name = 0);
    kpSqueezedTextLabel (const QString &text, QWidget *parent, const char *name = 0);

    virtual QSize minimumSizeHint () const;

    // TODO: maybe text() should return the full text?
    QString fullText () const;

    bool showEllipsis () const;
    void setShowEllipsis (bool yes = true);
    
public slots:
    virtual void setText (const QString &text);

protected:
    virtual void resizeEvent (QResizeEvent *);
    QString ellipsisText () const;
    void squeezeText ();

    QString m_fullText;
    bool m_showEllipsis;
};

#endif  // __kp_squeezed_text_label_h__
