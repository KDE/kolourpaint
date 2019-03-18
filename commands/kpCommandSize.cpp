
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DEBUG_KP_COMMAND_SIZE 0


#include "commands/kpCommandSize.h"
#include "layers/selections/kpAbstractSelection.h"

#include <QImage>
#include <QPolygon>
#include <QString>


// public static
kpCommandSize::SizeType kpCommandSize::PixmapSize (const QImage &image)
{
    return kpCommandSize::PixmapSize (image.width (), image.height (), image.depth ());
}

// public static
kpCommandSize::SizeType kpCommandSize::PixmapSize (const QImage *image)
{
    return (image ? kpCommandSize::PixmapSize (*image) : 0);
}

// public static
kpCommandSize::SizeType kpCommandSize::PixmapSize (int width, int height, int depth)
{
    // handle 15bpp
    int roundedDepth = (depth > 8 ? (depth + 7) / 8 * 8 : depth);
    kpCommandSize::SizeType ret =
            static_cast<kpCommandSize::SizeType> (width) * height * roundedDepth / 8;

#if DEBUG_KP_COMMAND_SIZE && 0
    qCDebug(kpLogCommands) << "kpCommandSize::PixmapSize() w=" << width
               << " h=" << height
               << " d=" << depth
               << " roundedDepth=" << roundedDepth
               << " ret=" << ret;
#endif
    return ret;
}


// public static
kpCommandSize::SizeType kpCommandSize::QImageSize (const QImage &image)
{
    return kpCommandSize::QImageSize (image.width (), image.height (), image.depth ());
}

// public static
kpCommandSize::SizeType kpCommandSize::QImageSize (const QImage *image)
{
    return (image ? kpCommandSize::QImageSize (*image) : 0);
}

// public static
kpCommandSize::SizeType kpCommandSize::QImageSize (int width, int height, int depth)
{
    // handle 15bpp
    int roundedDepth = (depth > 8 ? (depth + 7) / 8 * 8 : depth);
    kpCommandSize::SizeType ret =
        static_cast<kpCommandSize::SizeType> (width) * height * roundedDepth / 8;

#if DEBUG_KP_COMMAND_SIZE && 0
    qCDebug(kpLogCommands) << "kpCommandSize::QImageSize() w=" << width
               << " h=" << height
               << " d=" << depth
               << " roundedDepth=" << roundedDepth
               << " ret=" << ret;
#endif

    return ret;
}


// public static
kpCommandSize::SizeType kpCommandSize::ImageSize (const kpImage &image)
{
    return kpCommandSize::PixmapSize (image);
}

// public static
kpCommandSize::SizeType kpCommandSize::ImageSize (const kpImage *image)
{
    return kpCommandSize::PixmapSize (image);
}


// public static
kpCommandSize::SizeType kpCommandSize::SelectionSize (const kpAbstractSelection &sel)
{
    return sel.size ();
}

// public static
kpCommandSize::SizeType kpCommandSize::SelectionSize (const kpAbstractSelection *sel)
{
    return (sel ? sel->size () : 0);
}


// public static
kpCommandSize::SizeType kpCommandSize::StringSize (const QString &string)
{
#if DEBUG_KP_COMMAND_SIZE && 1
    qCDebug(kpLogCommands) << "kpCommandSize::StringSize(" << string << ")"
               << " len=" << string.length ()
               << " sizeof(QChar)=" << sizeof (QChar);
#endif
    return static_cast<SizeType> (static_cast<unsigned int> (string.length ()) * sizeof (QChar));
}


// public static
kpCommandSize::SizeType kpCommandSize::PolygonSize (const QPolygon &points)
{
#if DEBUG_KP_COMMAND_SIZE && 1
    qCDebug(kpLogCommands) << "kpCommandSize::PolygonSize() points.size="
               << points.size ()
               << " sizeof(QPoint)=" << sizeof (QPoint);
#endif

    return static_cast<SizeType> (static_cast<unsigned int> (points.size ()) * sizeof (QPoint));
}

