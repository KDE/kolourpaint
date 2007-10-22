
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


#define DEBUG_KP_TOOL_WIDGET_FILL_STYLE 0


#include <kpToolWidgetFillStyle.h>

#include <qbitmap.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpColor.h>
#include <kpDefs.h>
#include <kpPixmapFX.h>
#include <kpTool.h>


kpToolWidgetFillStyle::kpToolWidgetFillStyle (QWidget *parent, const QString &name)
    : kpToolWidgetBase (parent, name)
{
    setInvertSelectedPixmap ();

    for (int i = 0; i < (int) FillStyleNum; i++)
    {
        QPixmap pixmap;

        pixmap = fillStylePixmap ((FillStyle) i,
                                  (width () - 2/*margin*/) * 3 / 4,
                                  (height () - 2/*margin*/ - 2/*spacing*/) * 3 / (3 * 4));
        addOption (pixmap, fillStyleName ((FillStyle) i)/*tooltip*/);

        startNewOptionRow ();
    }

    finishConstruction (0, 0);
}

kpToolWidgetFillStyle::~kpToolWidgetFillStyle ()
{
}


// private
QPixmap kpToolWidgetFillStyle::fillStylePixmap (FillStyle fs, int w, int h)
{
    QPixmap pixmap ((w <= 0 ? width () : w), (h <= 0 ? height () : h));
    pixmap.fill (Qt::white);


    const int penWidth = 2;

    // -1's compensate for Qt4's 1 pixel higher and wider
    // QPainter::drawRect().
    const QRect rectRect (QRect (2, 2, w - 3, h - 3)
        .adjusted (0, 0, -1, -1));


    // Draw on RGB layer.
    QPainter painter (&pixmap);
    {
        const QPen pen = kpPixmapFX::QPainterDrawRectPen (Qt::black, penWidth);

        painter.setPen (pen);
        painter.setBrush (
            brushForFillStyle (fs,
                kpColor (QColor (Qt::black).rgb ())/*foreground*/,
                kpColor (QColor (Qt::gray).rgb ())/*background*/));

        painter.drawRect (rectRect);
    }
    painter.end ();


    QBitmap mask (pixmap.width (), pixmap.height ());
    mask.fill (Qt::color0);

    // Draw on mask layer.
    painter.begin (&mask);
    {
        const QPen pen = kpPixmapFX::QPainterDrawRectPen (
            Qt::color1/*opaque*/, penWidth);

        painter.setPen (pen);
        if (fs == FillWithBackground || fs == FillWithForeground)
            painter.setBrush (Qt::color1);

        painter.drawRect (rectRect);
    }
    painter.end ();

    pixmap.setMask (mask);


    return pixmap;
}

// private
QString kpToolWidgetFillStyle::fillStyleName (FillStyle fs) const
{
    // do not complain about the "useless" breaks
    // as the return statements might not be return statements one day

    switch (fs)
    {
    case NoFill:
        return i18n ("No Fill");
        break;
    case FillWithBackground:
        return i18n ("Fill with Background Color");
        break;
    case FillWithForeground:
        return i18n ("Fill with Foreground Color");
        break;
    default:
        return QString();
        break;
    }
}


// public
kpToolWidgetFillStyle::FillStyle kpToolWidgetFillStyle::fillStyle () const
{
#if DEBUG_KP_TOOL_WIDGET_FILL_STYLE
    kDebug () << "kpToolWidgetFillStyle::fillStyle() selected="
               << selectedRow ()
               << endl;
#endif
    return (FillStyle) selectedRow ();
}

// public static
// REFACTOR: remove since this widget is for document options and we are returning an on-screen Qt brush instead of e.g. kpColor
QBrush kpToolWidgetFillStyle::maskBrushForFillStyle (FillStyle fs,
                                                     const kpColor &foregroundColor,
                                                     const kpColor &backgroundColor)
{
    // do not complain about the "useless" breaks
    // as the return statements might not be return statements one day

    switch (fs)
    {
    case NoFill:
        return Qt::NoBrush;
        break;
    case FillWithBackground:
        return QBrush (backgroundColor.maskColor ());
        break;
    case FillWithForeground:
        return QBrush (foregroundColor.maskColor ());
        break;
    default:
        return Qt::NoBrush;
        break;
    }
}

// REFACTOR: remove since this widget is for document options and we are returning an on-screen Qt brush instead of e.g. kpColor
QBrush kpToolWidgetFillStyle::maskBrush (const kpColor &foregroundColor,
                                         const kpColor &backgroundColor)
{
    return maskBrushForFillStyle (fillStyle (), foregroundColor, backgroundColor);
}

// public static
// REFACTOR: remove since this widget is for document options and we are returning an on-screen Qt brush instead of e.g. kpColor
QBrush kpToolWidgetFillStyle::brushForFillStyle (FillStyle fs,
                                                 const kpColor &foregroundColor,
                                                 const kpColor &backgroundColor)
{
    // do not complain about the "useless" breaks
    // as the return statements might not be return statements one day

    switch (fs)
    {
    case NoFill:
        return Qt::NoBrush;
        break;
    case FillWithBackground:
        if (backgroundColor.isOpaque ())
            return QBrush (backgroundColor.toQColor ());
        else
            return Qt::NoBrush;
        break;
    case FillWithForeground:
        if (foregroundColor.isOpaque ())
            return QBrush (foregroundColor.toQColor ());
        else
            return Qt::NoBrush;
        break;
    default:
        return Qt::NoBrush;
        break;
    }
}

// public
// REFACTOR: remove since this widget is for document options and we are returning an on-screen Qt brush instead of e.g. kpColor
QBrush kpToolWidgetFillStyle::brush (const kpColor &foregroundColor,
                                     const kpColor &backgroundColor)
{
    return brushForFillStyle (fillStyle (), foregroundColor, backgroundColor);
}

kpColor kpToolWidgetFillStyle::drawingBackgroundColor (
        const kpColor &foregroundColor, const kpColor &backgroundColor) const
{
    switch (fillStyle ())
    {
    default:
    case NoFill:
        return kpColor::Invalid;

    case FillWithBackground:
        return backgroundColor;

    case FillWithForeground:
        return foregroundColor;
    }
}

// virtual protected slot [base kpToolWidgetBase]
bool kpToolWidgetFillStyle::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret)
        emit fillStyleChanged (fillStyle ());
    return ret;
}


#include <kpToolWidgetFillStyle.moc>
