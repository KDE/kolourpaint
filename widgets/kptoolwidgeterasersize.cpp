
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kptoolwidgeterasersize.h>

#include <qbitmap.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kptool.h>


static int eraserSizes [] = {2, 3, 5, 9, 17, 29};
static const int numEraserSizes = int (sizeof (eraserSizes) / sizeof (eraserSizes [0]));


kpToolWidgetEraserSize::kpToolWidgetEraserSize (QWidget *parent, const char *name)
    : kpToolWidgetBase (parent, name)
{
    setInvertSelectedPixmap ();

    m_cursorPixmaps = new QPixmap [numEraserSizes];
    QPixmap *cursorPixmap = m_cursorPixmaps;

    for (int i = 0; i < numEraserSizes; i++)
    {
        if (i == 3 || i == 5)
            startNewOptionRow ();

        int s = eraserSizes [i];

        cursorPixmap->resize (s, s);
        cursorPixmap->fill (Qt::black);


        QPixmap previewPixmap (s, s);
        if (i < 3)
        {
            // HACK: kpToolWidgetBase's layout code sucks and gives uneven spacing
            previewPixmap.resize ((width () - 4) / 3, 9);
        }

        QPainter painter (&previewPixmap);
        QRect rect ((previewPixmap.width () - s) / 2, (previewPixmap.height () - s) / 2, s, s);
        painter.fillRect (rect, Qt::black);
        painter.end ();

        QBitmap mask (previewPixmap.width (), previewPixmap.height ());
        mask.fill (Qt::color0/*transparent*/);

        QPainter maskPainter (&mask);
        maskPainter.fillRect (rect, Qt::color1/*opaque*/);
        maskPainter.end ();

        previewPixmap.setMask (mask);


        addOption (previewPixmap, i18n ("%1x%2").arg (s).arg (s)/*tooltip*/);


        cursorPixmap++;
    }

    finishConstruction (1, 0);
}

kpToolWidgetEraserSize::~kpToolWidgetEraserSize ()
{
    delete [] m_cursorPixmaps;
}

int kpToolWidgetEraserSize::eraserSize () const
{
    return eraserSizes [selected ()];
}

QPixmap kpToolWidgetEraserSize::cursorPixmap (const kpColor &color) const
{
#if DEBUG_KP_TOOL_WIDGET_ERASER_SIZE
    kdDebug () << "kpToolWidgetEraseSize::cursorPixmap() selected=" << selected ()
               << " numEraserSizes=" << numEraserSizes
               << endl;
#endif

    // TODO: why are we even storing m_cursorPixmaps?
    QPixmap pixmap = m_cursorPixmaps [selected ()];
    if (color.isOpaque ())
        pixmap.fill (color.toQColor ());


    bool showBorder = (pixmap.width () > 2 && pixmap.height () > 2);

    if (showBorder)
    {
        QPainter painter (&pixmap);
        painter.setPen (Qt::black);
        painter.drawRect (pixmap.rect ());
    }


    if (color.isTransparent ())
    {
        QBitmap maskBitmap (pixmap.width (), pixmap.height ());
        maskBitmap.fill (Qt::color0/*transparent*/);


        if (showBorder)
        {
            QPainter maskBitmapPainter (&maskBitmap);
            maskBitmapPainter.setPen (Qt::color1/*opaque*/);
            maskBitmapPainter.drawRect (maskBitmap.rect ());
        }


        pixmap.setMask (maskBitmap);
    }


    return pixmap;
}

// virtual protected slot [base kpToolWidgetBase]
bool kpToolWidgetEraserSize::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret)
        emit eraserSizeChanged (eraserSize ());
    return ret;
}

#include <kptoolwidgeterasersize.moc>
