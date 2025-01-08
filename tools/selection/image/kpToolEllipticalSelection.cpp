
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
