
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_PEN_H
#define KP_TOOL_PEN_H

#include "kpToolFlowBase.h"

// Pen = draws pixels, "interpolates" by "sweeping" pixels along a line (no brushes)
class kpToolPen : public kpToolFlowBase
{
    Q_OBJECT

public:
    kpToolPen(kpToolEnvironment *environ, QObject *parent);

protected:
    QString haventBegunDrawUserMessage() const override;
    QRect drawLine(const QPoint &thisPoint, const QPoint &lastPoint) override;
};

#endif // KP_TOOL_PEN_H
