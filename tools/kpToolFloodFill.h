
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_FLOOD_FILL_H
#define KP_TOOL_FLOOD_FILL_H

#include "tools/kpTool.h"

class kpToolFloodFill : public kpTool
{
    Q_OBJECT

public:
    kpToolFloodFill(kpToolEnvironment *environ, QObject *parent);
    ~kpToolFloodFill() override;

private:
    QString haventBegunDrawUserMessage() const;

public:
    void begin() override;
    void beginDraw() override;
    void draw(const QPoint &thisPoint, const QPoint &, const QRect &) override;
    void cancelShape() override;
    void releasedAllButtons() override;
    void endDraw(const QPoint &, const QRect &) override;

private:
    struct kpToolFloodFillPrivate *const d;
};

#endif // KP_TOOL_FLOOD_FILL_H
