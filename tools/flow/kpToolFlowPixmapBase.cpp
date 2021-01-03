
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


#include "kpToolFlowPixmapBase.h"

#include "imagelib/kpColor.h"
#include "document/kpDocument.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "commands/tools/flow/kpToolFlowCommand.h"

//---------------------------------------------------------------------

kpToolFlowPixmapBase::kpToolFlowPixmapBase (const QString &text, const QString &description,
            int key,
            kpToolEnvironment *environ, QObject *parent, const QString &name)
    : kpToolFlowBase (text, description, key, environ, parent, name)
{
}

//---------------------------------------------------------------------

QRect kpToolFlowPixmapBase::drawLine (const QPoint &thisPoint, const QPoint &lastPoint)
{
    QRect docRect = kpPainter::normalizedRect(thisPoint, lastPoint);
    docRect = neededRect (docRect, qMax (brushWidth (), brushHeight ()));
    kpImage image = document ()->getImageAt (docRect);


    QList <QPoint> points = kpPainter::interpolatePoints (lastPoint, thisPoint,
        brushIsDiagonalLine ());

    foreach (const QPoint &p, points)
    {
        const QPoint point =
            hotRectForMousePointAndBrushWidthHeight(p, brushWidth(), brushHeight())
                    .topLeft() - docRect.topLeft();

        // OPT: This may be redrawing pixels that were drawn on a previous
        //      iteration, since the brush is usually bigger than 1 pixel.
        //      Maybe we could use QRegion to determine all the non-intersecting
        //      regions and only draw each region once.
        //
        //      Try this at least for the easy case of the Eraser, which has
        //      square, simply-filled brushes.  Profiling needs to be done as
        //      QRegion is known to be a CPU hog.
        brushDrawFunction () (&image, point, brushDrawFunctionData ());
    }


    document ()->setImageAt (image, docRect.topLeft ());
    return docRect;
}

//---------------------------------------------------------------------

