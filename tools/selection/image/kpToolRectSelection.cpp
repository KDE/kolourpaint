
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

#define DEBUG_KP_TOOL_RECT_SELECTION 0

#include "kpToolRectSelection.h"
#include "kpLogCategories.h"
#include "document/kpDocument.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"

#include <KLocalizedString>

kpToolRectSelection::kpToolRectSelection (kpToolSelectionEnvironment *environ,
        QObject *parent)
    : kpAbstractImageSelectionTool (i18n ("Selection (Rectangular)"),
                       i18n ("Makes a rectangular selection"),
                       Qt::Key_S,
                       environ, parent,
                       QStringLiteral("tool_rect_selection"))
{
}

kpToolRectSelection::~kpToolRectSelection () = default;


// protected virtual [base kpAbstractSelectionTool]
bool kpToolRectSelection::drawCreateMoreSelectionAndUpdateStatusBar (
        bool dragAccepted,
        const QPoint &accidentalDragAdjustedPoint,
        const QRect &normalizedRect)
{
    // Prevent unintentional creation of 1-pixel selections.
    // REFACTOR: This line is duplicated code with other tools.
    if (!dragAccepted && accidentalDragAdjustedPoint == startPoint ())
    {
    #if DEBUG_KP_TOOL_RECT_SELECTION && 1
        qCDebug(kpLogTools) << "\tnon-text NOP - return";
    #endif
        setUserShapePoints (accidentalDragAdjustedPoint);
        return false;
    }

    Q_ASSERT (accidentalDragAdjustedPoint == currentPoint ());

    const QRect usefulRect = normalizedRect.intersected (document ()->rect ());
    document ()->setSelection (
        kpRectangularImageSelection (
            usefulRect,
            environ ()->imageSelectionTransparency ()));

    setUserShapePoints (startPoint (),
        QPoint (qMax (0, qMin (currentPoint ().x (), document ()->width () - 1)),
                qMax (0, qMin (currentPoint ().y (), document ()->height () - 1))));

    return true;
}
