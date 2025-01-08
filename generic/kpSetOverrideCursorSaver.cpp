
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "generic/kpSetOverrideCursorSaver.h"

#include <QApplication>

kpSetOverrideCursorSaver::kpSetOverrideCursorSaver(const QCursor &cursor)
{
    QApplication::setOverrideCursor(cursor);
}

kpSetOverrideCursorSaver::~kpSetOverrideCursorSaver()
{
    QApplication::restoreOverrideCursor();
}
