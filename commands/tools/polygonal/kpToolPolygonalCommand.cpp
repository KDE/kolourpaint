
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_POLYGON 0


#include <kpToolPolygonalCommand.h>

#include <float.h>
#include <math.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpdefs.h>
#include <kpimage.h>
#include <kpmainwindow.h>
#include <kppainter.h>
#include <kppixmapfx.h>
#include <kptemppixmap.h>
#include <kpToolPolygonalBase.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetlinewidth.h>
#include <kpviewmanager.h>


struct kpToolPolygonalCommandPrivate
{
    kpToolPolygonalBase::Mode mode;
    
    QPolygon points;
    QRect normalizedRect;

    kpColor fcolor;
    int penWidth;
    kpColor bcolor;

    kpImage oldImage;
};

kpToolPolygonalCommand::kpToolPolygonalCommand (const QString &name,
        enum kpToolPolygonalBase::Mode mode,
        const QPolygon &points,
        const QRect &normalizedRect,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        const QPixmap &oldImage,
        kpMainWindow *mainWindow)
        
    : kpNamedCommand (name, mainWindow),
      d (new kpToolPolygonalCommandPrivate ())
{
      d->mode = mode;
      
      d->points = points;
      d->normalizedRect = normalizedRect;
      
      d->fcolor = fcolor; 
      d->penWidth = penWidth;
      d->bcolor = bcolor;
      
      d->oldImage = oldImage;
}

kpToolPolygonalCommand::~kpToolPolygonalCommand ()
{
    delete d;
}


// public virtual [base kpCommand]
int kpToolPolygonalCommand::size () const
{
    return kpPixmapFX::pointArraySize (d->points) +
           kpPixmapFX::pixmapSize (d->oldImage);
}

// public virtual [base kpCommand]
void kpToolPolygonalCommand::execute ()
{
    QPixmap p =
        ::kpToolPolygonalBaseImage (
            d->oldImage,
            d->points, d->normalizedRect,
            d->fcolor, d->penWidth,
            d->bcolor,
            d->mode);
    document ()->setPixmapAt (p, d->normalizedRect.topLeft ());
}

// public virtual [base kpCommand]
void kpToolPolygonalCommand::unexecute ()
{
    document ()->setPixmapAt (d->oldImage, d->normalizedRect.topLeft ());
}


#include <kpToolPolygonalCommand.moc>
