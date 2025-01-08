
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpDocumentMetaInfoDialog_H
#define kpDocumentMetaInfoDialog_H

#include <QDialog>

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
class kpDocumentMetaInfoDialog : public QDialog
{
    Q_OBJECT

public:
    kpDocumentMetaInfoDialog(const kpDocumentMetaInfo *docMetaInfo, QWidget *parent);
    ~kpDocumentMetaInfoDialog() override;

public:
    bool isNoOp() const;

    kpDocumentMetaInfo originalMetaInfo() const;

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
    // If QDialog::exec() succeeded, all data in the UI was valid so the
    // returned meta information will be complete and correct.
    //
    // This is a slow method as it recalculates the meta information each
    // time it's called.
    kpDocumentMetaInfo metaInfo(QString *errorMessage = nullptr) const;

private:
    void editCell(int r, int c);
    void fieldsUpdateVerticalHeader();

    void fieldsAddEmptyRow(int atRow);
    void fieldsAppendEmptyRow();

    bool isFieldsRowDeleteable(int row) const;
    void fieldsDeleteRow(int r);

    void enableFieldsDeleteRowButtonIfShould();

private Q_SLOTS:
    void setUIToOriginalMetaInfo();
    void slotFieldsCurrentCellChanged(int row, int col, int oldRow, int oldCol);

    // Allows the user to add a row without pressing any pushbuttons:
    // Appends a new, blank row when text has been added to the last row.
    void slotFieldsItemChanged(QTableWidgetItem *item);

    void slotFieldsAddRowButtonClicked();
    void slotFieldsDeleteRowButtonClicked();

    void accept() override;

private:
    struct kpDocumentMetaInfoDialogPrivate *const d;
};

#endif // kpDocumentMetaInfoDialog_H
