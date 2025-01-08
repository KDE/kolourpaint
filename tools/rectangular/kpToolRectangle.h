
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_RECTANGLE_H
#define KP_TOOL_RECTANGLE_H

#include "tools/rectangular/kpToolRectangularBase.h"

class kpToolRectangle : public kpToolRectangularBase
{
    Q_OBJECT

public:
    kpToolRectangle(kpToolEnvironment *environ, QObject *parent);

    static void drawRect(kpImage *image, int x, int y, int width, int height, const kpColor &fcolor, int penWidth, const kpColor &bcolor);
};

#endif // KP_TOOL_RECTANGLE_H
