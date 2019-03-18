
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2007 John Layt <john@layt.net>
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


#define DEBUG_KP_PRINT_DIALOG_PAGE 0


#include "kpPrintDialogPage.h"

#include <QRadioButton>
#include <QVBoxLayout>
#include <QPrinter>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "kpDefs.h"


struct kpPrintDialogPagePrivate
{
    QRadioButton *printCenteredRadio, *printTopLeftRadio;
};

kpPrintDialogPage::kpPrintDialogPage (QWidget *parent)
    : QWidget (parent),
      d (new kpPrintDialogPagePrivate ())
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    qCDebug(kpLogWidgets) << "kpPrintDialogPage::<ctor>()";
#endif

    setWindowTitle (i18nc ("@title:tab", "I&mage Position"));

    d->printCenteredRadio = new QRadioButton (i18n ("&Center of the page"),
        this);
    d->printTopLeftRadio = new QRadioButton (i18n ("Top-&left of the page"),
        this);

    auto *lay = new QVBoxLayout (this);
    lay->addWidget (d->printCenteredRadio);
    lay->addWidget (d->printTopLeftRadio);
    lay->addStretch ();

    setPrintImageCenteredOnPage (true);
}

kpPrintDialogPage::~kpPrintDialogPage ()
{
    delete d;
}


bool kpPrintDialogPage::printImageCenteredOnPage ()
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    qCDebug(kpLogWidgets) << "kpPrintDialogPage::printImageCenteredOnPage()"
              << " returning " << d->printCenteredRadio->isChecked();
#endif
    return d->printCenteredRadio->isChecked ();
}


void kpPrintDialogPage::setPrintImageCenteredOnPage (bool printCentered)
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    qCDebug(kpLogWidgets) << "kpPrintDialogPage::setOptions(" << printCentered << ")";
#endif
    if (printCentered) {
        d->printCenteredRadio->setChecked (true);
    }
    else {
        d->printTopLeftRadio->setChecked (true);
    }
}


