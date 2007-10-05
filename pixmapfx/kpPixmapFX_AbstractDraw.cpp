
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


#define DEBUG_KP_PIXMAP_FX 0


#include <kpPixmapFX.h>

#include <math.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>
#include <kpTool.h>
#include <kconfiggroup.h>


// public static
QColor kpPixmapFX::draw_ToQColor (const kpColor &color, bool drawingOnRGBLayer)
{
    if (drawingOnRGBLayer)
    {
        if (color.isOpaque ())
            return color.toQColor ();
        else  // if (color.isTransparent())
        {
            // (arbitrary as image will be transparent here)
            return Qt::black;
        }
    }
    else
        return color.maskColor ();
}


struct DrawPack
{
    QPixmap *image;
    void (*userDrawFunc) (QPainter * /*p*/,
        bool /*drawingOnRGBLayer*/,
        void * /*data*/);
    void *userData;
};

static QRect DrawHelper (QPainter *rgbPainter, QPainter *maskPainter, void *data)
{
    DrawPack *pack = static_cast <DrawPack *> (data);

    if (rgbPainter)
        pack->userDrawFunc (rgbPainter, true/*drawing on RGB*/, pack->userData);

    if (maskPainter)
        pack->userDrawFunc (maskPainter, false/*drawing on mask*/, pack->userData);

    // Assume the whole image was clobbered.
    return QRect (0, 0, pack->image->width (), pack->image->height ());
}

// public static
void kpPixmapFX::draw (QPixmap *image,
        void (*drawFunc) (QPainter * /*p*/,
            bool /*drawingOnRGBLayer*/,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data)
{
    DrawPack pack;
    pack.image = image;
    pack.userDrawFunc = drawFunc;
    pack.userData = data;

    // Call below method.
    kpPixmapFX::draw (image,
        &::DrawHelper,
        anyColorOpaque, anyColorTransparent,
        &pack);
}

// public static
QRect kpPixmapFX::draw (QPixmap *image,
        QRect (*drawFunc) (QPainter * /*rgbPainter*/, QPainter * /*maskPainter*/,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kppixmapfx.cpp:Draw(image: rect=" << image->rect ()
              << ",drawFunc=" << drawFunc
              << ",anyColorOpaque=" << anyColorOpaque
              << ",anyColorTransparent=" << anyColorTransparent
              << ")" << endl;
#endif

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*image);

    // Interesting note (in case we support depth 1 later):
    // QBitmap's do not have masks but QBitmap::mask() returns itself.
    Q_ASSERT (image->depth () > 1);


    QBitmap mask = image->mask ();

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tDraw(): hasMask=" << !mask.isNull ();
#endif

    QPainter rgbPainter, maskPainter;

    // Draw on RGB layer?
    if (anyColorOpaque)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tDraw(): drawing on RGB";
    #endif
        // RGB draw is not allowed to touch mask.
        image->setMask (QBitmap ());

        rgbPainter.begin (image);
    }

    // Draw on mask layer?
    if (anyColorTransparent ||
        !mask.isNull ())
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tDraw(): drawing on transparent";
    #endif
        if (mask.isNull ())
            mask = kpPixmapFX::getNonNullMask (*image);

        maskPainter.begin (&mask);
    }


    if (!rgbPainter.isActive () && !maskPainter.isActive ())
    {
        // We did nothing.
        return QRect ();
    }


    const QRect dirtyRect = (*drawFunc) (
        rgbPainter.isActive () ? &rgbPainter : 0,
        maskPainter.isActive () ? &maskPainter : 0,
        data);


    if (rgbPainter.isActive ())
        rgbPainter.end ();

    if (maskPainter.isActive ())
        maskPainter.end ();

    if (anyColorOpaque)
    {
        // A mask should not have been created - that's the job of the next step.
        Q_ASSERT (!kpPixmapFX::hasMask (*image));
    }


#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tDraw(): setting mask " << !mask.isNull ();
#endif

    // Set new mask.
    image->setMask (mask);


    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*image);

    return dirtyRect;
}
