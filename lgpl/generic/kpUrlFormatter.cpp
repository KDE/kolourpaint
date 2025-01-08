
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpUrlFormatter.h"

#include <KLocalizedString>
#include <QUrl>

//---------------------------------------------------------------------

// public static
QString kpUrlFormatter::PrettyUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        return i18n("Untitled");
    }

    return url.url(QUrl::PreferLocalFile);
}

//---------------------------------------------------------------------

// public static
QString kpUrlFormatter::PrettyFilename(const QUrl &url)
{
    if (url.isEmpty()) {
        return i18n("Untitled");
    }

    if (url.fileName().isEmpty()) {
        return kpUrlFormatter::PrettyUrl(url); // better than the name ""
    }

    return url.fileName();
}

//---------------------------------------------------------------------
