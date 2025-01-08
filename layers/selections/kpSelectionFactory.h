
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpSelectionFactory_H
#define kpSelectionFactory_H

#include "pixmapfx/kpPixmapFX.h"

class QDataStream;

class kpAbstractImageSelection;

class kpSelectionFactory
{
public:
    static kpAbstractImageSelection *FromStream(QDataStream &stream);
};

#endif // kpSelectionFactory_H
