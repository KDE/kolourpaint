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


#define DEBUG_KP_DOCUMENT_META_INFO_DIALOG 0


#include "kpDocumentMetaInfoDialog.h"

#include "kpDefs.h"
#include "imagelib/kpDocumentMetaInfo.h"

#include <KLocalizedString>
#include <KMessageBox>
#include "kpLogCategories.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>


struct kpDocumentMetaInfoDialogPrivate
{
    const kpDocumentMetaInfo *originalMetaInfoPtr;

    QDoubleSpinBox *horizDpiInput, *vertDpiInput;
    QSpinBox *horizOffsetInput, *vertOffsetInput;

    QTableWidget *fieldsTableWidget;
    QPushButton *fieldsAddRowButton, *fieldsDeleteRowButton, *fieldsResetButton;
};


// (shared by all dialogs, across all main windows, in a KolourPaint instance)
static int LastWidth = -1, LastHeight = -1;


// sync: You must keep DpiMinStep = 10 ^ (-DpiPrecision).
//
// You can increase the precision to reduce the chance of inadvertently changing
// the resolution when converting from kpDocumentMetaInfo's "dots per meter"
// to our "dots per inch" and back.  It would be bad if simply going into this
// dialog and pressing OK changed the resolution (it's unlikely but I still think
// it might happen with the current precision).
// TODO: On a related note, for many particular resolutions, if the user enters
//       one of them into the UI, presses OK and then comes back to the dialog,
//       s/he is presented with a different resolution to the one typed in.
//       Maybe make DotsPerMeter[XY] be of type "double" instead of "int" to
//       solve this problem?
//
// Of course, if you increase the precision too much, the minimum step will
// become so small that it will start experiencing floating point inaccuracies
// esp. since we use it for the "DpiUnspecified" hack.
static const int DpiPrecision = 3;
static const double DpiMinStep = 0.001;

static const double DpiLegalMin =
    kpDocumentMetaInfo::MinDotsPerMeter / KP_INCHES_PER_METER;

// Out of range represents unspecified DPI.
static const double DpiUnspecified = ::DpiLegalMin - ::DpiMinStep;

static const double DpiInputMin = ::DpiUnspecified;
static const double DpiInputMax =
    kpDocumentMetaInfo::MaxDotsPerMeter / KP_INCHES_PER_METER;

// The increment the DPI spinboxes jump by when they're clicked.
//
// We make this relatively big since people don't usually just increase their
// DPIs by 1 or so -- they are usually changing from say, 72, to 96.
//
// Obviously, making it equal to DpiMinStep is too slow a UI.  Therefore, with
// our big setting, the user will still have to manually change the value in
// the spinbox, using the keyboard, after all their clicking to ensure it is
// exactly the value they want.
static const double DpiInputStep = 10;


// TODO: Halve groupbox layout margins in every other file since it doesn't
//       seem to be need in Qt4.
kpDocumentMetaInfoDialog::kpDocumentMetaInfoDialog (
        const kpDocumentMetaInfo *docMetaInfo,
        QWidget *parent)

    : QDialog (parent),
      d (new kpDocumentMetaInfoDialogPrivate ())
{
    d->originalMetaInfoPtr = docMetaInfo;


    setWindowTitle (i18nc ("@title:window", "Document Properties"));
    auto * buttons = new QDialogButtonBox (QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel, this);

    connect (buttons, &QDialogButtonBox::accepted, this, &kpDocumentMetaInfoDialog::accept);
    connect (buttons, &QDialogButtonBox::rejected, this, &kpDocumentMetaInfoDialog::reject);

    auto *baseWidget = new QWidget (this);

    auto *dialogLayout = new QVBoxLayout (this);
    dialogLayout->addWidget (baseWidget);
    dialogLayout->addWidget (buttons);


    //
    // DPI Group Box
    //

    Q_ASSERT (::DpiInputMin < ::DpiInputMax);

    auto *dpiGroupBox = new QGroupBox(i18n("Dots &Per Inch (DPI)"), baseWidget);

    d->horizDpiInput = new QDoubleSpinBox(dpiGroupBox);
    d->horizDpiInput->setRange(::DpiInputMin, ::DpiInputMax);
    d->horizDpiInput->setValue(0.0);
    d->horizDpiInput->setSingleStep(::DpiInputStep);
    d->horizDpiInput->setDecimals(::DpiPrecision);
    d->horizDpiInput->setSpecialValueText(i18n("Unspecified"));

    auto *dpiXLabel = new QLabel (
        i18nc ("Horizontal DPI 'x' Vertical DPI", " x "), dpiGroupBox);
    dpiXLabel->setAlignment (Qt::AlignCenter);

    d->vertDpiInput = new QDoubleSpinBox(dpiGroupBox);
    d->vertDpiInput->setRange(::DpiInputMin, ::DpiInputMax);
    d->vertDpiInput->setValue(0.0);
    d->vertDpiInput->setSingleStep(::DpiInputStep);
    d->vertDpiInput->setDecimals(::DpiPrecision);
    d->vertDpiInput->setSpecialValueText(i18n("Unspecified"));


    auto *dpiLay = new QGridLayout(dpiGroupBox);

    dpiLay->addWidget(new QLabel(i18n("Horizontal:")), 0, 0, Qt::AlignHCenter);
    dpiLay->addWidget(d->horizDpiInput, 1, 0);
    dpiLay->addWidget(dpiXLabel, 0, 1);
    dpiLay->addWidget(new QLabel(i18n("Vertical:")), 0, 2, Qt::AlignHCenter);
    dpiLay->addWidget(d->vertDpiInput, 1, 2);

    dpiLay->setRowStretch(2, 1);


    dpiGroupBox->setWhatsThis (
        i18n (
            "<qt>"
            "<p><b>Dots Per Inch</b> (DPI) specifies the number of pixels"
            " of the image that should be printed inside one inch (2.54cm).</p>"

            "<p>The higher the image's DPI, the smaller the printed image."
            " Note that your printer is unlikely to produce high"
            " quality prints if you increase this to more than 300 or 600 DPI,"
            " depending on the printer.</p>"

            "<p>If you would like to print the image so that it is the same"
            " size as it is displayed on the screen, set the image's DPI"
            " values to be the same as the screen's.</p>"

            // TODO: This is currently not true!
            //       See "96dpi" TODO in kpMainWindow::sendPixmapToPrinter().
            //       This also why we don't try to report the current screen DPI
            //       for the above paragraph.
            "<p>If either DPI value is <b>Unspecified</b>, the image will also"
            " be printed to be the same size as on the screen.</p>"

            "<p>Not all image formats support DPI values. If the format you"
            " save in does not support them, they will not be saved.</p>"
            "</qt>"
        ));


    //
    // Offset Group Box
    //

    auto *offsetGroupBox = new QGroupBox(i18n ("O&ffset"), baseWidget);

    d->horizOffsetInput = new QSpinBox;
    d->horizOffsetInput->setRange(kpDocumentMetaInfo::MinOffset, kpDocumentMetaInfo::MaxOffset);

    d->vertOffsetInput = new QSpinBox;
    d->vertOffsetInput->setRange(kpDocumentMetaInfo::MinOffset, kpDocumentMetaInfo::MaxOffset);

    auto *offsetLay = new QGridLayout(offsetGroupBox);

    offsetLay->addWidget(new QLabel(i18n("Horizontal:")), 0, 0, Qt::AlignHCenter);
    offsetLay->addWidget(d->horizOffsetInput, 1, 0);
    offsetLay->addWidget(new QLabel(i18n("Vertical:")), 0, 1, Qt::AlignHCenter);
    offsetLay->addWidget(d->vertOffsetInput, 1, 1);

    offsetLay->setRowStretch (2, 1);


    offsetGroupBox->setWhatsThis (
        i18n (
            "<qt>"
            "<p>The <b>Offset</b> is the relative position where this image"
            " should be placed, compared to other images.</p>"

            "<p>Not all image formats support the <b>Offset</b> feature."
            " If the format you save in does not support it, the values"
            " specified here will not be saved.</p>"
            "</qt>"
        ));


    //
    // Fields Group Box
    //


    auto *fieldsGroupBox = new QGroupBox (i18n ("&Text Fields"),
        baseWidget);

    d->fieldsTableWidget = new QTableWidget (fieldsGroupBox);
    d->fieldsTableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);

    connect (d->fieldsTableWidget, &QTableWidget::currentCellChanged,
             this, &kpDocumentMetaInfoDialog::slotFieldsCurrentCellChanged);

    connect (d->fieldsTableWidget, &QTableWidget::itemChanged,
             this, &kpDocumentMetaInfoDialog::slotFieldsItemChanged);

    d->fieldsAddRowButton = new QPushButton (i18n ("&Add Row"),
        fieldsGroupBox);
    connect (d->fieldsAddRowButton, &QPushButton::clicked,
             this, &kpDocumentMetaInfoDialog::slotFieldsAddRowButtonClicked);

    d->fieldsDeleteRowButton = new QPushButton (i18n ("&Delete Row"),
        fieldsGroupBox);
    connect (d->fieldsDeleteRowButton, &QPushButton::clicked,
             this, &kpDocumentMetaInfoDialog::slotFieldsDeleteRowButtonClicked);

    d->fieldsResetButton = new QPushButton (i18n ("&Reset"),
        fieldsGroupBox);
    connect (d->fieldsResetButton, &QPushButton::clicked,
             this, &kpDocumentMetaInfoDialog::setUIToOriginalMetaInfo);

    auto *fieldsButtonsLayout = new QHBoxLayout ();
    fieldsButtonsLayout->addWidget (d->fieldsAddRowButton);
    fieldsButtonsLayout->addWidget (d->fieldsDeleteRowButton);
    fieldsButtonsLayout->addStretch ();
    fieldsButtonsLayout->addWidget (d->fieldsResetButton);

    auto *fieldsLayout = new QVBoxLayout (fieldsGroupBox);

    fieldsLayout->addWidget (d->fieldsTableWidget);
    fieldsLayout->addLayout (fieldsButtonsLayout);


    fieldsGroupBox->setWhatsThis (
        i18n (
            "<qt>"
            "<p><b>Text Fields</b> provide extra information about the image."
            " This is probably a comment area that you can freely write any text in.</p>"

            "<p>However, this is format-specific so the fields could theoretically be"
            " computer-interpreted data - that you should not modify -"
            " but this is unlikely.</p>"

            "<p>Not all image formats support <b>Text Fields</b>. If the format"
            " you save in does not support them, they will not be saved.</p>"
            "</qt>"
        ));


    //
    // Global Layout
    //
    auto *baseLayout = new QGridLayout (baseWidget);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    // Col 0
    baseLayout->addWidget (dpiGroupBox, 0, 0);
    baseLayout->addWidget (offsetGroupBox, 1, 0);

    // Col 1
    baseLayout->addWidget (fieldsGroupBox, 0, 1, 2/*row span*/, 1/*col span*/);
    baseLayout->setColumnStretch (1, 1/*stretch*/);


    //
    // Remaining UI Setup
    //


    setUIToOriginalMetaInfo ();


    if (::LastWidth > 0 && ::LastHeight > 0) {
        resize (::LastWidth, ::LastHeight);
    }
}

//---------------------------------------------------------------------

kpDocumentMetaInfoDialog::~kpDocumentMetaInfoDialog ()
{
    ::LastWidth = width ();
    ::LastHeight = height ();

    delete d;
}

//---------------------------------------------------------------------
// private

void kpDocumentMetaInfoDialog::editCell (int r, int c)
{
    d->fieldsTableWidget->setCurrentCell (r, c);
    d->fieldsTableWidget->editItem (d->fieldsTableWidget->item (r, c));
}

//---------------------------------------------------------------------
// private slot

void kpDocumentMetaInfoDialog::setUIToOriginalMetaInfo ()
{
    // Set DPI spinboxes.
    d->horizDpiInput->setValue (d->originalMetaInfoPtr->dotsPerMeterX () /
        KP_INCHES_PER_METER);
    d->vertDpiInput->setValue (d->originalMetaInfoPtr->dotsPerMeterY () /
        KP_INCHES_PER_METER);


    // Set Offset spinboxes.
    d->horizOffsetInput->setValue (d->originalMetaInfoPtr->offset ().x ());
    d->vertOffsetInput->setValue (d->originalMetaInfoPtr->offset ().y ());


    // Set Text Fields.
    //
    // Block itemChanged() signal as slotFieldsItemChanged() should not get called
    // when rows are half-created.
    const bool b = d->fieldsTableWidget->blockSignals (true);
    {
        d->fieldsTableWidget->clear ();

        d->fieldsTableWidget->setRowCount (d->originalMetaInfoPtr->textKeys ().size ());
        d->fieldsTableWidget->setColumnCount (2);

        QStringList fieldsHeader;
        fieldsHeader << i18n ("Key") << i18n ("Value");
        d->fieldsTableWidget->setHorizontalHeaderLabels (fieldsHeader);

        int row = 0;
        for (const auto &key : d->originalMetaInfoPtr->textKeys ())
        {
            d->fieldsTableWidget->setItem (row, 0/*1st col*/,
                new QTableWidgetItem (key));
            d->fieldsTableWidget->setItem (row, 1/*2nd col*/,
                new QTableWidgetItem (d->originalMetaInfoPtr->text (key)));

            row++;
        }

        fieldsAppendEmptyRow ();
    }
    d->fieldsTableWidget->blockSignals (b);


    editCell (0/*row*/, 0/*col*/);


    enableFieldsDeleteRowButtonIfShould ();
}

//---------------------------------------------------------------------
// public

bool kpDocumentMetaInfoDialog::isNoOp () const
{
    return (metaInfo () == *d->originalMetaInfoPtr);
}

//---------------------------------------------------------------------

// public
kpDocumentMetaInfo kpDocumentMetaInfoDialog::originalMetaInfo () const
{
    return *d->originalMetaInfoPtr;
}

// public
kpDocumentMetaInfo kpDocumentMetaInfoDialog::metaInfo (
        QString *errorMessage) const
{
    if (errorMessage)
    {
        // No errors to start with.
        *errorMessage = QString ();
    }


    kpDocumentMetaInfo ret;


    if (d->horizDpiInput->value () < ::DpiLegalMin) {
        ret.setDotsPerMeterX (0/*unspecified*/);
    }
    else {
        ret.setDotsPerMeterX (qRound (d->horizDpiInput->value () * KP_INCHES_PER_METER));
    }

    if (d->vertDpiInput->value () < ::DpiLegalMin) {
        ret.setDotsPerMeterY (0/*unspecified*/);
    }
    else {
        ret.setDotsPerMeterY (qRound (d->vertDpiInput->value () * KP_INCHES_PER_METER));
    }


    ret.setOffset (QPoint (d->horizOffsetInput->value (),
                           d->vertOffsetInput->value ()));


    for (int r = 0; r < d->fieldsTableWidget->rowCount (); r++)
    {
        const QString key = d->fieldsTableWidget->item (r, 0)->text ();
        const QString value = d->fieldsTableWidget->item (r, 1)->text ();

        // Empty key?
        if (key.isEmpty ())
        {
            // Empty value too?
            if (value.isEmpty ())
            {
                // Ignore empty row.
                continue;
            }
            // Value without a key?


            if (errorMessage)
            {
                *errorMessage =
                        ki18n ("The text value \"%1\" on line %2 requires a key.")
                        .subs (value).subs (r + 1/*count from 1*/).toString ();

                // Print only 1 error message per method invocation.
                errorMessage = nullptr;
            }

            // Ignore.
            continue;

        }

        // Duplicate key?
        if (ret.textKeys ().contains (key))
        {
            if (errorMessage)
            {
                int q;
                for (q = 0; q < r; q++)
                {
                    if (d->fieldsTableWidget->item (q, 0)->text () == key) {
                        break;
                    }
                }
                Q_ASSERT (q != r);

                *errorMessage =
                    ki18n ("All text keys must be unique. The text key \"%1\""
                           " on lines %2 and %3 are identical.")
                    .subs (key)
                    .subs (q + 1/*count from 1*/)
                    .subs (r + 1/*count from 1*/)
                    .toString ();

                // Print only 1 error message per method invocation.
                errorMessage = nullptr;
            }

            // Ignore this duplicate - keep the first value of the key.
            continue;
        }

        ret.setText (key, value);

    }  // for (r = 0; r < table widget rows; r++) {


    return ret;
}


// private
void kpDocumentMetaInfoDialog::fieldsUpdateVerticalHeader ()
{
    QStringList vertLabels;
    for (int r = 1; r <= d->fieldsTableWidget->rowCount (); r++) {
        vertLabels << QString::number (r);
    }

    d->fieldsTableWidget->setVerticalHeaderLabels (vertLabels);
}


// private
void kpDocumentMetaInfoDialog::fieldsAddEmptyRow (int atRow)
{
    // Block itemChanged() signal as slotFieldsItemChanged() should not get called
    // when rows are half-created.
    const bool b = d->fieldsTableWidget->blockSignals (true);
    {
        d->fieldsTableWidget->insertRow (atRow);

        d->fieldsTableWidget->setItem (atRow, 0, new QTableWidgetItem (QString ()));
        d->fieldsTableWidget->setItem (atRow, 1, new QTableWidgetItem (QString ()));
    }
    d->fieldsTableWidget->blockSignals (b);

    // Hack around Qt's failure to redraw these sometimes.
    fieldsUpdateVerticalHeader ();

    enableFieldsDeleteRowButtonIfShould ();
}

// private
void kpDocumentMetaInfoDialog::fieldsAppendEmptyRow ()
{
    fieldsAddEmptyRow (d->fieldsTableWidget->rowCount ());
}


// private
bool kpDocumentMetaInfoDialog::isFieldsRowDeleteable (int row) const
{
    // Can't delete no row and can't delete last (always blank) row, which
    // is used to make it easy for the user to add rows without pressing
    // the "Add" button explicitly.
    return (row >= 0 && row < d->fieldsTableWidget->rowCount () - 1);
}

// private
void kpDocumentMetaInfoDialog::fieldsDeleteRow (int r)
{
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "kpDocumentMetaInfoDialog::fieldsDeleteRow("
              << "row=" << r << ")"
              << " currentRow=" << d->fieldsTableWidget->currentRow ();
#endif

    Q_ASSERT (isFieldsRowDeleteable (r));

    if (r == d->fieldsTableWidget->currentRow ())
    {
        // Assertion follows from previous assertion.
        const int newRow = r + 1;
    #if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
        qCDebug(kpLogDialogs) << "\tnewRow=" << newRow;
    #endif
        Q_ASSERT (newRow < d->fieldsTableWidget->rowCount ());

        int newCol = d->fieldsTableWidget->currentColumn ();
    #if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
        qCDebug(kpLogDialogs) << "\tnewCol=" << newCol;
    #endif
        if (newCol != 0 && newCol != 1)
        {
            newCol = 0;
        #if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
            qCDebug(kpLogDialogs) << "\t\tcorrecting to " << newCol;
        #endif
        }

        // WARNING: You must call this _before_ deleting the row.  Else, you'll
        //          trigger a Qt bug where if the editor is active on the row to
        //          be deleted, it might crash.  To reproduce, move this line to
        //          after removeRow() (and subtract 1 from newRow) and
        //          press the "Delete Row" button several times in succession
        //          very quickly.
        //
        // TODO: This usually results in a redraw error if the scrollbar scrolls
        //       after deleting the 2nd last row.  Qt bug.
        editCell (newRow, newCol);
    }


    d->fieldsTableWidget->removeRow (r);


    fieldsUpdateVerticalHeader ();

    enableFieldsDeleteRowButtonIfShould ();
}


// private
void kpDocumentMetaInfoDialog::enableFieldsDeleteRowButtonIfShould ()
{
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "kpDocumentMetaInfoDialog::enableFieldsDeleteRowButtonIfShould()";
#endif

    const int r = d->fieldsTableWidget->currentRow ();
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "\tr=" << r;
#endif

    d->fieldsDeleteRowButton->setEnabled (isFieldsRowDeleteable (r));
}


// private slot
void kpDocumentMetaInfoDialog::slotFieldsCurrentCellChanged (int row, int col,
        int oldRow, int oldCol)
{
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "kpDocumentMetaInfoDialog::slotFieldsCurrentCellChanged("
              << "row=" << row << ",col=" << col
              << ",oldRow=" << oldRow << ",oldCol=" << oldCol
              << ")" << endl;
#endif

    (void) row;
    (void) col;
    (void) oldRow;
    (void) oldCol;

    enableFieldsDeleteRowButtonIfShould ();
}

//---------------------------------------------------------------------
// private slot

void kpDocumentMetaInfoDialog::slotFieldsItemChanged (QTableWidgetItem *it)
{
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "kpDocumentMetaInfoDialog::slotFieldsItemChanged("
              << "item=" << it << ") rows=" << d->fieldsTableWidget->rowCount ();
#endif

    const int r = d->fieldsTableWidget->row (it);
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "\tr=" << r;
#endif
    Q_ASSERT (r >= 0 && r < d->fieldsTableWidget->rowCount ());

    const QString key = d->fieldsTableWidget->item (r, 0)->text ();
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << " key='" << key << "'";
#endif

    const QString value = d->fieldsTableWidget->item (r, 1)->text ();
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << " value='" << value << "'";
#endif

    // At the last row?
    if (r == d->fieldsTableWidget->rowCount () - 1)
    {
        // Typed some text?
        if (!key.isEmpty () || !value.isEmpty ())
        {
            // LOTODO: If we're called due to the cell's text being finalized
            //         as a result of the user pressing the "Add Row" button,
            //         should this really append a row since
            //         slotFieldsAddRowButtonClicked() button is going to add
            //         another one?  That's two rows when the user only clicked
            //         that button once!
            fieldsAppendEmptyRow ();
        }
    }
}

//---------------------------------------------------------------------
// private slot

void kpDocumentMetaInfoDialog::slotFieldsAddRowButtonClicked ()
{
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "kpDocumentMetaInfoDialog::slotFieldsAddRowButtonClicked()";
#endif

    const int r = d->fieldsTableWidget->currentRow ();
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "\tr=" << r;
#endif

    // (if no row is selected, r = -1)
    fieldsAddEmptyRow (r + 1);

    // Edit the key of this new row (column 0).
    // No one edits the value first (column 1).
    editCell ((r + 1)/*row*/, 0/*col*/);
}

//---------------------------------------------------------------------

// private slot
void kpDocumentMetaInfoDialog::slotFieldsDeleteRowButtonClicked ()
{
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "kpDocumentMetaInfoDialog::slotFieldsDeleteRowButtonClicked()";
#endif

    const int r = d->fieldsTableWidget->currentRow ();
#if DEBUG_KP_DOCUMENT_META_INFO_DIALOG
    qCDebug(kpLogDialogs) << "\tr=" << r;
#endif

    Q_ASSERT (isFieldsRowDeleteable (r));
    fieldsDeleteRow (r);
}


// private slot virtual [base QDialog]
void kpDocumentMetaInfoDialog::accept ()
{
    // Validate text fields.
    QString errorMessage;
    (void) metaInfo (&errorMessage);
    if (!errorMessage.isEmpty ())
    {
        KMessageBox::sorry (this, errorMessage, i18nc ("@title:window", "Invalid Text Fields"));
        return;
    }

    QDialog::accept ();
}


