/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_LINE 0

#include "kpToolLine.h"
#include "kpLogCategories.h"
#include "kpToolPolyline.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpToolLine::kpToolLine(kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase(i18n("Line"), i18n("Draws lines"), &kpToolPolyline::drawShape, Qt::Key_L, environ, parent, QStringLiteral("tool_line"))
{
}

//--------------------------------------------------------------------------------

// private virtual [base kpToolPolygonalBase]
QString kpToolLine::haventBegunShapeUserMessage() const
{
    return i18n("Drag to draw.");
}

//--------------------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolLine::endDraw(const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_LINE
    qCDebug(kpLogTools) << "kpToolLine::endDraw()  points=" << points()->toList() << endl;
#endif

    // After the first drag, we should have a line.
    Q_ASSERT(points()->count() == 2);
    endShape();
}

//--------------------------------------------------------------------------------

#include "moc_kpToolLine.cpp"
