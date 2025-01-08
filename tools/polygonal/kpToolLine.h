
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_LINE_H
#define KP_TOOL_LINE_H

#include "kpToolPolygonalBase.h"

class kpToolLine : public kpToolPolygonalBase
{
    Q_OBJECT

public:
    kpToolLine(kpToolEnvironment *environ, QObject *parent);

private:
    QString haventBegunShapeUserMessage() const override;

public:
    void endDraw(const QPoint &, const QRect &) override;
};

#endif // KP_TOOL_LINE_H
