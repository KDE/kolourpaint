
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


#define DEBUG_KP_PRINT_DIALOG_PAGE 1


#include <kpPrintDialogPage.h>

#include <QRadioButton>
#include <QVBoxLayout>

#include <KDebug>
#include <KDialog>
#include <KLocale>
#include <KPrinter>
#include <KVBox>

#include <kpDefs.h>


// HITODO: It's not saving the option (kdelibs crashes on exit).
//         And it doesn't PROPAGATE interprocess.
#define OptionPrintImageCentered "kde-kolourpaint-print_image_centered_on_page"


struct kpPrintDialogPagePrivate
{
    QRadioButton *printCenteredRadio, *printTopLeftRadio;
};

kpPrintDialogPage::kpPrintDialogPage (QWidget *parent)
    : KPrintDialogPage (parent),
      d (new kpPrintDialogPagePrivate ())
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    kDebug () << "kpPrintDialogPage::<ctor>()" << endl;
#endif

    setTitle (i18n ("Ima&ge Position"));

    KVBox *base = new KVBox (this);
    base->setMargin (KDialog::marginHint ());

    d->printCenteredRadio = new QRadioButton (i18n ("&Center of the page"),
        base);
    d->printTopLeftRadio = new QRadioButton (i18n ("Top-&left of the page"),
        base);
}

kpPrintDialogPage::~kpPrintDialogPage ()
{
    delete d;
}


static bool ShouldPrintImageCenteredOnPage (const QString &printCenteredStr)
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    kDebug () << "kpPrintDialogPage.cpp:ShouldPrintImageCenteredOnPage('"
              << printCenteredStr << "')" << endl;
#endif
    return (printCenteredStr.isEmpty () || printCenteredStr == "true");
}

// public static
bool kpPrintDialogPage::shouldPrintImageCenteredOnPage (KPrinter *printer)
{
    const QString printCenteredStr = printer->option (OptionPrintImageCentered);
    const bool ret = ::ShouldPrintImageCenteredOnPage (printCenteredStr);
#if DEBUG_KP_PRINT_DIALOG_PAGE
    kDebug () << "kpPrintDialogPage::shouldPrintImageCenteredOnPage()"
              << " returning " << ret << endl;
#endif
    return ret;
}


// public virtual [base KPrintDialogPage]
void kpPrintDialogPage::getOptions (QMap <QString, QString> &options,
        bool changeEvenIfUsingDefaultValues)
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    kDebug () << "kpPrintDialogPage::getOptions(changeEvenIfUsingDefaultValues="
              << changeEvenIfUsingDefaultValues << ")" << endl;
#endif
    const bool printCentered = d->printCenteredRadio->isChecked ();

// SYNC: KPrinter bug?
//       Changing the checkbox from the non-default (false) to the default
//       value (true) does not work if we ignore default values.
#if 0
    if (changeEvenIfUsingDefaultValues || printCentered != true/*default*/)
#endif
    {
    #if DEBUG_KP_PRINT_DIALOG_PAGE
        kDebug () << "\tsetting config" << endl;
    #endif
        options [OptionPrintImageCentered] = printCentered ?
            "true" :
            "false";
    }
}

// public virtual [base KPrintDialogPage]
void kpPrintDialogPage::setOptions (const QMap <QString, QString> &options)
{
#if DEBUG_KP_PRINT_DIALOG_PAGE
    kDebug () << "kpPrintDialogPage::setOptions() filling dialog" << endl;
#endif
    const QString printCenteredStr = options [OptionPrintImageCentered];
    const bool printCentered = ::ShouldPrintImageCenteredOnPage (printCenteredStr);
#if DEBUG_KP_PRINT_DIALOG_PAGE
    kDebug () << "\tgot config: printCentered=" << printCentered << endl;
#endif
    if (printCentered)
        d->printCenteredRadio->setChecked (true);
    else
        d->printTopLeftRadio->setChecked (true);
}


#include <kpPrintDialogPage.moc>
