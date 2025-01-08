
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_POLYLINE 0

#include "kpToolPolyline.h"
#include "environments/tools/kpToolEnvironment.h"
#include "kpLogCategories.h"
#include "pixmapfx/kpPixmapFX.h"

#include <KLocalizedString>

#include <QPainter>
#include <QPen>

//--------------------------------------------------------------------------------

kpToolPolyline::kpToolPolyline(kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase(i18n("Connected Lines"), i18n("Draws connected lines"), &drawShape, Qt::Key_N, environ, parent, QStringLiteral("tool_polyline"))
{
}

//--------------------------------------------------------------------------------

// private virtual [base kpToolPolygonalBase]
QString kpToolPolyline::haventBegunShapeUserMessage() const
{
    return i18n("Drag to draw the first line.");
}

//--------------------------------------------------------------------------------
// public static

void kpToolPolyline::drawShape(kpImage *image, const QPolygon &points, const kpColor &fcolor, int penWidth, const kpColor &bcolor, bool isFinal)
{
    (void)bcolor;
    (void)isFinal;

    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

    painter.setPen(QPen(fcolor.toQColor(), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if (kpPixmapFX::Only1PixelInPointArray(points)) {
        painter.drawPoint(points[0]);
    } else {
        painter.drawPolyline(points);
    }
}

//--------------------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolPolyline::endDraw(const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYLINE
    qCDebug(kpLogTools) << "kpToolPolyline::endDraw()  points=" << points()->toList();
#endif

    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.  We are being called by kpTool::mouseReleaseEvent().
    if (points()->count() == 0) {
        return;
    }

    if (points()->count() >= kpToolPolygonalBase::MaxPoints) {
#if DEBUG_KP_TOOL_POLYLINE
        qCDebug(kpLogTools) << "\tending shape";
#endif
        endShape();
        return;
    }

    if (originatingMouseButton() == 0) {
        setUserMessage(i18n("Left drag another line or right click to finish."));
    } else {
        setUserMessage(i18n("Right drag another line or left click to finish."));
    }
}

#include "moc_kpToolPolyline.cpp"
