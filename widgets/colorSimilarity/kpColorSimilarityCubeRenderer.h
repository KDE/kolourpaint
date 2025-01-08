
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpColorSimilarityCubeRenderer_H
#define kpColorSimilarityCubeRenderer_H

class QPaintDevice;

class kpColorSimilarityCubeRenderer
{
public:
    // <highlight> is used for animations:
    //
    //  0 = no highlight
    //  .
    //  .
    //  .
    //  255 = full highlight

    static void Paint(QPaintDevice *target, int x, int y, int size, double colorSimilarity, int highlight = 0);
};

#endif // kpColorSimilarityCubeRenderer_H
