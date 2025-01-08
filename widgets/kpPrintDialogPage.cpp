
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_PRINT_DIALOG_PAGE 0

#include "kpPrintDialogPage.h"

#include <QRadioButton>
#include <QVBoxLayout>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "kpDefs.h"

struct kpPrintDialogPagePrivate {
    QRadioButton *printCenteredRadio, *printTopLeftRadio;
};

kpPrintDialogPage::kpPrintDialogPage(QWidget *parent)
    : QWidget(parent)
    , d(new kpPrintDialogPagePrivate())
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    qCDebug(kpLogWidgets) << "kpPrintDialogPage::<ctor>()";
#endif

    setWindowTitle(i18nc("@title:tab", "I&mage Position"));

    d->printCenteredRadio = new QRadioButton(i18n("&Center of the page"), this);
    d->printTopLeftRadio = new QRadioButton(i18n("Top-&left of the page"), this);

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(d->printCenteredRadio);
    lay->addWidget(d->printTopLeftRadio);
    lay->addStretch();

    setPrintImageCenteredOnPage(true);
}

kpPrintDialogPage::~kpPrintDialogPage()
{
    delete d;
}

bool kpPrintDialogPage::printImageCenteredOnPage()
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    qCDebug(kpLogWidgets) << "kpPrintDialogPage::printImageCenteredOnPage()"
                          << " returning " << d->printCenteredRadio->isChecked();
#endif
    return d->printCenteredRadio->isChecked();
}

void kpPrintDialogPage::setPrintImageCenteredOnPage(bool printCentered)
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    qCDebug(kpLogWidgets) << "kpPrintDialogPage::setOptions(" << printCentered << ")";
#endif
    if (printCentered) {
        d->printCenteredRadio->setChecked(true);
    } else {
        d->printTopLeftRadio->setChecked(true);
    }
}

#include "moc_kpPrintDialogPage.cpp"
