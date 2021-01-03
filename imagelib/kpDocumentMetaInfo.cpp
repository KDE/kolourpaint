
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#include "kpDocumentMetaInfo.h"

#include <cmath>

#include <QPoint>

#include "kpLogCategories.h"

#include "kpDefs.h"


//
// Constants which "ought to be enough for anybody"
// LOTODO: Maybe there are some QImage constants somewhere?
//

// public static

// (round up to guarantee at least 1 dot per inch)
const int kpDocumentMetaInfo::MinDotsPerMeter =
    int (std::ceil (1/*single dot per inch - a very low DPI*/ * KP_INCHES_PER_METER) + 0.1);

const int kpDocumentMetaInfo::MaxDotsPerMeter =
    int ((600 * 100)/*a lot of DPI*/ * KP_INCHES_PER_METER);

// public static
const int kpDocumentMetaInfo::MaxOffset = (4000/*big image*/ * 100)/*a very big image*/;
const int kpDocumentMetaInfo::MinOffset = -kpDocumentMetaInfo::MaxOffset;

//---------------------------------------------------------------------

struct kpDocumentMetaInfoPrivate
{
    int m_dotsPerMeterX{}, m_dotsPerMeterY{};
    QPoint m_offset;

    QMap <QString, QString> m_textMap;
};

//---------------------------------------------------------------------

// public
kpDocumentMetaInfo::kpDocumentMetaInfo ()
    : d (new kpDocumentMetaInfoPrivate ())
{
    d->m_dotsPerMeterX = 0;
    d->m_dotsPerMeterY = 0;
    d->m_offset = QPoint (0, 0);
}

//---------------------------------------------------------------------

kpDocumentMetaInfo::kpDocumentMetaInfo (const kpDocumentMetaInfo &rhs)
    : d (new kpDocumentMetaInfoPrivate ())
{
    d->m_dotsPerMeterX = rhs.dotsPerMeterX ();
    d->m_dotsPerMeterY = rhs.dotsPerMeterY ();
    d->m_offset = rhs.offset ();
    d->m_textMap = rhs.textMap ();
}

//---------------------------------------------------------------------

// public
kpDocumentMetaInfo::~kpDocumentMetaInfo ()
{
    delete d;
}

//---------------------------------------------------------------------

// public
bool kpDocumentMetaInfo::operator== (const kpDocumentMetaInfo &rhs) const
{
    return (d->m_dotsPerMeterX == rhs.d->m_dotsPerMeterX &&
            d->m_dotsPerMeterY == rhs.d->m_dotsPerMeterY &&
            d->m_offset == rhs.d->m_offset &&
            d->m_textMap == rhs.d->m_textMap);
}

//---------------------------------------------------------------------

// public
bool kpDocumentMetaInfo::operator!= (const kpDocumentMetaInfo &rhs) const
{
    return !(*this == rhs);
}

//---------------------------------------------------------------------

// public
kpDocumentMetaInfo &kpDocumentMetaInfo::operator= (const kpDocumentMetaInfo &rhs)
{
    if (this == &rhs) {
        return *this;
    }

    d->m_dotsPerMeterX = rhs.dotsPerMeterX ();
    d->m_dotsPerMeterY = rhs.dotsPerMeterY ();
    d->m_offset = rhs.offset ();
    d->m_textMap = rhs.textMap ();

    return *this;
}

//---------------------------------------------------------------------

// public
void kpDocumentMetaInfo::printDebug (const QString &prefix) const
{
    const QString usedPrefix = !prefix.isEmpty() ? QString(prefix + QLatin1String(":")) : QString();

    qCDebug(kpLogImagelib) << usedPrefix;

    qCDebug(kpLogImagelib) << "dotsPerMeter X=" << dotsPerMeterX ()
               << " Y=" << dotsPerMeterY ()
               << " offset=" << offset ();

    foreach (const QString &key, textKeys())
      qCDebug(kpLogImagelib) << "key=" << key << " text=" << text(key);

    qCDebug(kpLogImagelib) << usedPrefix << "ENDS";
}

//---------------------------------------------------------------------

// public
kpCommandSize::SizeType kpDocumentMetaInfo::size () const
{
    kpCommandSize::SizeType ret = 0;

    for (const auto &key : d->m_textMap.keys ())
    {
        ret += kpCommandSize::StringSize (key) +
               kpCommandSize::StringSize (d->m_textMap [key]);
    }

    // We don't know what the QMap size overhead is so overestimate the size
    // rather than underestimating it.
    // LOTODO: Find the proper size in bytes.
    return ret * 3;
}

//---------------------------------------------------------------------

// public
int kpDocumentMetaInfo::dotsPerMeterX () const
{
    return d->m_dotsPerMeterX;
}

//---------------------------------------------------------------------

// public
void kpDocumentMetaInfo::setDotsPerMeterX (int val)
{
    // Unspecified resolution?
    if (val == 0)
    {
        d->m_dotsPerMeterX = 0;
        return;
    }

    d->m_dotsPerMeterX = qBound (MinDotsPerMeter, val, MaxDotsPerMeter);
}

//---------------------------------------------------------------------

// public
int kpDocumentMetaInfo::dotsPerMeterY () const
{
    return d->m_dotsPerMeterY;
}

//---------------------------------------------------------------------

// public
void kpDocumentMetaInfo::setDotsPerMeterY (int val)
{
    // Unspecified resolution?
    if (val == 0)
    {
        d->m_dotsPerMeterY = 0;
        return;
    }

    d->m_dotsPerMeterY = qBound (MinDotsPerMeter, val, MaxDotsPerMeter);
}

//---------------------------------------------------------------------

// public
QPoint kpDocumentMetaInfo::offset () const
{
    return d->m_offset;
}

//---------------------------------------------------------------------

// public
void kpDocumentMetaInfo::setOffset (const QPoint &point)
{
    const int x = qBound (MinOffset, point.x (), MaxOffset);
    const int y = qBound (MinOffset, point.y (), MaxOffset);

    d->m_offset = QPoint (x, y);
}

//---------------------------------------------------------------------

// public
QMap <QString, QString> kpDocumentMetaInfo::textMap () const
{
    return d->m_textMap;
}

//---------------------------------------------------------------------

// public
QList <QString> kpDocumentMetaInfo::textKeys () const
{
    return d->m_textMap.keys ();
}

//---------------------------------------------------------------------

// public
QString kpDocumentMetaInfo::text (const QString &key) const
{
    if (key.isEmpty ()) {
        return {};
    }

    return d->m_textMap [key];
}

//---------------------------------------------------------------------

// public
void kpDocumentMetaInfo::setText (const QString &key, const QString &value)
{
    if (key.isEmpty ()) {
        return;
    }

    d->m_textMap [key] = value;
}

//---------------------------------------------------------------------
