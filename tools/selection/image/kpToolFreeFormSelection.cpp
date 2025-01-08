
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_FREE_FROM_SELECTION 0

#include "kpToolFreeFormSelection.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"

//---------------------------------------------------------------------

kpToolFreeFormSelection::kpToolFreeFormSelection(kpToolSelectionEnvironment *environ, QObject *parent)
    : kpAbstractImageSelectionTool(i18n("Selection (Free-Form)"),
                                   i18n("Makes a free-form selection"),
                                   Qt::Key_M,
                                   environ,
                                   parent,
                                   QStringLiteral("tool_free_form_selection"))
{
}

//---------------------------------------------------------------------

kpToolFreeFormSelection::~kpToolFreeFormSelection() = default;

//---------------------------------------------------------------------

// protected virtual [base kpAbstractSelectionTool]
bool kpToolFreeFormSelection::drawCreateMoreSelectionAndUpdateStatusBar(bool dragAccepted,
                                                                        const QPoint &accidentalDragAdjustedPoint,
                                                                        const QRect & /*normalizedRect*/)
{
#if DEBUG_KP_TOOL_FREE_FROM_SELECTION
    qCDebug(kpLogTools) << "kpToolFreeFormSelection::createMoreSelectionAndUpdateStatusBar("
                        << "dragAccepted=" << dragAccepted << ",accidentalDragAdjustedPoint=" << accidentalDragAdjustedPoint << ")";
#endif

    // Prevent unintentional creation of 1-pixel selections.
    if (!dragAccepted && accidentalDragAdjustedPoint == startPoint()) {
#if DEBUG_KP_TOOL_FREE_FROM_SELECTION && 1
        qCDebug(kpLogTools) << "\tnon-text NOP - return";
#endif
        setUserShapePoints(accidentalDragAdjustedPoint);
        return false;
    }

    Q_ASSERT(accidentalDragAdjustedPoint == currentPoint());
    Q_ASSERT(dragAccepted == static_cast<bool>(document()->selection()));

    const kpFreeFormImageSelection *oldPointsSel = nullptr;
    if (document()->selection()) {
        kpAbstractSelection *sel = document()->selection();
        Q_ASSERT(dynamic_cast<kpFreeFormImageSelection *>(sel));
        oldPointsSel = dynamic_cast<kpFreeFormImageSelection *>(sel);
    }

    QPolygon points;

    // First point in drag?
    if (!dragAccepted) {
        points.append(startPoint());
    }
    // Not first point in drag.
    else {
        if (!oldPointsSel) { // assert above says we never reach this, but let's make coverity happy
            return false;
        }

        // Get existing points in selection.
        points = oldPointsSel->cardinallyAdjacentPoints();
    }

#if DEBUG_KP_TOOL_FREE_FROM_SELECTION
    qCDebug(kpLogTools) << "\tlast old point=" << points.last();
#endif

    // TODO: There should be an upper limit on this before drawing the
    //       polygon becomes too slow.
    points.append(accidentalDragAdjustedPoint);

    document()->setSelection(kpFreeFormImageSelection(points, environ()->imageSelectionTransparency()));

    // Prevent accidental usage of dangling pointer to old selection
    // (deleted by kpDocument::setSelection()).
    oldPointsSel = nullptr;

#if DEBUG_KP_TOOL_FREE_FROM_SELECTION && 1
    qCDebug(kpLogTools) << "\t\tfreeform; #points=" << document()->selection()->calculatePoints().count();
#endif

    setUserShapePoints(accidentalDragAdjustedPoint);

    return true;
}

//---------------------------------------------------------------------
