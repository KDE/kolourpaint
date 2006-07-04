
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_COLOR_WASHER 0


#include <kptoolcolorwasher.h>

#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>

#include <kapplication.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kptoolflowcommand.h>


kpToolColorWasher::kpToolColorWasher (kpMainWindow *mainWindow)
    : kpToolFlowBase (i18n ("Color Eraser"),
        i18n ("Replaces pixels of the foreground color with the background color"),
        Qt::Key_O,
        mainWindow,
        "tool_color_washer")
{
}

kpToolColorWasher::~kpToolColorWasher ()
{
}


// public virtual [base kpTool]
void kpToolColorWasher::globalDraw ()
{
#if DEBUG_KP_TOOL_COLOR_WASHER
    kDebug () << "kpToolColorWasher::globalDraw()" << endl;
#endif
    if (foregroundColor () == backgroundColor () && processedColorSimilarity () == 0)
        return;

    QApplication::setOverrideCursor (Qt::WaitCursor);

    kpToolFlowCommand *cmd = new kpToolFlowCommand (
        i18n ("Color Eraser"), mainWindow ());

    QPainter painter, maskPainter;
    QBitmap maskBitmap;

    if (backgroundColor ().isOpaque ())
    {
        painter.begin (document ()->pixmap ());
        painter.setPen (backgroundColor ().toQColor ());
    }

    if (backgroundColor ().isTransparent () ||
        document ()->pixmap ()->mask ())
    {
        maskBitmap = kpPixmapFX::getNonNullMask (*document ()->pixmap ());
        maskPainter.begin (&maskBitmap);

        maskPainter.setPen (backgroundColor ().maskColor ());
    }

    const QImage image = kpPixmapFX::convertToImage (*document ()->pixmap ());
    QRect rect = document ()->rect ();

    const bool didSomething = wash (&painter, &maskPainter, image,
                                    foregroundColor ()/*replace foreground*/,
                                    rect, rect);

    // flush
    if (painter.isActive ())
        painter.end ();

    if (maskPainter.isActive ())
        maskPainter.end ();

    if (didSomething)
    {
        if (!maskBitmap.isNull ())
            document ()->pixmap ()->setMask (maskBitmap);


        document ()->slotContentsChanged (rect);


        cmd->updateBoundingRect (rect);
        cmd->finalize ();

        commandHistory ()->addCommand (cmd, false /* don't exec */);

        // don't delete - it's up to the commandHistory
        cmd = 0;
    }
    else
    {
    #if DEBUG_KP_TOOL_COLOR_WASHER
        kDebug () << "\tisNOP" << endl;
    #endif
        delete cmd;
        cmd = 0;
    }

    QApplication::restoreOverrideCursor ();
}

QString kpToolColorWasher::haventBegunDrawUserMessage () const
{
    return i18n ("Click or drag to erase pixels of the foreground color.");
}



bool kpToolColorWasher::wash (QPainter *painter, QPainter *maskPainter,
                      const QImage &image,
                      const kpColor &colorToReplace,
                      const QRect &imageRect, int plotx, int ploty)
{
    return wash (painter, maskPainter, image, colorToReplace, imageRect, hotRect (plotx, ploty));
}

bool kpToolColorWasher::wash (QPainter *painter, QPainter *maskPainter,
                      const QImage &image,
                      const kpColor &colorToReplace,
                      const QRect &imageRect, const QRect &drawRect)
{
    bool didSomething = false;

#if DEBUG_KP_TOOL_COLOR_WASHER && 0
    kDebug () << "kpToolColorWasher::wash(imageRect=" << imageRect
               << ",drawRect=" << drawRect
               << ")" << endl;
#endif

// make use of scanline coherence
#define FLUSH_LINE()                                     \
{                                                        \
    if (painter && painter->isActive ())                 \
        painter->drawLine (startDrawX, y, x - 1, y);     \
    if (maskPainter && maskPainter->isActive ())         \
        maskPainter->drawLine (startDrawX, y, x - 1, y); \
    didSomething = true;                                 \
    startDrawX = -1;                                     \
}

    const int maxY = drawRect.bottom () - imageRect.top ();

    const int minX = drawRect.left () - imageRect.left ();
    const int maxX = drawRect.right () - imageRect.left ();

    for (int y = drawRect.top () - imageRect.top ();
         y <= maxY;
         y++)
    {
        int startDrawX = -1;

        int x;  // for FLUSH_LINE()
        for (x = minX; x <= maxX; x++)
        {
        #if DEBUG_KP_TOOL_COLOR_WASHER && 0
            fprintf (stderr, "y=%i x=%i colorAtPixel=%08X colorToReplace=%08X ... ",
                     y, x,
                     kpPixmapFX::getColorAtPixel (image, QPoint (x, y)).toQRgb (),
                     colorToReplace.toQRgb ());
        #endif
            if (kpPixmapFX::getColorAtPixel (image, QPoint (x, y)).isSimilarTo (colorToReplace, processedColorSimilarity ()))
            {
            #if DEBUG_KP_TOOL_COLOR_WASHER && 0
                fprintf (stderr, "similar\n");
            #endif
                if (startDrawX < 0)
                    startDrawX = x;
            }
            else
            {
            #if DEBUG_KP_TOOL_COLOR_WASHER && 0
                fprintf (stderr, "different\n");
            #endif
                if (startDrawX >= 0)
                    FLUSH_LINE ();
            }
        }

        if (startDrawX >= 0)
            FLUSH_LINE ();
    }

#undef FLUSH_LINE

    return didSomething;
}



bool kpToolColorWasher::drawShouldProceed (const QPoint & /*thisPoint*/,
    const QPoint & /*lastPoint*/,
    const QRect & /*normalizedRect*/)
{
    if (foregroundColor () == backgroundColor () &&
        processedColorSimilarity () == 0)
    {
        return false;
    }
    
    return true;
}


void kpToolColorWasher::drawPoint (const QPoint &)
{            
#if DEBUG_KP_TOOL_COLOR_WASHER
    kDebug () << "Washing pixmap (immediate)" << endl;
    QTime timer;
#endif
    QRect rect = hotRect ();
#if DEBUG_KP_TOOL_COLOR_WASHER
    timer.start ();
#endif
    QPixmap pixmap = document ()->getPixmapAt (rect);
#if DEBUG_KP_TOOL_COLOR_WASHER
    kDebug () << "\tget from doc: " << timer.restart () << "ms" << endl;
#endif
    const QImage image = kpPixmapFX::convertToImage (pixmap);
#if DEBUG_KP_TOOL_COLOR_WASHER
    kDebug () << "\tconvert to image: " << timer.restart () << "ms" << endl;
#endif
    QPainter painter, maskPainter;
    QBitmap maskBitmap;

    if (color (m_mouseButton).isOpaque ())
    {
        painter.begin (&pixmap);
        painter.setPen (color (m_mouseButton).toQColor ());
    }

    if (color (m_mouseButton).isTransparent () ||
        !pixmap.mask ().isNull ())
    {
        maskBitmap = kpPixmapFX::getNonNullMask (pixmap);
        maskPainter.begin (&maskBitmap);
        maskPainter.setPen (color (m_mouseButton).maskColor ());
    }

    bool didSomething = wash (&painter, &maskPainter,
                                image,
                                color (1 - m_mouseButton)/*color to replace*/,
                                rect, rect);

    if (painter.isActive ())
        painter.end ();

    if (maskPainter.isActive ())
        maskPainter.end ();

    if (didSomething)
    {
        if (!maskBitmap.isNull ())
            pixmap.setMask (maskBitmap);

    #if DEBUG_KP_TOOL_COLOR_WASHER
        kDebug () << "\twashed: " << timer.restart () << "ms" << endl;
    #endif
        document ()->setPixmapAt (pixmap, hotPoint ());
    #if DEBUG_KP_TOOL_COLOR_WASHER
        kDebug () << "\tset doc: " << timer.restart () << "ms" << endl;
    #endif
        m_currentCommand->updateBoundingRect (hotRect ());
    #if DEBUG_KP_TOOL_COLOR_WASHER
        kDebug () << "\tupdate boundingRect: " << timer.restart () << "ms" << endl;
        kDebug () << "\tdone" << endl;
    #endif
    }

#if DEBUG_KP_TOOL_COLOR_WASHER && 1
    kDebug () << endl;
#endif
}

bool kpToolColorWasher::drawLine (QPixmap *pixmap, const QRect &docRect,
    const QPoint &thisPoint, const QPoint &lastPoint)
{
#if DEBUG_KP_TOOL_COLOR_WASHER
    kDebug () << "Washing pixmap (w=" << rect.width ()
                << ",h=" << rect.height () << ")" << endl;
    QTime timer;
    int convAndWashTime;
#endif

    QBitmap maskBitmap;    
    QPainter painter, maskPainter;

    drawLineSetupPainterMask (pixmap,
        &maskBitmap,
        &painter, &maskPainter);

        
    QImage image;
#if DEBUG_KP_TOOL_COLOR_WASHER
    timer.start ();
#endif
    image = kpPixmapFX::convertToImage (*pixmap);
#if DEBUG_KP_TOOL_COLOR_WASHER
    convAndWashTime = timer.restart ();
    kDebug () << "\tconvert to image: " << convAndWashTime << " ms" << endl;
#endif

    bool didSomething = false;

    kpColor colorToReplace = color (1 - m_mouseButton);

    QVector <QPoint> points = interpolatePoints (docRect,
        thisPoint, lastPoint);
    for (QVector <QPoint>::const_iterator pit = points.begin ();
            pit != points.end ();
            pit++)
    {
        const int XXX = (*pit).x (), YYY = (*pit).y ();
        
        if (wash (&painter, &maskPainter, image,
                    colorToReplace,
                    docRect, XXX + docRect.left (), YYY + docRect.top ()))
        {
            didSomething = true;
        }
    }


    
    drawLineTearDownPainterMask (pixmap,
        &maskBitmap,
        &painter, &maskPainter,
        didSomething);


#if DEBUG_KP_TOOL_COLOR_WASHER
    int ms = timer.restart ();
    kDebug () << "\ttried to wash: " << ms << "ms"
                << " (" << (ms ? (rect.width () * rect.height () / ms) : -1234)
                << " pixels/ms)"
                << endl;
    convAndWashTime += ms;
#endif

    return didSomething;
}

#include <kptoolcolorwasher.moc>
