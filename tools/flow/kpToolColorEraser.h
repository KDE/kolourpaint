
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_COLOR_ERASER_H
#define KP_TOOL_COLOR_ERASER_H

#include "kpToolFlowBase.h"

// Color Eraser = Brush that replaces/washes the foreground color with the background color
class kpToolColorEraser : public kpToolFlowBase
{
    Q_OBJECT

public:
    kpToolColorEraser(kpToolEnvironment *environ, QObject *parent);
    ~kpToolColorEraser() override;

public:
    void globalDraw() override;

protected:
    QString haventBegunDrawUserMessage() const override;

    bool drawShouldProceed(const QPoint &thisPoint, const QPoint &lastPoint, const QRect &normalizedRect) override;

    bool haveSquareBrushes() const override
    {
        return true;
    }
    bool colorsAreSwapped() const override
    {
        return true;
    }

    QRect drawLine(const QPoint &thisPoint, const QPoint &lastPoint) override;
};

#endif // KP_TOOL_COLOR_ERASER_H
