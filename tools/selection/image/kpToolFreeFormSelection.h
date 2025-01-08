
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_FREE_FORM_SELECTION_H
#define KP_TOOL_FREE_FORM_SELECTION_H

#include "kpAbstractImageSelectionTool.h"

class kpToolFreeFormSelection : public kpAbstractImageSelectionTool
{
public:
    kpToolFreeFormSelection(kpToolSelectionEnvironment *environ, QObject *parent);
    ~kpToolFreeFormSelection() override;

protected:
    bool drawCreateMoreSelectionAndUpdateStatusBar(bool dragAccepted, const QPoint &accidentalDragAdjustedPoint, const QRect &normalizedRect) override;
};

#endif // KP_TOOL_FREE_FORM_SELECTION_H
