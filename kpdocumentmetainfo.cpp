
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

#include <kpdocumentmetainfo.h>

#include <qpoint.h>

#include <kdebug.h>


struct kpDocumentMetaInfoPrivate
{
    int m_dotsPerMeterX, m_dotsPerMeterY;
    QPoint m_offset;

    QMap <QImageTextKeyLang, QString> m_textMap;
};


// public
kpDocumentMetaInfo::kpDocumentMetaInfo ()
    : d (new kpDocumentMetaInfoPrivate ())
{
    d->m_dotsPerMeterX = 0;
    d->m_dotsPerMeterY = 0;
    d->m_offset = QPoint (0, 0);
}

kpDocumentMetaInfo::kpDocumentMetaInfo (const kpDocumentMetaInfo &rhs)
    : d (new kpDocumentMetaInfoPrivate ())
{
    d->m_dotsPerMeterX = rhs.dotsPerMeterX ();
    d->m_dotsPerMeterY = rhs.dotsPerMeterY ();
    d->m_offset = rhs.offset ();
    d->m_textMap = rhs.textMap ();
}

// public
kpDocumentMetaInfo::~kpDocumentMetaInfo ()
{
    delete d;
}


// public
kpDocumentMetaInfo &kpDocumentMetaInfo::operator= (const kpDocumentMetaInfo &rhs)
{
    d->m_dotsPerMeterX = rhs.dotsPerMeterX ();
    d->m_dotsPerMeterY = rhs.dotsPerMeterY ();
    d->m_offset = rhs.offset ();
    d->m_textMap = rhs.textMap ();

    return *this;
}


// public
void kpDocumentMetaInfo::printDebug (const QString &prefix) const
{
    const QString usedPrefix = !prefix.isEmpty () ?
                                   prefix + QString::fromLatin1 (":") :
                                   QString::null;

    kdDebug () << usedPrefix << endl;

    kdDebug () << "dotsPerMeter X=" << dotsPerMeterX ()
               << " Y=" << dotsPerMeterY ()
               << " offset=" << offset () << endl;

    QValueList <QImageTextKeyLang> keyList = textList ();
    for (QValueList <QImageTextKeyLang>::const_iterator it = keyList.begin ();
         it != keyList.end ();
         it++)
    {
        kdDebug () << "key=" << (*it).key
                   << " lang=" << (*it).lang
                   << " text=" << text (*it)
                   << endl;
    }

    kdDebug () << usedPrefix << "ENDS" << endl;
}


// public
int kpDocumentMetaInfo::dotsPerMeterX () const
{
    return d->m_dotsPerMeterX;
}

// public
void kpDocumentMetaInfo::setDotsPerMeterX (int val)
{
    d->m_dotsPerMeterX = val;
}


// public
int kpDocumentMetaInfo::dotsPerMeterY () const
{
    return d->m_dotsPerMeterY;
}

// public
void kpDocumentMetaInfo::setDotsPerMeterY (int val)
{
    d->m_dotsPerMeterY = val;
}


// public
QPoint kpDocumentMetaInfo::offset () const
{
    return d->m_offset;
}

// public
void kpDocumentMetaInfo::setOffset (const QPoint &point)
{
    d->m_offset = point;
}


// public
QMap <QImageTextKeyLang, QString> kpDocumentMetaInfo::textMap () const
{
    return d->m_textMap;
}

// public
QValueList <QImageTextKeyLang> kpDocumentMetaInfo::textList () const
{
    return d->m_textMap.keys ();
}


// public
QString kpDocumentMetaInfo::text (const QImageTextKeyLang &itkl) const
{
    return d->m_textMap [itkl];
}

// public
QString kpDocumentMetaInfo::text (const char *key, const char *lang) const
{
    return text (QImageTextKeyLang (key, lang));
}


// public
void kpDocumentMetaInfo::setText (const QImageTextKeyLang &itkl,
                                  const QString &string)
{
    d->m_textMap [itkl] = string;
}

// public
void kpDocumentMetaInfo::setText (const char *key, const char *lang,
                                  const QString &string)
{
    setText (QImageTextKeyLang (key, lang), string);
}
