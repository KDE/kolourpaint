
// REFACTOR: Rename to kpAbstractFlowImageTool, and use kpImage instead of QPixmap

/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_FLOW_PIXMAP_BASE_H
#define KP_TOOL_FLOW_PIXMAP_BASE_H

#include "kpToolFlowBase.h"

/**
 * @short Abstract base call for all continuous tools that draw pixmaps
 * (e.g. Brush, Eraser).
 *
 * @author Clarence Dang <dang@kde.org>
 */
class kpToolFlowPixmapBase : public kpToolFlowBase
{
    Q_OBJECT

public:
    kpToolFlowPixmapBase(const QString &text, const QString &description, int key, kpToolEnvironment *environ, QObject *parent, const QString &name);

protected:
    QRect drawLine(const QPoint &thisPoint, const QPoint &lastPoint) override;
};

#endif // KP_TOOL_FLOW_PIXMAP_BASE_H
