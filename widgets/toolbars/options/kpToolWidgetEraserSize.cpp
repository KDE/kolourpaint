
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


#define DEBUG_KP_TOOL_WIDGET_ERASER_SIZE 0


#include <kpToolWidgetEraserSize.h>

#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpColor.h>
#include <kpPainter.h>
#include <kpTool.h>


static int EraserSizes [] = {2, 3, 5, 9, 17, 29};
static const int NumEraserSizes =
    int (sizeof (::EraserSizes) / sizeof (::EraserSizes [0]));


static void DrawPixmap (QPixmap *destPixmap, const QPoint &topLeft, void *userData)
{
    kpToolWidgetEraserSize::DrawPackage *pack =
        static_cast <kpToolWidgetEraserSize::DrawPackage *> (userData);

    const int size = ::EraserSizes [pack->selected];

    kpPainter::fillRect (destPixmap,
        topLeft.x (), topLeft.y (), size, size,
        pack->color);
}

static void DrawCursor (QPixmap *destPixmap, const QPoint &topLeft, void *userData)
{
    ::DrawPixmap (destPixmap, topLeft, userData);


    kpToolWidgetEraserSize::DrawPackage *pack =
        static_cast <kpToolWidgetEraserSize::DrawPackage *> (userData);

    const int size = ::EraserSizes [pack->selected];
    
    // Would 1-pixel border on all sides completely cover the color of the
    // eraser?
    if (size <= 2)
        return;

    // Draw 1-pixel border on all sides.
    kpPainter::drawRect (destPixmap,
        topLeft.x (), topLeft.y (), size, size,
        kpColor::Black);
}


kpToolWidgetEraserSize::kpToolWidgetEraserSize (QWidget *parent, const QString &name)
    : kpToolWidgetBase (parent, name)
{
    setInvertSelectedPixmap ();

    for (int i = 0; i < ::NumEraserSizes; i++)
    {
        if (i == 3 || i == 5)
            startNewOptionRow ();

        const int s = ::EraserSizes [i];


        QPixmap previewPixmap (s, s);
        if (i < 3)
        {
            // HACK: kpToolWidgetBase's layout code sucks and gives uneven spacing
            previewPixmap = QPixmap ((width () - 4) / 3, 9);
            Q_ASSERT (previewPixmap.width () >= s &&
                previewPixmap.height () >= s);
        }

        kpPainter::fillRect (&previewPixmap,
            0, 0, previewPixmap.width (), previewPixmap.height (),
            kpColor::Transparent);
            
        DrawPackage pack = drawFunctionDataForSelected (kpColor::Black, i);
        ::DrawPixmap (&previewPixmap,
            QPoint ((previewPixmap.width () - s) / 2,
                    (previewPixmap.height () - s) / 2),
            &pack);


        addOption (previewPixmap, i18n ("%1x%2", s, s)/*tooltip*/);
    }

    finishConstruction (1, 0);
}

kpToolWidgetEraserSize::~kpToolWidgetEraserSize ()
{
}


// public
int kpToolWidgetEraserSize::eraserSize () const
{
    return ::EraserSizes [selected ()];
}


// public
kpTempImage::UserFunctionType kpToolWidgetEraserSize::drawFunction () const
{
    return &::DrawPixmap;
}

// public
kpTempImage::UserFunctionType kpToolWidgetEraserSize::drawCursorFunction () const
{
    return &::DrawCursor;
}


// public static
kpToolWidgetEraserSize::DrawPackage kpToolWidgetEraserSize::drawFunctionDataForSelected (
        const kpColor &color, int selectedIndex)
{
    DrawPackage pack;
    
    pack.selected = selectedIndex;
    pack.color = color;

    return pack;
}

// public
kpToolWidgetEraserSize::DrawPackage kpToolWidgetEraserSize::drawFunctionData (
        const kpColor &color) const
{
    return drawFunctionDataForSelected (color, selected ());
}

    
// protected slot virtual [base kpToolWidgetBase]
bool kpToolWidgetEraserSize::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret)
        emit eraserSizeChanged (eraserSize ());
    return ret;
}


#include <kpToolWidgetEraserSize.moc>