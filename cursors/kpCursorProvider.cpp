
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpCursorProvider.h"

#include "kpCursorLightCross.h"

#include <QCursor>

static const QCursor *TheLightCursor = nullptr;

// public static
QCursor kpCursorProvider::lightCross()
{
    // TODO: don't leak (although it's cleaned up on exit by OS anyway)
    if (!::TheLightCursor) {
        ::TheLightCursor = kpCursorLightCrossCreate();
    }

    return *::TheLightCursor;
}
