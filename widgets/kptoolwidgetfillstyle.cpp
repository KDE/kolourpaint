
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <qbitmap.h>
#include <qbrush.h>
#include <qpainter.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kptoolwidgetfillstyle.h>


kpToolWidgetFillStyle::kpToolWidgetFillStyle (QWidget *parent)
    : kpToolWidgetBase (parent)
{
    kpToolWidgetBase::setInvertSelectedPixmap (false);

    for (int i = 0; i < (int) FillStyleNum; i++)
    {
        QPixmap pixmap;
        int width = 44 / 2, height = width;
        
        pixmap = fillStylePixmap ((FillStyle) i, width, height);
        kpToolWidgetBase::addOption (pixmap);
    }

    kpToolWidgetBase::setSelected (0);
}

kpToolWidgetFillStyle::~kpToolWidgetFillStyle ()
{
}


// private
QPixmap kpToolWidgetFillStyle::fillStylePixmap (FillStyle fs, int width, int height)
{
    QPixmap pixmap (width, height);
    QPainter painter;
    
    pixmap.fill (Qt::white);

    painter.begin (&pixmap);
    
    painter.setPen (QPen (Qt::black, 2));
    painter.setBrush (brushForFillStyle (fs, Qt::black/*foreground*/, Qt::red/*background*/));
    
    painter.drawRect (3, 3, width - 6, height - 6);
    
    painter.end ();

    pixmap.setMask (pixmap.createHeuristicMask ());
    
    return pixmap;
}


// public
kpToolWidgetFillStyle::FillStyle kpToolWidgetFillStyle::fillStyle () const
{
#if 1
    kdDebug () << "kpToolWidgetFillStyle::fillStyle() selected="
               << kpToolWidgetBase::selected ()
               << endl;
#endif
    return (FillStyle) kpToolWidgetBase::selected ();
}

// public static
QBrush kpToolWidgetFillStyle::brushForFillStyle (FillStyle fs,
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
        return QBrush (backgroundColor);
        break;
    case FillWithForeground:
        return QBrush (foregroundColor);
        break;
    case FillWithForeground50Percent:
        return QBrush (backgroundColor, Qt::Dense4Pattern);
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
void kpToolWidgetFillStyle::setSelected (int which)
{
    kpToolWidgetBase::setSelected (which);
    emit fillStyleChanged (fillStyle ());
};

#include <kptoolwidgetfillstyle.moc>
