
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


#define DEBUG_DMITFTM 1


#include <kpDocumentMetaInfoTextFieldsTableModel.h>

#include <KDebug>
#include <KLocale>

#include <kpDocumentMetaInfo.h>


struct kpDocumentMetaInfoTextFieldsTableModelPrivate
{
    kpDocumentMetaInfoProvider *metaInfoProvider;
};

kpDocumentMetaInfoTextFieldsTableModel::kpDocumentMetaInfoTextFieldsTableModel (
        kpDocumentMetaInfoProvider *metaInfoProvider,
        QObject *parent)
    : QAbstractTableModel (parent),
      d (new kpDocumentMetaInfoTextFieldsTableModelPrivate ())
{
    d->metaInfoProvider = metaInfoProvider;
}

kpDocumentMetaInfoTextFieldsTableModel::~kpDocumentMetaInfoTextFieldsTableModel ()
{
    delete d;
}


// private
kpDocumentMetaInfo *kpDocumentMetaInfoTextFieldsTableModel::metaInfo () const
{
    return d->metaInfoProvider->metaInfo ();
}


// public virtual [base QAbstractItemModel]
Qt::ItemFlags kpDocumentMetaInfoTextFieldsTableModel::flags (const QModelIndex &index) const
{
#if DEBUG_DMITFTM
    kDebug () << "DMITFTM::flags(index=" << index << ")" << endl;
#endif
    // All items are editable.
    (void) index;

    return Qt::ItemIsEditable;
}


// public virtual [base QAbstractItemModel]
QVariant kpDocumentMetaInfoTextFieldsTableModel::headerData (
        int section, Qt::Orientation orientation, int role) const
{
#if DEBUG_DMITFTM
    kDebug () << "DMITFTM::headerData(section=" << section
              << ",o=" << (orientation == Qt::Vertical ? "vert" : "horiz")
              << ",role=" << role << ")" << endl;
#endif

    // Numbered rows...
    if (orientation == Qt::Vertical)
    {
        return QAbstractTableModel::headerData (section, orientation, role);
    }
    else if (orientation == Qt::Horizontal)
    {
        if (role != Qt::DisplayRole)
            return QVariant ();

        if (section == 0)
            return i18n ("Key");
        else if (section == 1)
            return i18n ("Value");
        else
        {
            Q_ASSERT (!"unknown section");
            return QVariant ();
        }
    }
    else
    {
        Q_ASSERT (!"unknown orientation");
        return QVariant ();
    }
}


// public virtual [base QAbstractItemModel]
int kpDocumentMetaInfoTextFieldsTableModel::rowCount (const QModelIndex &parent) const
{
#if DEBUG_DMITFTM
    kDebug () << "DMITFTM::rowCount(parent=" << parent << ")" << endl;
#endif
    return metaInfo ()->textKeys ().size () + 1/*header*/;
}

// public virtual [base QAbstractItemModel]
int kpDocumentMetaInfoTextFieldsTableModel::columnCount (const QModelIndex &parent) const
{
#if DEBUG_DMITFTM
    kDebug () << "DMITFTM::columnCount(parent=" << parent << ")" << endl;
#endif
    return 2;
}

// public virtual [base QAbstractItemModel]
QVariant kpDocumentMetaInfoTextFieldsTableModel::data (const QModelIndex &parent,
        int role) const
{
#if DEBUG_DMITFTM
    kDebug () << "DMITFTM::data(parent="
              << parent << ",role=" << role << ")" << endl;
#endif

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant ();

    const int row = parent.row (), col = parent.column ();
    Q_ASSERT (row >= 0 && row < metaInfo ()->textKeys ().size () + 1);

    // Special extra row for adding new data.
    if (row == metaInfo ()->textKeys ().size ())
        return QVariant ();

    if (col == 0)
    {
        return metaInfo ()->textKeys () [row];
    }
    else if (col == 1)
    {
        return metaInfo ()->text (metaInfo ()->textKeys () [row]);
    }
    else
    {
        Q_ASSERT (!"invalid column");
        return QVariant ();
    }
}


#include <kpDocumentMetaInfoTextFieldsTableModel.moc>
