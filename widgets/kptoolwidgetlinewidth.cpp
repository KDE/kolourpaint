
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
#include <qpainter.h>

#include <kptoolwidgetlinewidth.h>

static int lineWidths [] = {1, 2, 3, 4, 6, 8};

kpToolWidgetLineWidth::kpToolWidgetLineWidth (QWidget *parent)
    : kpToolWidgetBase (parent)
{
    QPixmap pixmap (44, 11);

    for (int i = 0; i < int (sizeof (lineWidths) / sizeof (lineWidths [0])); i++)
    {
        QPainter painter;

        pixmap.fill (Qt::white);
        painter.begin (&pixmap);
        painter.setBrush (Qt::black);
        painter.drawRect (3, (pixmap.height () - lineWidths [i]) / 2,
                          pixmap.width () - 6, lineWidths [i]);
        painter.end ();
        pixmap.setMask (pixmap.createHeuristicMask ());
        kpToolWidgetBase::addOption (pixmap);
    }

    kpToolWidgetBase::setSelected (0);
}

kpToolWidgetLineWidth::~kpToolWidgetLineWidth ()
{
}

int kpToolWidgetLineWidth::lineWidth () const
{
    return lineWidths [kpToolWidgetBase::selected ()];
}

// virtual protected slot
void kpToolWidgetLineWidth::setSelected (int which)
{
    kpToolWidgetBase::setSelected (which);
    emit lineWidthChanged (lineWidth ());
};

#include <kptoolwidgetlinewidth.moc>
