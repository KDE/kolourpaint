
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_POLYLINE_H
#define KP_TOOL_POLYLINE_H

#include "kpToolPolygonalBase.h"

class kpToolPolyline : public kpToolPolygonalBase
{
    Q_OBJECT

public:
    kpToolPolyline(kpToolEnvironment *environ, QObject *parent);

private:
    QString haventBegunShapeUserMessage() const override;

public:
    // (used by kpToolLine)
    static void drawShape(kpImage *image, const QPolygon &points, const kpColor &fcolor, int penWidth, const kpColor &bcolor, bool isFinal);

    void endDraw(const QPoint &, const QRect &) override;
};

#endif // KP_TOOL_POLYLINE_H
