
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


#include <kpPixmapFX.h>

#include <qpainter.h>
#include <QImage>
#include <QWidget>
#include <qpolygon.h>
#include <qrect.h>

#include <kdebug.h>

#include <kpColor.h>

//---------------------------------------------------------------------

static void WidgetFillStippledRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &colorHint1, const kpColor &colorHint2)
{
    // LOREFACTOR: code dup with FillRectHelper() but hard to not dup
    
    QPainter p (widget);
    p.setClipRect (x, y, width, height);
    
    const int StippleSize = 4;

    for (int dy = 0; dy < height; dy += StippleSize)
    {
        for (int dx = 0; dx < width; dx += StippleSize)
        {
            const bool parity = ((dy + dx) / StippleSize) % 2;

            kpColor useColor;
            if (!parity)
                useColor = colorHint1;
            else
                useColor = colorHint2;

            p.fillRect (x + dx, y + dy, StippleSize, StippleSize,
                useColor.toQColor ());
        }
    }
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawStippledXORPolygon (QImage *image,
        const QPolygon &points,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2,
        bool isFinal)
{
    (void) fcolor1; (void) fcolor2;
    
    if (!isFinal)
    {
        kpPixmapFX::drawPolyline (image,
            points,
            colorHint1, 1/*pen width*/,
            colorHint2);
    }
    else
    {
        kpPixmapFX::drawPolygon (image,
            points,
            colorHint1, 1/*pen width*/,
            kpColor::Invalid/*no background*/,
            true/*is final*/,
            colorHint2);
    }
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawStippledXORRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2)
{
    (void) fcolor1; (void) fcolor2;
    kpPixmapFX::drawRect (image,
        x, y, width, height,
        colorHint1, 1/*pen width*/,
        kpColor::Invalid/*no background*/,
        colorHint2);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::widgetDrawStippledXORRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &colorHint1, const kpColor &colorHint2,
        const QRect &clipRect)
{
    (void) fcolor1; (void) fcolor2;

    QPainter p (widget);

    if (!clipRect.isEmpty ())
        p.setClipRect (clipRect);

    p.setPen (QPen (colorHint1.toQColor (), 1/*width*/, Qt::DotLine));
    p.setBackground (colorHint2.toQColor ());
    p.setBackgroundMode (Qt::OpaqueMode);

    // LOREFACTOR: code dup with DrawGenericRect() but hard to not dup
    if (width == 1 || height == 1)
    {
        p.drawLine (x, y, x + width - 1, y + height - 1);
        return;
    }

    // -1's compensate for Qt4's 1 pixel higher and wider
    // QPainter::drawRect().
    p.drawRect (x, y, width - 1, height - 1);
}

//---------------------------------------------------------------------


// public static
void kpPixmapFX::fillXORRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint1, const kpColor &colorHint2)
{
    (void) fcolor;
    kpPixmapFX::fillRect (image,
        x, y, width, height,
        colorHint1, colorHint2);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::widgetFillXORRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint1, const kpColor &colorHint2)
{
    (void) fcolor;
    ::WidgetFillStippledRect (widget,
        x, y, width, height,
        colorHint1, colorHint2);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawNOTRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &colorHint1, const kpColor &colorHint2)
{
    kpPixmapFX::drawRect (image,
        x, y, width, height,
        colorHint1, 1/*pen width*/,
        kpColor::Invalid/*no background*/,
        colorHint2);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::widgetFillNOTRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &colorHint1, const kpColor &colorHint2)
{
    ::WidgetFillStippledRect (widget,
        x, y, width, height,
        colorHint1, colorHint2);
}

//---------------------------------------------------------------------
