
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SPRAYCAN 0

#include "kpToolSpraycan.h"

#include "commands/tools/flow/kpToolFlowCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpPainter.h"
#include "kpDefs.h"
#include "pixmapfx/kpPixmapFX.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetSpraycanSize.h"

#include <cstdlib>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QPoint>
#include <QRect>
#include <QTimer>

//---------------------------------------------------------------------

kpToolSpraycan::kpToolSpraycan(kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowBase(i18n("Spraycan"), i18n("Sprays graffiti"), Qt::Key_Y, environ, parent, QStringLiteral("tool_spraycan"))
    , m_toolWidgetSpraycanSize(nullptr)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(25 /*ms*/);
    connect(m_timer, &QTimer::timeout, this, &kpToolSpraycan::timeoutDraw);
}

//---------------------------------------------------------------------

// protected virtual [base kpToolFlowBase]
QString kpToolSpraycan::haventBegunDrawUserMessage() const
{
    return i18n("Click or drag to spray graffiti.");
}

//---------------------------------------------------------------------

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::begin()
{
    kpToolToolBar *tb = toolToolBar();
    Q_ASSERT(tb);

    m_toolWidgetSpraycanSize = tb->toolWidgetSpraycanSize();
    connect(m_toolWidgetSpraycanSize, &kpToolWidgetSpraycanSize::spraycanSizeChanged, this, &kpToolSpraycan::slotSpraycanSizeChanged);
    m_toolWidgetSpraycanSize->show();

    kpToolFlowBase::begin();
}

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::end()
{
    kpToolFlowBase::end();

    disconnect(m_toolWidgetSpraycanSize, &kpToolWidgetSpraycanSize::spraycanSizeChanged, this, &kpToolSpraycan::slotSpraycanSizeChanged);

    m_toolWidgetSpraycanSize = nullptr;
}

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::beginDraw()
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "kpToolSpraycan::beginDraw()";
#endif

    kpToolFlowBase::beginDraw();

    // We draw even if the user doesn't move the mouse.
    // We still timeout-draw even if the user _does_ move the mouse.
    m_timer->start();
}

// protected
QRect kpToolSpraycan::drawLineWithProbability(const QPoint &thisPoint, const QPoint &lastPoint, double probability)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "CALL(thisPoint=" << thisPoint << ",lastPoint=" << lastPoint << ")";
#endif

    QList<QPoint> docPoints = kpPainter::interpolatePoints(lastPoint, thisPoint, false /*no need for cardinally adjacency points*/, probability);
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "\tdocPoints=" << docPoints;
#endif

    // By chance no points to draw?
    if (docPoints.empty()) {
        return {};
    }

    // For efficiency, only get image after NOP check above.
    QRect docRect = kpPainter::normalizedRect(thisPoint, lastPoint);
    docRect = neededRect(docRect, spraycanSize());
    kpImage image = document()->getImageAt(docRect);

    // Spray at each point, onto the image.
    //
    // Note in passing: Unlike other tools such as the Brush, drawing
    //                  over the same point does result in a different
    //                  appearance.

    QList<QPoint> imagePoints;
    for (const auto &dp : docPoints)
        imagePoints.append(dp - docRect.topLeft());

    kpPainter::sprayPoints(&image, imagePoints, color(mouseButton()), spraycanSize());

    viewManager()->setFastUpdates();
    document()->setImageAt(image, docRect.topLeft());
    viewManager()->restoreFastUpdates();

    return docRect;
}

// public virtual [base kpToolFlowBase]
QRect kpToolSpraycan::drawPoint(const QPoint &point)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "kpToolSpraycan::drawPoint" << point << " lastPoint=" << lastPoint();
#endif

    // If this is the first in the flow or if the user is moving the spray,
    // make the spray line continuous.
    if (point != lastPoint()) {
        // Draw at this single point without delay.
        return drawLineWithProbability(point, point, 1.0 /*100% chance of drawing*/);
    }

    return {};
}

// public virtual [base kpToolFlowBase]
QRect kpToolSpraycan::drawLine(const QPoint &thisPoint, const QPoint &lastPoint)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "CALL(thisPoint=" << thisPoint << ",lastPoint=" << lastPoint;
#endif

    // Draw only every so often in response to movement.
    return drawLineWithProbability(thisPoint, lastPoint, 0.05 /*less dense: select 5% of adjacent pixels - not all*/);
}

// protected slot
void kpToolSpraycan::timeoutDraw()
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "kpToolSpraycan::timeoutDraw()";
#endif

    // Draw at this single point without delay.
    const QRect drawnRect = drawLineWithProbability(currentPoint(), currentPoint(), 1.0 /*100% chance of drawing*/);

    // kpToolFlowBase() does this after calling drawPoint() and drawLine() so
    // we need to do it too.
    currentCommand()->updateBoundingRect(drawnRect);
}

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::cancelShape()
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "kpToolSpraycan::cancelShape()";
#endif

    m_timer->stop();
    kpToolFlowBase::cancelShape();
}

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::endDraw(const QPoint &thisPoint, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    qCDebug(kpLogTools) << "kpToolSpraycan::endDraw(thisPoint=" << thisPoint << ")";
#endif

    m_timer->stop();
    kpToolFlowBase::endDraw(thisPoint, normalizedRect);
}

// protected
int kpToolSpraycan::spraycanSize() const
{
    return m_toolWidgetSpraycanSize->spraycanSize();
}

// protected slot
void kpToolSpraycan::slotSpraycanSizeChanged(int size)
{
    (void)size;
}

#include "moc_kpToolSpraycan.cpp"
