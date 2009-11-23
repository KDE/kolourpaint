
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

#include <cstdlib>
#include <cmath>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

#include <KApplication>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>
#include <kpTool.h>


#if DEBUG_KP_PIXMAP_FX
    #define KP_PRINTF if (1) printf
#else
    #define KP_PRINTF if (0) (void)
#endif


#ifdef Q_WS_X11
// Same as QPixmap::defaultDepth(), but returns a meaningful answer when
// called before QApplication has been constructed, rather than always 32.
// After QApplication has been constructed, you must use QPixmap::defaultDepth()
// instead.
//
// Returns 0 if it encounters an error.
//
// Internally, this method forks the process.  In the child, a KApplication
// is constructed, QPixmap::defaultDepth() is called and the result is sent to
// the parent.  The child is then killed.
static int QPixmapCalculateDefaultDepthWithoutQApplication ()
{
    KP_PRINTF (("QPixmapCalculateDefaultDepthWithoutQApplication()\n"));

    // [0] = read
    // [1] = write
    enum
    {
        Read = 0, Write = 1
    };
    int fds [2];
    if (pipe (fds) != 0)
    {
        perror ("pipe");
        return 0;
    }
    
    pid_t pid = fork ();
    if (pid == -1)
    {
        perror ("fork");
        close (fds [Read]);
        close (fds [Write]);
        return 0;
    }

    // In child?
    if (pid == 0)
    {
        KP_PRINTF ("Child: created\n");
        close (fds [Read]);

        // use separate scope to force calling app destructor before exit()
        {
            KApplication app;

            const int depth = QPixmap::defaultDepth ();
            KP_PRINTF (("Child: writing default depth\n"));
            write (fds [Write], &depth, sizeof (depth));
            KP_PRINTF (("Child: wrote default depth\n"));

            close (fds [Write]);
            KP_PRINTF ("Child: exit\n");
        }
        exit (0);
    }
    // In parent?
    else
    {
        KP_PRINTF ("Parent: in here\n");
        close (fds [Write]);

        int depth = 0;
        KP_PRINTF ("Parent: reading default depth\n");
        read (fds [Read], &depth, sizeof (depth));
        KP_PRINTF ("Parent: read default depth %d\n", depth);

        close (fds [Read]);

        // Kill zombie child.
        KP_PRINTF ("Parent: waiting for child\n");
        int status;
        (void) waitpid (pid, &status, 0/*options*/);

        KP_PRINTF ("Parent: complete\n");
        return depth;
    }
}
#endif


// public static
// (KApplication has not been constructed yet)
void kpPixmapFX::initMaskOpsPre ()
{
#ifdef Q_WS_X11
    const int defaultDepth = QPixmapCalculateDefaultDepthWithoutQApplication ();

    // This is important for diagnosing KolourPaint bugs so always print it
    // -- even in release mode.
    printf ("Starting KolourPaint on a %d-bit screen...\n", defaultDepth);

    if (defaultDepth == 32)
    {
        KP_PRINTF ("\tCannot handle alpha channel - disabling XRENDER\n");

        // SYNC: Might break with Qt upgrades.
        setenv ("QT_X11_NO_XRENDER", "1", 1/*overwrite value*/);
    }
#else
#ifdef __GNUC__
    #warning "KolourPaint is heavily dependent on the behavior of QPixmap under X11."
    #warning "Until KolourPaint gets a proper image library, it is unlikely to work under non-X11."
#endif
#endif
}

// public static
void kpPixmapFX::initMaskOpsPost ()
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::initMaskOpsPost():"
              << "QPixmap().depth()=" << QPixmap ().depth ()
              << "QPixmap::defaultDepth()=" << QPixmap::defaultDepth ();
#endif

    // Check KolourPaint invariant.
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (QPixmap ());
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (QPixmap (1, 1));
    Q_ASSERT (QPixmap (1, 1).depth () == QPixmap::defaultDepth ());

    // initMaskOpsPre() should have ensured this (if it was 32 previously, it
    // should now be 24 due to the disabling of XRENDER).
    Q_ASSERT (QPixmap::defaultDepth () < 32);
}


// public static
bool kpPixmapFX::hasMask (const QPixmap &pixmap)
{
    return pixmap.hasAlpha ();
}

// public static
bool kpPixmapFX::hasAlphaChannel (const QPixmap &pixmap)
{
    return pixmap.hasAlphaChannel ();
}

// public static
bool kpPixmapFX::checkNoAlphaChannelInternal (const QPixmap &pixmap)
{
    if (!kpPixmapFX::hasAlphaChannel (pixmap))
        return true;

#if 1
    kError () << "Pixmap has alpha channel.  See the .h doc for"
                 " kpPixmapFX::ensureNoAlphaChannel() to fix this.";
    // Ignore bug rather than crashing the program because I bet developers
    // will inadvertently trigger this, when changing the code.  The bug has
    // very annoying effects under XRENDER but only causes serious failure
    // without XRENDER (which is not so common).
    return true;
#else
    // Assert-crash the program.
    return false;
#endif
}


// public static
void kpPixmapFX::ensureNoAlphaChannel (QPixmap *destPixmapPtr)
{
    Q_ASSERT (destPixmapPtr);

    if (kpPixmapFX::hasAlphaChannel (*destPixmapPtr))
    {
        // We need to change <destPixmapPtr> from 32-bit (RGBA
        // i.e. hasAlphaChannel() returns true -- regardless of whether the
        // channel has any actual content -- which causes trouble) to 24-bit
        // (RGB with possible mask).
        //
        // "destPixmapPtr->setMask (destPixmapPtr->mask())" is not
        //  sufficient so do it a long way:

        QPixmap oldPixmap = *destPixmapPtr;

        QBitmap oldMask = oldPixmap.mask ();
        // Kill RGB source mask in case it causes alpha channel / composition
        // operations in the following copy.
        oldPixmap.setMask (QBitmap ());

        // Copy RGB layer.
        *destPixmapPtr = QPixmap (oldPixmap.width (), oldPixmap.height ());
        QPainter p (destPixmapPtr);
        p.drawPixmap (QPoint (0, 0), oldPixmap);
        p.end ();

        // Copy mask layer (if any).
        destPixmapPtr->setMask (oldMask);
    }

    // Note that we don't check this on function entry as the purpose of
    // this function is to force the pixmap to satisfy this invariant.
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}


// public static
QBitmap kpPixmapFX::getNonNullMask (const QPixmap &pm)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);

    // (a bit slow so we cache it)
    const QBitmap mask = pm.mask ();

    if (!mask.isNull ())
        return mask;
    else
    {
        QBitmap maskBitmap (pm.width (), pm.height ());
        maskBitmap.fill (Qt::color1/*opaque*/);

        return maskBitmap;
    }
}


// public static
void kpPixmapFX::ensureTransparentAt (QPixmap *destPixmapPtr, const QRect &destRect)
{
    if (!destPixmapPtr)
        return;

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    QBitmap maskBitmap = getNonNullMask (*destPixmapPtr);

    QPainter p (&maskBitmap);
    p.fillRect (destRect, Qt::color0/*transparent*/);
    p.end ();

    destPixmapPtr->setMask (maskBitmap);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}


// public static
void kpPixmapFX::paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, const QPoint &destAt,
                                                const QPixmap &brushBitmap)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::paintMaskTransparentWithBrush(destAt="
              << destAt << ") brushRect=" << brushBitmap.rect ()
              << endl;
#endif

    Q_ASSERT (destPixmapPtr);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    Q_ASSERT (brushBitmap.depth () == 1);

    const QRegion brushRegion = QRegion (brushBitmap).translated (destAt);

    // OPT: Hopelessly inefficent due to function call overhead and
    //      fillRect() changing the mask every single iteration.
    //
    //      kpPixmapFX should have a function that does this with only a
    //      single mask write.
    foreach (const QRect &r, brushRegion.rects ())
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kDebug () << "\tcopy rect=" << r;
    #endif
        kpPixmapFX::fillRect (destPixmapPtr,
            r.x (), r.y (), r.width (), r.height (),
            kpColor::Transparent);
    }

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}

// public static
void kpPixmapFX::paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, int destX, int destY,
                                                const QPixmap &brushBitmap)
{
    kpPixmapFX::paintMaskTransparentWithBrush (destPixmapPtr,
                                               QPoint (destX, destY),
                                               brushBitmap);
}


// public static
void kpPixmapFX::ensureOpaqueAt (QPixmap *destPixmapPtr, const QRect &destRect)
{
    if (!destPixmapPtr)
        return;

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    // (a bit slow so we cache it)
    QBitmap maskBitmap = destPixmapPtr->mask ();

    // Already opaque?
    if (maskBitmap.isNull ())
        return;

    QPainter p (&maskBitmap);
    p.fillRect (destRect, Qt::color1/*opaque*/);
    p.end ();

    destPixmapPtr->setMask (maskBitmap);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}



