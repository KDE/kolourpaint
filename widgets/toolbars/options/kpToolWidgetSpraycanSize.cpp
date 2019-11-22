
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


#define DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE 0


#include "kpToolWidgetSpraycanSize.h"

#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QPixmap>


static int spraycanSizes [] = {9, 17, 29};

kpToolWidgetSpraycanSize::kpToolWidgetSpraycanSize (QWidget *parent, const QString &name)
    : kpToolWidgetBase (parent, name)
{
#if DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE
    qCDebug(kpLogWidgets) << "kpToolWidgetSpraycanSize::kpToolWidgetSpraycanSize() CALLED!";
#endif

    for (int i = 0; i < int (sizeof (spraycanSizes) / sizeof (spraycanSizes [0])); i++)
    {
        int s = spraycanSizes [i];
        const QString iconName = QStringLiteral (":/icons/tool_spraycan_%1x%2").arg (s).arg(s);
        
    #if DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE
        qCDebug(kpLogWidgets) << "\ticonName=" << iconName;
    #endif

        QPixmap pixmap (s, s);
        pixmap.fill (Qt::white);
        
        QPainter painter (&pixmap);
        painter.drawPixmap (0, 0, QPixmap (iconName));
        painter.end ();

        QImage image = pixmap.toImage();

        QBitmap mask (pixmap.width (), pixmap.height ());
        mask.fill (Qt::color0);

        painter.begin (&mask);
        painter.setPen (Qt::color1);
        
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                if ((image.pixel (x, y) & RGB_MASK) == 0/*black*/) {
                    painter.drawPoint (x, y);  // mark as opaque
                }
            }
        }

        painter.end ();

        pixmap.setMask (mask);
        
        addOption (pixmap, i18n ("%1x%2", s, s)/*tooltip*/);
        if (i == 1) {
            startNewOptionRow ();
        }
    }

    finishConstruction (0, 0);
}

kpToolWidgetSpraycanSize::~kpToolWidgetSpraycanSize () = default;


// public
int kpToolWidgetSpraycanSize::spraycanSize () const
{
    return spraycanSizes[selected() < 0 ? 0 : selected()];
}

// protected slot virtual [base kpToolWidgetBase]
bool kpToolWidgetSpraycanSize::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret) {
        emit spraycanSizeChanged (spraycanSize ());
    }
    return ret;
}

