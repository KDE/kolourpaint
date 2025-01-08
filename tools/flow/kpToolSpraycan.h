
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_SPRAYCAN_H
#define KP_TOOL_SPRAYCAN_H

#include "kpToolFlowBase.h"

class QPoint;
class QRect;
class QString;
class QTimer;

class kpToolWidgetSpraycanSize;

class kpToolSpraycan : public kpToolFlowBase
{
    Q_OBJECT

public:
    kpToolSpraycan(kpToolEnvironment *environ, QObject *parent);

protected:
    QString haventBegunDrawUserMessage() const override;

public:
    void begin() override;
    void end() override;

public:
    void beginDraw() override;

protected:
    // (ASSUMPTION: <probability> is between 0.0 and 1.0 inclusive)
    QRect drawLineWithProbability(const QPoint &thisPoint, const QPoint &lastPoint, double probability);

public:
    QRect drawPoint(const QPoint &point) override;
    QRect drawLine(const QPoint &thisPoint, const QPoint &lastPoint) override;
    void cancelShape() override;
    void endDraw(const QPoint &thisPoint, const QRect &normalizedRect) override;

protected Q_SLOTS:
    void timeoutDraw();

protected:
    int spraycanSize() const;
protected Q_SLOTS:
    void slotSpraycanSizeChanged(int size);

protected:
    QTimer *m_timer;
    kpToolWidgetSpraycanSize *m_toolWidgetSpraycanSize;
};

#endif // KP_TOOL_SPRAYCAN_H
