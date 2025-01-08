
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_ZOOM_H
#define KP_TOOL_ZOOM_H

#include "tools/kpTool.h"

class kpToolZoom : public kpTool
{
    Q_OBJECT

public:
    kpToolZoom(kpToolEnvironment *environ, QWidget *parent);
    ~kpToolZoom() override;

    bool returnToPreviousToolAfterEndDraw() const override;

private:
    QString haventBegunDrawUserMessage() const;

public:
    void begin() override;
    void end() override;

    void globalDraw() override;

    void beginDraw() override;
    void draw(const QPoint &thisPoint, const QPoint &, const QRect &) override;
    void cancelShape() override;
    void releasedAllButtons() override;
    void endDraw(const QPoint &thisPoint, const QRect &) override;

private:
    struct kpToolZoomPrivate *d;
};

#endif // KP_TOOL_ZOOM_H
