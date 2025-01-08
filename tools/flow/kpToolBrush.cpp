
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpToolBrush.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpToolBrush::kpToolBrush(kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowPixmapBase(i18n("Brush"), i18n("Draw using brushes of different shapes and sizes"), Qt::Key_B, environ, parent, QStringLiteral("tool_brush"))
{
}

//---------------------------------------------------------------------

// protected virtual [base kpToolFlowBase]
QString kpToolBrush::haventBegunDrawUserMessage() const
{
    return i18n("Click to draw dots or drag to draw strokes.");
}

//---------------------------------------------------------------------

// See our corresponding .h for brush selection.

// Logic is in kpToolFlowPixmapBase.

#include "moc_kpToolBrush.cpp"
