
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_ELLIPTICAL_SELECTION 0

#include "kpToolEllipticalSelection.h"

#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpEllipticalImageSelection.h"

#include <KLocalizedString>

kpToolEllipticalSelection::kpToolEllipticalSelection(kpToolSelectionEnvironment *environ, QObject *parent)
    : kpAbstractImageSelectionTool(i18n("Selection (Elliptical)"),
                                   i18n("Makes an elliptical or circular selection"),
                                   Qt::Key_I,
                                   environ,
                                   parent,
                                   QStringLiteral("tool_elliptical_selection"))
{
}

kpToolEllipticalSelection::~kpToolEllipticalSelection() = default;

// protected virtual [base kpAbstractSelectionTool]
bool kpToolEllipticalSelection::drawCreateMoreSelectionAndUpdateStatusBar(bool dragAccepted,
                                                                          const QPoint &accidentalDragAdjustedPoint,
                                                                          const QRect &normalizedRect)
{
    // Prevent unintentional creation of 1-pixel selections.
    if (!dragAccepted && accidentalDragAdjustedPoint == startPoint()) {
#if DEBUG_KP_TOOL_ELLIPTICAL_SELECTION && 1
        qCDebug(kpLogTools) << "\tnon-text NOP - return";
#endif
        setUserShapePoints(accidentalDragAdjustedPoint);
        return false;
    }

    Q_ASSERT(accidentalDragAdjustedPoint == currentPoint());

    document()->setSelection(kpEllipticalImageSelection(normalizedRect, environ()->imageSelectionTransparency()));

    setUserShapePoints(startPoint(), currentPoint());

    return true;
}
