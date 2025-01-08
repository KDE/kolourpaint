
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_ROUNDED_RECTANGLE_H
#define KP_TOOL_ROUNDED_RECTANGLE_H

#include "tools/rectangular/kpToolRectangularBase.h"

class kpToolRoundedRectangle : public kpToolRectangularBase
{
    Q_OBJECT

public:
    kpToolRoundedRectangle(kpToolEnvironment *environ, QObject *parent);

    static void drawRoundedRect(kpImage *image, int x, int y, int width, int height, const kpColor &fcolor, int penWidth, const kpColor &bcolor);
};

#endif // KP_TOOL_ROUNDED_RECTANGLE_H
