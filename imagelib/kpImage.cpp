
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


#include <kpImage.h>

#include <kpPixmapFX.h>


kpImage::kpImage ()
    : QPixmap ()
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*this);
}

kpImage::kpImage (int width, int height)
    : QPixmap (width, height)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*this);
}

kpImage::kpImage (const kpImage &image)
    : QPixmap (image)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (image);
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*this);
}

kpImage::kpImage (const QPixmap &pixmap)
    : QPixmap (pixmap)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pixmap);
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*this);
}


// public static
kpImage *kpImage::CastPixmapPtr (QPixmap *pixmap)
{
    return static_cast <kpImage *> (pixmap);
}

// public static
const kpImage *kpImage::CastPixmapPtr (const QPixmap *pixmap)
{
    return static_cast <const kpImage *> (pixmap);
}
