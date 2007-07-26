
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


#include <kpDocumentMetaInfoDialog.h>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QVBoxLayout>

#include <KIntNumInput>
#include <KLocale>

#include <kpDocumentMetaInfo.h>
#include <kpDocumentMetaInfoTextFieldsTableModel.h>


struct kpDocumentMetaInfoDialogPrivate
{
    const kpDocumentMetaInfo *startingDocMetaInfo;
    kpDocumentMetaInfo newDocMetaInfo;

    KIntNumInput *horizDpiInput, *vertDpiInput;
    KIntNumInput *horizOffsetInput, *vertOffsetInput;

    QAbstractItemView *fieldsTableView;
    QAbstractItemModel *fieldsTableModel;
};

// "ought to be enough for anybody"
// TODO: Maybe there are some QImage constants somewhere?
static const int MaxDPI = 600/*a lot of DPI*/ * 100;
static const int MaxOffset = 4000/*a big image*/ * 100,
                 MinOffset = -::MaxOffset;

// (shared by all dialogs, across all main windows, in a KolourPaint instance)
static int LastWidth = -1, LastHeight = -1;

kpDocumentMetaInfoDialog::kpDocumentMetaInfoDialog (const kpDocumentMetaInfo *docMetaInfo,
                                                  QWidget *parent)
    : KDialog (parent),
      d (new kpDocumentMetaInfoDialogPrivate ())
{
    d->startingDocMetaInfo = docMetaInfo;
    d->newDocMetaInfo = *docMetaInfo;


    setCaption (i18n ("Document Properties"));
    setButtons (KDialog::Ok | KDialog::Cancel);

    QWidget *baseWidget = new QWidget (this);
    setMainWidget (baseWidget);


    QGroupBox *dpiGroupBox = new QGroupBox (i18n ("Resolution (DPI)"),
        baseWidget);

    // Top Row:
    QLabel *horizDpiLabel = new QLabel (i18n ("&Horizontal:"));
    horizDpiLabel->setAlignment (Qt::AlignHCenter);
    // --- Gap --
    QLabel *vertDpiLabel = new QLabel (i18n ("&Vertical:"));
    vertDpiLabel->setAlignment (Qt::AlignHCenter);

    // Bottom Row:

    d->horizDpiInput = new KIntNumInput (dpiGroupBox);
    d->horizDpiInput->setRange (0, ::MaxDPI, 1/*step*/, false/*no slider*/);
    d->horizDpiInput->setSpecialValueText (i18n ("Unspecified"));

    QLabel *dpiXLabel = new QLabel (i18n ("x"), dpiGroupBox);
    dpiXLabel->setAlignment (Qt::AlignHCenter);

    d->vertDpiInput = new KIntNumInput (dpiGroupBox);
    d->vertDpiInput->setRange (0, ::MaxDPI, 1/*step*/, false/*no slider*/);
    d->vertDpiInput->setSpecialValueText (i18n ("Unspecified"));


    horizDpiLabel->setBuddy (d->horizDpiInput);
    vertDpiLabel->setBuddy (d->vertDpiInput);


    // We need a full blown QGridLayout -- instead of using
    // KIntNumInput::setLabel() so that the 'x' can be located
    // not necessarily vertically centered on the bottom row but
    // certainly not flush bottom.
    QGridLayout *dpiLay = new QGridLayout (dpiGroupBox);
    dpiLay->setSpacing (spacingHint ());
    dpiLay->setMargin (marginHint () * 2);

    dpiLay->addWidget (horizDpiLabel, 0, 0);
    // --- Gap ---
    dpiLay->addWidget (vertDpiLabel, 0, 2);

    dpiLay->addWidget (d->horizDpiInput, 1, 0);
    dpiLay->addWidget (dpiXLabel, 1, 1);
    dpiLay->addWidget (d->vertDpiInput, 1, 2);


    QGroupBox *offsetGroupBox = new QGroupBox (i18n ("Offset"), baseWidget);

    d->horizOffsetInput = new KIntNumInput (offsetGroupBox);
    d->horizOffsetInput->setLabel (i18n ("Horizontal:"),
        Qt::AlignTop | Qt::AlignHCenter);
    d->horizOffsetInput->setRange (::MinOffset, ::MaxOffset,
        1/*step*/, false/*no slider*/);

    d->vertOffsetInput = new KIntNumInput (offsetGroupBox);
    d->vertOffsetInput->setLabel (i18n ("Vertical:"),
        Qt::AlignTop | Qt::AlignHCenter);
    d->vertOffsetInput->setRange (::MinOffset, ::MaxOffset,
        1/*step*/, false/*no slider*/);


    QHBoxLayout *offsetLay = new QHBoxLayout (offsetGroupBox);
    offsetLay->setSpacing (spacingHint ());
    offsetLay->setMargin (marginHint () * 2);

    offsetLay->addWidget (d->horizOffsetInput);
    offsetLay->addWidget (d->vertOffsetInput);


// TODO: Use for above
#if 0
    connect (m_colorSimilarityInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotColorSimilarityValueChanged ()));
#endif

    QGroupBox *fieldsGroupBox = new QGroupBox (i18n ("&Text Fields"), baseWidget);

    d->fieldsTableView = new QTableView (fieldsGroupBox);
    d->fieldsTableModel = new kpDocumentMetaInfoTextFieldsTableModel (this, this);
    d->fieldsTableView->setModel (d->fieldsTableModel);

    QVBoxLayout *fieldsLayout = new QVBoxLayout (fieldsGroupBox);
    fieldsLayout->setSpacing (spacingHint ());
    fieldsLayout->setMargin (marginHint () * 2);
    fieldsLayout->addWidget (d->fieldsTableView);


    QGridLayout *baseLayout = new QGridLayout (baseWidget);
    baseLayout->setSpacing (spacingHint ());
    baseLayout->setMargin (0/*margin*/);

    // Row 0
    baseLayout->addWidget (dpiGroupBox, 0, 0);
    baseLayout->addWidget (offsetGroupBox, 0, 1);

    // Row 1
    baseLayout->addWidget (fieldsGroupBox, 1, 0, 1/*row span*/, 2/*col span*/);
    baseLayout->setRowStretch (1, 1/*stretch*/);


    //
    // Fill in information
    //

    // TODO: Rounding error - use KDoubleNumInput instead
    d->horizDpiInput->setValue (d->startingDocMetaInfo->dotsPerMeterX () /
        (100 / 2.54));
    d->vertDpiInput->setValue (d->startingDocMetaInfo->dotsPerMeterY () /
        (100 / 2.54));

    d->horizOffsetInput->setValue (d->startingDocMetaInfo->offset ().x ());
    d->vertOffsetInput->setValue (d->startingDocMetaInfo->offset ().y ());


    if (::LastWidth > 0 && ::LastHeight > 0)
        resize (::LastWidth, ::LastHeight);
}

kpDocumentMetaInfoDialog::~kpDocumentMetaInfoDialog ()
{
    ::LastWidth = width (), ::LastHeight = height ();

    delete d;
}


// public
bool kpDocumentMetaInfoDialog::isNoOp () const
{
    return false;  // TODO (d->newDocMetaInfo == *d->startingDocMetaInfo);
}

// public virtual [kpDocumentMetaInfoProvider]
kpDocumentMetaInfo *kpDocumentMetaInfoDialog::metaInfo ()
{
    return &d->newDocMetaInfo;
}


#include <kpDocumentMetaInfoDialog.moc>
