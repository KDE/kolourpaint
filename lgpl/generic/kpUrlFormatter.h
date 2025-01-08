
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

//
// This class is in the lpgl/ folder because other code in the folder needs
// to access it.  But it is not under the LPGL.
//

#ifndef kpUrlFormatter_H
#define kpUrlFormatter_H

#include <kolourpaint_lgpl_export.h>

class QString;

class QUrl;

class KOLOURPAINT_LGPL_EXPORT kpUrlFormatter
{
public:
    // (will convert: empty Url --> "Untitled")
    static QString PrettyUrl(const QUrl &url);

    // (will convert: empty Url --> "Untitled")
    static QString PrettyFilename(const QUrl &url);
};

#endif // kpUrlFormatter_H
