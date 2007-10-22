
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


#ifndef KP_IMAGE_H
#define KP_IMAGE_H


#include <QPixmap>


//
// Bitmap abstraction - pixel data.
// Does not care about metadata (including DPI).
//
// Supposed to be independent of the screen and represents precious
// document data that should not be dithered down.  Should support 1-bit
// indexed, 8-bit indexed, 16-bit RGB(A) and 24-bit RGB(A) colour models
// at least.
//
// Currently, the reality is that its color model is the same as the
// screen mode (!) and it optionally contains a 0/1 transparency mask.  A
// KolourPaint invariant is that it will not contain an alpha channel
// -- it can either have a transparency mask or nothing at all.
//
// sync: kpAbstractImageSelection::sizeWithoutImage() depends on kpImage
//       using copy-on-write.
//
// REFACTOR: Make this into a class that doesn't expose the underlying
//           QPixmap except to methods in imagelib/
//
class kpImage : public QPixmap
{
public:
    //
    // These establish the invariant of no alpha channel.
    //
    // Use them to construct images that will be placed inside
    // kpDocument's.  Definitely use instead of QPixmap's constructor.
    //
    // If constructing a pixmap (i.e. displayed on screen but never
    // placed inside a kpDocument), use QPixmap() instead.
    //
    kpImage ();
    kpImage (int width, int height);

    kpImage (const kpImage &image);

    // ASSUMPTION: <pixmap> satisfies the kpImage invariant of no alpha
    //             channel.
    kpImage (const QPixmap &pixmap);


    //
    // REFACTOR: Yucky compatibility stuff that must go once kpImage != QPixmap.
    //
    // Returns a static cast of the given pointer.
    // Use this instead of static casts all over the source base, to prepare
    // for the day when casting kpImage to and from QPixmap is invalid.
    //
    // sync: As kpImage and QPixmap are still related, do not add data
    //       fields to kpImage or you'll break these methods.
    //
    static kpImage *CastPixmapPtr (QPixmap *pixmap);
    static const kpImage *CastPixmapPtr (const QPixmap *pixmap);
};


#endif  // KP_IMAGE_H
