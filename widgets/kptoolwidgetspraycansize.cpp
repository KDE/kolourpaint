
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

#define DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE 1

#include <qbitmap.h>
#include <qpainter.h>

#include <kdebug.h>
#include <kiconloader.h>

#include <kptoolwidgetspraycansize.h>

static int spraycanSizes [] = {10, 20, 30};

kpToolWidgetSpraycanSize::kpToolWidgetSpraycanSize (QWidget *parent)
    : kpToolWidgetBase (parent)
{
    #if DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE
        kdDebug () << "kpToolWidgetSpraycanSize::kpToolWidgetSpraycanSize() CALLED!" << endl;
    #endif

    for (int i = 0; i < int (sizeof (spraycanSizes) / sizeof (spraycanSizes [0])); i++)
    {
        int s = spraycanSizes [i];
        QString iconName = QString ("tool_spraycan_%1x%1").arg (s).arg(s);
        
    #if DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE
        kdDebug () << "\ticonName=" << iconName << endl;
    #endif

        QPixmap pixmap (s + 2, s + 2);
        pixmap.fill (Qt::white);
        
        QPainter painter (&pixmap);
        painter.drawPixmap (1, 1, UserIcon (iconName));
        painter.end ();

        pixmap.setMask (pixmap.createHeuristicMask ());
        kpToolWidgetBase::addOption (pixmap);
    }

    kpToolWidgetBase::setSelected (0);
}

kpToolWidgetSpraycanSize::~kpToolWidgetSpraycanSize ()
{
}


// public
int kpToolWidgetSpraycanSize::spraycanSize () const
{
    return spraycanSizes [kpToolWidgetBase::selected ()];
}

// protected slot virtual [base kpToolWidgetBase]
void kpToolWidgetSpraycanSize::setSelected (int which)
{
    kpToolWidgetBase::setSelected (which);
    emit spraycanSizeChanged (spraycanSize ());
};

#include <kptoolwidgetspraycansize.moc>
