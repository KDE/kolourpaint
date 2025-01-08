
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpToolFlowPixmapBase.h"

#include "commands/tools/flow/kpToolFlowCommand.h"
#include "document/kpDocument.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"

//---------------------------------------------------------------------

kpToolFlowPixmapBase::kpToolFlowPixmapBase(const QString &text,
                                           const QString &description,
                                           int key,
                                           kpToolEnvironment *environ,
                                           QObject *parent,
                                           const QString &name)
    : kpToolFlowBase(text, description, key, environ, parent, name)
{
}

//---------------------------------------------------------------------

QRect kpToolFlowPixmapBase::drawLine(const QPoint &thisPoint, const QPoint &lastPoint)
{
    QRect docRect = kpPainter::normalizedRect(thisPoint, lastPoint);
    docRect = neededRect(docRect, qMax(brushWidth(), brushHeight()));
    kpImage image = document()->getImageAt(docRect);

    const QList<QPoint> points = kpPainter::interpolatePoints(lastPoint, thisPoint, brushIsDiagonalLine());

    for (const QPoint &p : points) {
        const QPoint point = hotRectForMousePointAndBrushWidthHeight(p, brushWidth(), brushHeight()).topLeft() - docRect.topLeft();

        // OPT: This may be redrawing pixels that were drawn on a previous
        //      iteration, since the brush is usually bigger than 1 pixel.
        //      Maybe we could use QRegion to determine all the non-intersecting
        //      regions and only draw each region once.
        //
        //      Try this at least for the easy case of the Eraser, which has
        //      square, simply-filled brushes.  Profiling needs to be done as
        //      QRegion is known to be a CPU hog.
        brushDrawFunction()(&image, point, brushDrawFunctionData());
    }

    document()->setImageAt(image, docRect.topLeft());
    return docRect;
}

//---------------------------------------------------------------------

#include "moc_kpToolFlowPixmapBase.cpp"
