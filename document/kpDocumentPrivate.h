
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpDocumentPrivate_H
#define kpDocumentPrivate_H

class kpDocumentEnvironment;

struct kpDocumentPrivate {
    kpDocumentPrivate()
        : environ(nullptr)
    {
    }

    kpDocumentEnvironment *environ;
};

#endif // kpDocumentPrivate_H
