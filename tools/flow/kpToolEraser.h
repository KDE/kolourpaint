
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_ERASER_H
#define KP_TOOL_ERASER_H

#include "kpToolFlowPixmapBase.h"

// Eraser = Brush but with foreground & background colors swapped (a few square brushes)
class kpToolEraser : public kpToolFlowPixmapBase
{
    Q_OBJECT

public:
    kpToolEraser(kpToolEnvironment *environ, QObject *parent);

    void globalDraw() override;

protected:
    QString haventBegunDrawUserMessage() const override;

    bool haveSquareBrushes() const override
    {
        return true;
    }
    bool colorsAreSwapped() const override
    {
        return true;
    }
};

#endif // KP_TOOL_ERASER_H
