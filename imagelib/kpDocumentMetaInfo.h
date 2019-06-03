
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


#ifndef KP_DOCUMENT_META_INFO_H
#define KP_DOCUMENT_META_INFO_H


#include <QImage>
#include <QList>
#include <QMap>
#include <QString>

#include "commands/kpCommandSize.h"


class QPoint;


class kpDocumentMetaInfo
{
public:
    kpDocumentMetaInfo ();
    kpDocumentMetaInfo (const kpDocumentMetaInfo &rhs);
    virtual ~kpDocumentMetaInfo ();

    bool operator== (const kpDocumentMetaInfo &rhs) const;
    bool operator!= (const kpDocumentMetaInfo &rhs) const;

    kpDocumentMetaInfo &operator= (const kpDocumentMetaInfo &rhs);


    void printDebug (const QString &prefix) const;


    kpCommandSize::SizeType size () const;


    //
    // Constants (enforced by methods)
    //

    static const int MinDotsPerMeter, MaxDotsPerMeter;
    static const int MinOffset, MaxOffset;


    // See QImage documentation

    // <val> is 0 if the resolution is unspecified.
    // Else, these methods automatically bound <val> to be between
    // MinDotsPerMeter ... MaxDotsPerMeter inclusive.
    int dotsPerMeterX () const;
    void setDotsPerMeterX (int val);

    // <val> is 0 if the resolution is unspecified.
    // Else, these methods automatically bound <val> to be between
    // MinDotsPerMeter ... MaxDotsPerMeter inclusive.
    int dotsPerMeterY () const;
    void setDotsPerMeterY (int val);


    // These methods automatically bound each of X and Y to be between
    // MinOffset and MaxOffset inclusive.
    QPoint offset () const;
    void setOffset (const QPoint &point);


    QMap <QString, QString> textMap () const;
    QList <QString> textKeys () const;

    // (if <key> is empty, it returns an empty string)
    QString text (const QString &key) const;

    // (if <key> is empty, the operation is ignored)
    void setText (const QString &key, const QString &value);


private:
    struct kpDocumentMetaInfoPrivate *d;
};


#endif  // KP_DOCUMENT_META_INFO_H
