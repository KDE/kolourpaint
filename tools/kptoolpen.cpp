
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


#include <kptoolpen.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <klocale.h>

#include <kpbug.h>
#include <kpcolor.h>
#include <kpdocument.h>
#include <kpimage.h>
#include <kppainter.h>
#include <kptoolflowcommand.h>


//
// kpToolPen
//

kpToolPen::kpToolPen (kpMainWindow *mainWindow)
    : kpToolFlowBase (i18n ("Pen"), i18n ("Draws dots and freehand strokes"),
        Qt::Key_P,
        mainWindow, "tool_pen")
{
}

kpToolPen::~kpToolPen ()
{
}


// protected virtual [base kpToolFlowBase]
QString kpToolPen::haventBegunDrawUserMessage () const
{
    return i18n ("Click to draw dots or drag to draw strokes.");
}


// Wants porting to Qt4.  But may be a bogus optimization anyway.
#if 0
QRect kpToolPen::drawPoint (const QPoint &point)
{
    QPixmap pixmap (1, 1);

    const kpColor c = color (m_mouseButton);


    // OPT: this seems hopelessly inefficient
    if (c.isOpaque ())
    {
        pixmap.fill (c.toQColor ());
    }
    else
    {
        QBitmap mask (1, 1);
        mask.fill (Qt::color0/*transparent*/);

        pixmap.setMask (mask);
    }

    // draw onto doc
    document ()->setPixmapAt (pixmap, point);

    return QRect (point, point);
}
#endif


// protected virtual [base kpToolFlowBase]
QRect kpToolPen::drawLine (const QPoint &thisPoint, const QPoint &lastPoint)
{
    QRect docRect = kpBug::QRect_Normalized (QRect (thisPoint, lastPoint));
    docRect = neededRect (docRect, 1/*pen width*/);
    kpImage image = document ()->getPixmapAt (docRect);


    const QPoint sp = lastPoint - docRect.topLeft (),
                 ep = thisPoint - docRect.topLeft ();
    
    kpPainter::drawLine (&image,
        sp.x (), sp.y (),
        ep.x (), ep.y (),
        color (m_mouseButton),
        1/*pen width*/);

 
    document ()->setPixmapAt (image, docRect.topLeft ());
    return docRect;   
}


#include <kptoolpen.moc>
