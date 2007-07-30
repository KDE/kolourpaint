
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


#ifndef kpDocumentMetaInfoDialog_H
#define kpDocumentMetaInfoDialog_H


#include <kdialog.h>


class QTableWidgetItem;

class kpDocumentMetaInfo;


// Dialog for editing document meta information (see kpDocumentMetaInfo).
// It contains:
//
// 1. DPI spinboxes
// 2. Offset spinboxes
// 3. Text Fields
//
// The Text Fields widget always keeps an empty key-value row at the bottom.
// If text is entered in this row, a new one is created.  This allows users
// to add new rows without pressing any pushbuttons.
class kpDocumentMetaInfoDialog : public KDialog
{
Q_OBJECT

public:
    kpDocumentMetaInfoDialog (const kpDocumentMetaInfo *docMetaInfo,
                              QWidget *parent);
    virtual ~kpDocumentMetaInfoDialog ();


private:
    void editCell (int r, int c);

private slots:
    void setUIToOriginalMetaInfo ();


public:
    bool isNoOp () const;

    kpDocumentMetaInfo originalMetaInfo () const;

    // Returns the meta information gathered from all the UI.
    //
    // If there is any invalid data in the UI (e.g. duplicate text field
    // keys), the returned meta information will still be valid but will not
    // contain the complete contents of the UI (this description is
    // deliberately vague so that you don't use the return value in such a
    // situation).  In this situation, if <errorMessage> is set, a non-empty,
    // translated error message will be returned through it.
    //
    // If all data in the UI is valid and <errorMessage> is set, an empty
    // string will be returned through it.
    //
    // If KDialog::exec() succeeded, all data in the UI was valid so the
    // returned meta information will be complete and correct.
    //
    // This is a slow method as it recalculates the meta information each
    // time it's called.
    kpDocumentMetaInfo metaInfo (QString *errorMessage = 0) const;


private:
    void fieldsUpdateVerticalHeader ();

    void fieldsAddEmptyRow (int atRow);
    void fieldsAppendEmptyRow ();

private:
    bool isFieldsRowDeleteable (int row) const;
    void fieldsDeleteRow (int r);

    void enableFieldsDeleteRowButtonIfShould ();


private slots:
    void slotFieldsCurrentCellChanged (int row, int col, int oldRow, int oldCol);
    void slotFieldsItemActivated (QTableWidgetItem *item);

    // Allows the user to add a row without pressing any pushbuttons:
    // Appends a new, blank row when text has been added to the last row.
    void slotFieldsItemChanged (QTableWidgetItem *item);

    void slotFieldsItemClicked (QTableWidgetItem *item);
    void slotFieldsItemEntered (QTableWidgetItem *item);
    void slotFieldsItemPressed (QTableWidgetItem *item);

    void slotFieldsAddRowButtonClicked ();
    void slotFieldsDeleteRowButtonClicked ();


private slots:
    virtual void accept ();


private:
    struct kpDocumentMetaInfoDialogPrivate * const d;
};


#endif  // kpDocumentMetaInfoDialog_H
