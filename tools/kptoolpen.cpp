
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


// private
QString kpToolPen::haventBegunDrawUserMessage () const
{
    return i18n ("Click to draw dots or drag to draw strokes.");
}


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


QRect kpToolPen::drawLine (const QPoint &thisPoint, const QPoint &lastPoint)
{
    QRect docRect = kpBug::QRect_Normalized (QRect (thisPoint, lastPoint));
    // TODO: I think this is wrong for pens due to lack of m_brushPixmap.
    //       See comment for 011_kptoolpen_draw_push_down_draw_methods.diff
    //       (part of r557112: approx. 2006-07-02 22:32:37 +10:00 AEST).
    docRect = neededRect (docRect, m_brushPixmap [m_mouseButton].width ());
    QPixmap pixmap = document ()->getPixmapAt (docRect);


    QBitmap maskBitmap;
    QPainter painter, maskPainter;
    
    drawLineSetupPainterMask (&pixmap,
        &maskBitmap,
        &painter, &maskPainter);
    
        
    const QPoint sp = lastPoint - docRect.topLeft (),
                 ep = thisPoint - docRect.topLeft ();
                 
    if (painter.isActive ())
        painter.drawLine (sp, ep);

    if (maskPainter.isActive ())
        maskPainter.drawLine (sp, ep);

 
    drawLineTearDownPainterMask (&pixmap,
        &maskBitmap,
        &painter, &maskPainter);

 
    document ()->setPixmapAt (pixmap, docRect.topLeft ());
    return docRect;   
}


#include <kptoolpen.moc>
