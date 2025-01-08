
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_CURSOR_LIGHT_CROSS 0

#include "kpCursorLightCross.h"

#include "kpLogCategories.h"

#include <QBitmap>
#include <QCursor>

enum PixelValue {
    White,
    Black,
    Transparent
};

static void setPixel(unsigned char *colorBitmap, unsigned char *maskBitmap, int width, int y, int x, enum PixelValue pv)
{
    const int ColorBlack = 1;
    const int ColorWhite = 0;

    const int MaskOpaque = 1;
    const int MaskTransparent = 0;

    int colorValue, maskValue;

    switch (pv) {
    case White:
        colorValue = ColorWhite;
        maskValue = MaskOpaque;
        break;

    case Black:
        colorValue = ColorBlack;
        maskValue = MaskOpaque;
        break;

    case Transparent:
        colorValue = ColorWhite;
        maskValue = MaskTransparent;
        break;
    }

    if (colorValue) {
        colorBitmap[y * (width / 8) + (x / 8)] |= (1 << (x % 8));
    }

    if (maskValue) {
        maskBitmap[y * (width / 8) + (x / 8)] |= (1 << (x % 8));
    }
}

const QCursor *kpCursorLightCrossCreate()
{
#if DEBUG_KP_CURSOR_LIGHT_CROSS
    qCDebug(kpLogMisc) << "kpCursorLightCrossCreate() ";
#endif

    const int side = 24;
    const int byteSize = (side * side) / 8;
    auto *colorBitmap = new unsigned char[byteSize];
    auto *maskBitmap = new unsigned char[byteSize];

    memset(colorBitmap, 0, byteSize);
    memset(maskBitmap, 0, byteSize);

    const int oddSide = side - 1;
    const int strokeLen = oddSide * 3 / 8;

    for (int i = 0; i < strokeLen; i++) {
        const enum PixelValue pv = (i % 2) ? Black : White;

#define X_(val) (val)
#define Y_(val) (val)
#define DRAW(y, x) setPixel(colorBitmap, maskBitmap, side, (y), (x), pv)
        // horizontal
        DRAW(Y_(side / 2), X_(1 + i));
        DRAW(Y_(side / 2), X_(side - 1 - i));

        // vertical
        DRAW(Y_(1 + i), X_(side / 2));
        DRAW(Y_(side - 1 - i), X_(side / 2));
#undef DRAW
#undef Y_
#undef X_
    }

    const QSize size(side, side);
    QCursor *cursor = new QCursor(QBitmap::fromData(size, colorBitmap, QImage::Format_MonoLSB), QBitmap::fromData(size, maskBitmap, QImage::Format_MonoLSB));

    delete[] maskBitmap;
    delete[] colorBitmap;

    return cursor;
}
