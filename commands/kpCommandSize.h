
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpCommandSize_H
#define kpCommandSize_H

#include "imagelib/kpImage.h"

class QImage;
class QPolygon;
class QString;

class kpAbstractSelection;

//
// Estimates the size of the object being pointed to, in bytes.
//
// This is used by the command history to trim stored commands, once a
// certain amount of memory is used by those commands.
//
class kpCommandSize
{
public:
    // Force 64-bit arithmetic, instead of 32-bit, to prevent overflow
    // when determining whether to clip the command history -- we might be
    // adding a large number of large sizes.  This will eventually help
    // KolourPaint support more than 2GB of image data.
    //
    // For some reason, GCC doesn't warn of accidental casts to smaller types
    // (e.g. 32-bit).  An easy way to get around this is to change "SizeType"
    // to be "double" temporarily and recompile - every time an implicit cast to
    // "int" (32-bit) is made, we'll be warned.
    //
    // TODO: Exhaustively test that we're not accidentally doing intermediate
    //       calculations using 32-bit in some places (mainly inside
    //       implementations of kpCommand::size()).
    typedef qlonglong SizeType;

    static SizeType PixmapSize(const QImage &image);
    static SizeType PixmapSize(const QImage *image);
    static SizeType PixmapSize(int width, int height, int depth);

    static SizeType QImageSize(const QImage &image);
    static SizeType QImageSize(const QImage *image);
    static SizeType QImageSize(int width, int height, int depth);

    static SizeType ImageSize(const kpImage &image);
    static SizeType ImageSize(const kpImage *image);

    static SizeType SelectionSize(const kpAbstractSelection &sel);
    static SizeType SelectionSize(const kpAbstractSelection *sel);

    static SizeType StringSize(const QString &string);

    static SizeType PolygonSize(const QPolygon &points);
};

#endif // kpCommandSize_H
