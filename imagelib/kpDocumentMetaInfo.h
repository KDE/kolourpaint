
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
    kpDocumentMetaInfo();
    kpDocumentMetaInfo(const kpDocumentMetaInfo &rhs);
    virtual ~kpDocumentMetaInfo();

    bool operator==(const kpDocumentMetaInfo &rhs) const;
    bool operator!=(const kpDocumentMetaInfo &rhs) const;

    kpDocumentMetaInfo &operator=(const kpDocumentMetaInfo &rhs);

    void printDebug(const QString &prefix) const;

    kpCommandSize::SizeType size() const;

    //
    // Constants (enforced by methods)
    //

    static const int MinDotsPerMeter, MaxDotsPerMeter;
    static const int MinOffset, MaxOffset;

    // See QImage documentation

    // <val> is 0 if the resolution is unspecified.
    // Else, these methods automatically bound <val> to be between
    // MinDotsPerMeter ... MaxDotsPerMeter inclusive.
    int dotsPerMeterX() const;
    void setDotsPerMeterX(int val);

    // <val> is 0 if the resolution is unspecified.
    // Else, these methods automatically bound <val> to be between
    // MinDotsPerMeter ... MaxDotsPerMeter inclusive.
    int dotsPerMeterY() const;
    void setDotsPerMeterY(int val);

    // These methods automatically bound each of X and Y to be between
    // MinOffset and MaxOffset inclusive.
    QPoint offset() const;
    void setOffset(const QPoint &point);

    QMap<QString, QString> textMap() const;
    QList<QString> textKeys() const;

    // (if <key> is empty, it returns an empty string)
    QString text(const QString &key) const;

    // (if <key> is empty, the operation is ignored)
    void setText(const QString &key, const QString &value);

private:
    struct kpDocumentMetaInfoPrivate *d;
};

#endif // KP_DOCUMENT_META_INFO_H
