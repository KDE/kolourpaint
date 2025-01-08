
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_RECT_SELECTION 0

#include "kpToolRectSelection.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpRectangularImageSelection.h"

#include <KLocalizedString>

kpToolRectSelection::kpToolRectSelection(kpToolSelectionEnvironment *environ, QObject *parent)
    : kpAbstractImageSelectionTool(i18n("Selection (Rectangular)"),
                                   i18n("Makes a rectangular selection"),
                                   Qt::Key_S,
                                   environ,
                                   parent,
                                   QStringLiteral("tool_rect_selection"))
{
}

kpToolRectSelection::~kpToolRectSelection() = default;

// protected virtual [base kpAbstractSelectionTool]
bool kpToolRectSelection::drawCreateMoreSelectionAndUpdateStatusBar(bool dragAccepted, const QPoint &accidentalDragAdjustedPoint, const QRect &normalizedRect)
{
    // Prevent unintentional creation of 1-pixel selections.
    // REFACTOR: This line is duplicated code with other tools.
    if (!dragAccepted && accidentalDragAdjustedPoint == startPoint()) {
#if DEBUG_KP_TOOL_RECT_SELECTION && 1
        qCDebug(kpLogTools) << "\tnon-text NOP - return";
#endif
        setUserShapePoints(accidentalDragAdjustedPoint);
        return false;
    }

    Q_ASSERT(accidentalDragAdjustedPoint == currentPoint());

    const QRect usefulRect = normalizedRect.intersected(document()->rect());
    document()->setSelection(kpRectangularImageSelection(usefulRect, environ()->imageSelectionTransparency()));

    setUserShapePoints(startPoint(),
                       QPoint(qMax(0, qMin(currentPoint().x(), document()->width() - 1)), qMax(0, qMin(currentPoint().y(), document()->height() - 1))));

    return true;
}
