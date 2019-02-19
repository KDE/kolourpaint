
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


#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"

#include "imagelib/kpColor.h"
#include "pixmapfx/kpPixmapFX.h"

#include <KLocalizedString>

#include <QBitmap>
#include <QPainter>
#include <QPixmap>


static int lineWidths [] = {1, 2, 3, 5, 8};

kpToolWidgetLineWidth::kpToolWidgetLineWidth (QWidget *parent, const QString &name)
    : kpToolWidgetBase (parent, name)
{
    int numLineWidths = sizeof (lineWidths) / sizeof (lineWidths [0]);

    int w = (width () - 2/*margin*/) * 3 / 4;
    int h = (height () - 2/*margin*/ - (numLineWidths - 1)/*spacing*/) * 3 / (numLineWidths * 4);

    for (int i = 0; i < numLineWidths; i++)
    {
        QImage image ((w <= 0 ? width () : w),
                        (h <= 0 ? height () : h), QImage::Format_ARGB32_Premultiplied);
        image.fill(QColor(Qt::transparent).rgba());


        kpPixmapFX::fillRect (&image,
            0, (image.height () - lineWidths [i]) / 2,
            image.width (), lineWidths [i],
            kpColor::Black);
        

        addOption (QPixmap::fromImage(image), QString::number (lineWidths [i]));
        startNewOptionRow ();
    }

    finishConstruction (0, 0);
}

kpToolWidgetLineWidth::~kpToolWidgetLineWidth () = default;

int kpToolWidgetLineWidth::lineWidth () const
{
    return lineWidths [selectedRow ()];
}

// virtual protected slot [base kpToolWidgetBase]
bool kpToolWidgetLineWidth::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret) {
        emit lineWidthChanged (lineWidth ());
    }
    return ret;
}


