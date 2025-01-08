
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_POLYGON_H
#define KP_TOOL_POLYGON_H

#include "kpToolPolygonalBase.h"

class kpToolPolygon : public kpToolPolygonalBase
{
    Q_OBJECT

public:
    kpToolPolygon(kpToolEnvironment *environ, QObject *parent);
    ~kpToolPolygon() override;

private:
    QString haventBegunShapeUserMessage() const override;

public:
    void begin() override;
    void end() override;

protected:
    kpColor drawingBackgroundColor() const override;

public:
    void endDraw(const QPoint &, const QRect &) override;

private:
    struct kpToolPolygonPrivate *d;
};

#endif // KP_TOOL_POLYGON_H
