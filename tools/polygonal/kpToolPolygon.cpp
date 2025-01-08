/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2017 Martin Koller <kollix@aon.at>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_POLYGON 0

#include "kpToolPolygon.h"
#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpColor.h"
#include "pixmapfx/kpPixmapFX.h"
#include "widgets/toolbars/kpToolToolBar.h"

#include <KLocalizedString>

#include <QPainter>
#include <QPen>

//--------------------------------------------------------------------------------

static void DrawPolygonShape(kpImage *image, const QPolygon &points, const kpColor &fcolor, int penWidth, const kpColor &bcolor, bool isFinal)
{
    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

    painter.setPen(QPen(fcolor.toQColor(), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if (kpPixmapFX::Only1PixelInPointArray(points)) {
        painter.drawPoint(points[0]);
        return;
    }

    if (bcolor.isValid()) {
        painter.setBrush(QBrush(bcolor.toQColor()));
    } else {
        painter.setBrush(Qt::NoBrush);
    }

    painter.drawPolygon(points, Qt::OddEvenFill);

    if (isFinal) {
        return;
    }

    if (points.count() <= 2) {
        return;
    }

    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(QPen(Qt::white));
    painter.drawLine(points[0], points[points.count() - 1]);
}

//--------------------------------------------------------------------------------

struct kpToolPolygonPrivate {
    kpToolWidgetFillStyle *toolWidgetFillStyle;
};

kpToolPolygon::kpToolPolygon(kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase(i18n("Polygon"), i18n("Draws polygons"), &::DrawPolygonShape, Qt::Key_G, environ, parent, QStringLiteral("tool_polygon"))
    , d(new kpToolPolygonPrivate())
{
}

kpToolPolygon::~kpToolPolygon()
{
    delete d;
}

// private virtual [base kpToolPolygonBase]
QString kpToolPolygon::haventBegunShapeUserMessage() const
{
    return i18n("Drag to draw the first line.");
}

// public virtual [base kpToolPolygonalBase]
void kpToolPolygon::begin()
{
    kpToolPolygonalBase::begin();

    kpToolToolBar *tb = toolToolBar();
    Q_ASSERT(tb);

    d->toolWidgetFillStyle = tb->toolWidgetFillStyle();
    connect(d->toolWidgetFillStyle, &kpToolWidgetFillStyle::fillStyleChanged, this, &kpToolPolygon::updateShape);
    d->toolWidgetFillStyle->show();
}

// public virtual [base kpToolPolygonalBase]
void kpToolPolygon::end()
{
    kpToolPolygonalBase::end();

    disconnect(d->toolWidgetFillStyle, &kpToolWidgetFillStyle::fillStyleChanged, this, &kpToolPolygon::updateShape);
    d->toolWidgetFillStyle = nullptr;
}

// TODO: code dup with kpToolRectangle
// protected virtual [base kpToolPolygonalBase]
kpColor kpToolPolygon::drawingBackgroundColor() const
{
    const kpColor foregroundColor = color(originatingMouseButton());
    const kpColor backgroundColor = color(1 - originatingMouseButton());

    return d->toolWidgetFillStyle->drawingBackgroundColor(foregroundColor, backgroundColor);
}

// public virtual [base kpTool]
// TODO: dup with kpToolPolyline but we don't want to create another level of
//       inheritance and readability.
void kpToolPolygon::endDraw(const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "kpToolPolygon::endDraw()  points=" << points()->toList();
#endif

    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.  We are being called by kpTool::mouseReleaseEvent().
    if (points()->count() == 0) {
        return;
    }

    if (points()->count() >= kpToolPolygonalBase::MaxPoints) {
#if DEBUG_KP_TOOL_POLYGON
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

#include "moc_kpToolPolygon.cpp"
