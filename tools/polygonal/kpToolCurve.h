
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_CURVE_H
#define KP_TOOL_CURVE_H

#include "kpToolPolygonalBase.h"

class kpToolCurve : public kpToolPolygonalBase
{
    Q_OBJECT

public:
    kpToolCurve(kpToolEnvironment *environ, QObject *parent);
    ~kpToolCurve() override;

protected:
    QString haventBegunShapeUserMessage() const override;

    bool drawingALine() const override;

public:
    void endDraw(const QPoint &, const QRect &) override;
};

#endif // KP_TOOL_CURVE_H
