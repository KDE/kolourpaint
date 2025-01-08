
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_BRUSH_H
#define KP_TOOL_BRUSH_H

#include "kpToolFlowPixmapBase.h"

// Brush = draws pixmaps, "interpolates" by "sweeping" pixmaps along a line (interesting brushes)
class kpToolBrush : public kpToolFlowPixmapBase
{
    Q_OBJECT

public:
    kpToolBrush(kpToolEnvironment *environ, QObject *parent);

protected:
    QString haventBegunDrawUserMessage() const override;
    bool haveDiverseBrushes() const override
    {
        return true;
    }
};

#endif // KP_TOOL_BRUSH_H
