
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTextSelectionPrivate_H
#define kpTextSelectionPrivate_H

#include <QList>

#include "imagelib/kpImage.h"
#include "layers/selections/text/kpPreeditText.h"
#include "layers/selections/text/kpTextStyle.h"

struct kpTextSelectionPrivate {
    QList<QString> textLines;
    kpTextStyle textStyle;
    kpPreeditText preeditText;
};

#endif // kpTextSelectionPrivate_H
