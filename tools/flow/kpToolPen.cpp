
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


#include <kpToolPen.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>

#include <klocale.h>

#include <kpBug.h>
#include <kpColor.h>
#include <kpDocument.h>
#include <kpImage.h>
#include <kpPainter.h>
#include <kpToolFlowCommand.h>


struct kpToolPenPrivate
{
};

kpToolPen::kpToolPen (kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowBase (i18n ("Pen"), i18n ("Draws dots and freehand strokes"),
        Qt::Key_P,
        environ, parent, "tool_pen"),
      d (new kpToolPenPrivate ())
{
}

kpToolPen::~kpToolPen ()
{
    delete d;
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
    kpImage image (1, 1);

    const kpColor c = color (mouseButton ());


    // OPT: this seems hopelessly inefficient
    if (c.isOpaque ())
    {
        image.fill (c.toQColor ());
    }
    else
    {
        QBitmap mask (1, 1);
        mask.fill (Qt::color0/*transparent*/);

        image.setMask (mask);
    }

    // draw onto doc
    document ()->setImageAt (image, point);

    return QRect (point, point);
}
#endif


// protected virtual [base kpToolFlowBase]
QRect kpToolPen::drawLine (const QPoint &thisPoint, const QPoint &lastPoint)
{
    QRect docRect = kpBug::QRect_Normalized (QRect (thisPoint, lastPoint));
    docRect = neededRect (docRect, 1/*pen width*/);
    kpImage image = document ()->getImageAt (docRect);


    const QPoint sp = lastPoint - docRect.topLeft (),
                 ep = thisPoint - docRect.topLeft ();

    kpPainter::drawLine (&image,
        sp.x (), sp.y (),
        ep.x (), ep.y (),
        color (mouseButton ()),
        1/*pen width*/);


    document ()->setImageAt (image, docRect.topLeft ());
    return docRect;
}


#include <kpToolPen.moc>
