
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <qbitmap.h>
#include <qbrush.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kptool.h>
#include <kptoolwidgetfillstyle.h>


kpToolWidgetFillStyle::kpToolWidgetFillStyle (QWidget *parent)
    : kpToolWidgetBase (parent)
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

    relayoutOptions ();
    setSelected (0, 0);
}

kpToolWidgetFillStyle::~kpToolWidgetFillStyle ()
{
}


// private
QPixmap kpToolWidgetFillStyle::fillStylePixmap (FillStyle fs, int w, int h)
{
    QPixmap pixmap ((w <= 0 ? width () : w), (h <= 0 ? height () : h));
    pixmap.fill (Qt::white);

    QPainter painter (&pixmap);
    
    painter.setPen (QPen (Qt::black, 2));
    painter.setBrush (brushForFillStyle (fs, Qt::black/*foreground*/, Qt::gray/*background*/));
    
    painter.drawRect (2, 2, w - 3, h - 3);
    
    painter.end ();

    
    QBitmap mask (pixmap.width (), pixmap.height ());
    mask.fill (Qt::color0);
    
    painter.begin (&mask);
    painter.setPen (QPen (Qt::color1, 2));

    if (fs == FillWithBackground || fs == FillWithForeground)
        painter.setBrush (Qt::color1);

    painter.drawRect (2, 2, w - 3, h - 3);
    
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
        return QString::null;
        break;
    }
}


// public
kpToolWidgetFillStyle::FillStyle kpToolWidgetFillStyle::fillStyle () const
{
#if 1
    kdDebug () << "kpToolWidgetFillStyle::fillStyle() selected="
               << selectedRow ()
               << endl;
#endif
    return (FillStyle) selectedRow ();
}

// public static
QBrush kpToolWidgetFillStyle::maskBrushForFillStyle (FillStyle fs,
                                                     const QColor &foregroundColor,
                                                     const QColor &backgroundColor)
{
    // do not complain about the "useless" breaks
    // as the return statements might not be return statements one day

    switch (fs)
    {
    case NoFill:
        return Qt::NoBrush;
        break;
    case FillWithBackground:
        if (kpTool::isColorTransparent (backgroundColor))
            return QBrush (Qt::color0/*transparent*/);
        else
            return QBrush (Qt::color1/*opaque*/);
        break;
    case FillWithForeground:
        if (kpTool::isColorTransparent (foregroundColor))
            return QBrush (Qt::color0/*transparent*/);
        else
            return QBrush (Qt::color1/*opaque*/);
        break;
    default:
        return Qt::NoBrush;
        break;
    }
}

QBrush kpToolWidgetFillStyle::maskBrush (const QColor &foregroundColor,
                                         const QColor &backgroundColor)
{
    return maskBrushForFillStyle (fillStyle (), foregroundColor, backgroundColor);
}

// public static
QBrush kpToolWidgetFillStyle::brushForFillStyle (FillStyle fs,
                                                 const QColor &foregroundColor,
                                                 const QColor &backgroundColor)
{
    // do not complain about the "useless" breaks
    // as the return statements might not be return statements one day

    // sync: kptoolpolygon.cpp pixmap()

    switch (fs)
    {
    case NoFill:
        return Qt::NoBrush;
        break;
    case FillWithBackground:
        if (kpTool::isColorOpaque (backgroundColor))
            return QBrush (backgroundColor);
        else
            return Qt::NoBrush;
        break;
    case FillWithForeground:
        if (kpTool::isColorOpaque (foregroundColor))
            return QBrush (foregroundColor);
        else
            return Qt::NoBrush;
        break;
    default:
        return Qt::NoBrush;
        break;
    }
}

// public
QBrush kpToolWidgetFillStyle::brush (const QColor &foregroundColor,
                                     const QColor &backgroundColor)
{
    return brushForFillStyle (fillStyle (), foregroundColor, backgroundColor);
}


// virtual protected slot [base kpToolWidgetBase]
void kpToolWidgetFillStyle::setSelected (int row, int col)
{
    kpToolWidgetBase::setSelected (row, col);
    emit fillStyleChanged (fillStyle ());
};

#include <kptoolwidgetfillstyle.moc>
