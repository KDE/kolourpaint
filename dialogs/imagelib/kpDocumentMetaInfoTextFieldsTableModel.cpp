
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


#include <kpDocumentMetaInfoTextFieldsTableModel.h>

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


// public virtual [base QAbstractTableModel]
int kpDocumentMetaInfoTextFieldsTableModel::rowCount (const QModelIndex &parent) const
{
    return d->metaInfoProvider->metaInfo ()->textKeys ().size () + 1/*header*/;
}

// public virtual [base QAbstractTableModel]
int kpDocumentMetaInfoTextFieldsTableModel::columnCount (const QModelIndex &parent) const
{
    return 2;
}

// public virtual [base QAbstractTableModel]
QVariant kpDocumentMetaInfoTextFieldsTableModel::data (const QModelIndex &parent,
        int role) const
{
    return QVariant ();
#if 0
    QHeaderView *fieldsHeader = new QHeaderView (Qt::Horizontal, fieldsGroupBox);
    fieldsHeader->setHeaderData (0, Qt::Horizontal,
    fieldsGroupBox->setHorizontalHeader (fieldsHeader);
        QStringList () << i18n ("Key") << i18n ("Value"));
    int row = 0;
    foreach (QString key, docMetaInfo->textKeys ())
    {
        fieldsGroupBox->setItem (row, 0/*1st col*/,
            new QTableWidgetItem (key));
        fieldsGroupBox->setItem (row, 1/*2nd col*/,
            new QTableWidgetItem (docMetaInfo->text (key));

        row++;
    }
#endif
}


#include <kpDocumentMetaInfoTextFieldsTableModel.moc>
