
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpPrintDialogPage_H
#define kpPrintDialogPage_H

#include <QWidget>

class kpPrintDialogPage : public QWidget
{
    Q_OBJECT

public:
    explicit kpPrintDialogPage(QWidget *parent);
    ~kpPrintDialogPage() override;

    bool printImageCenteredOnPage();
    void setPrintImageCenteredOnPage(bool printCentered);

private:
    struct kpPrintDialogPagePrivate *const d;
};

#endif // kpPrintDialogPage_H
