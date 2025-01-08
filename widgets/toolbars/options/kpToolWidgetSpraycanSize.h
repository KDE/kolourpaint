
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_SPRAYCAN_SIZE_H
#define KP_TOOL_WIDGET_SPRAYCAN_SIZE_H

#include "kpToolWidgetBase.h"

class kpToolWidgetSpraycanSize : public kpToolWidgetBase
{
    Q_OBJECT

public:
    kpToolWidgetSpraycanSize(QWidget *parent, const QString &name);
    ~kpToolWidgetSpraycanSize() override;

    int spraycanSize() const;

Q_SIGNALS:
    void spraycanSizeChanged(int size);

protected Q_SLOTS:
    bool setSelected(int row, int col, bool saveAsDefault) override;
};

#endif // KP_TOOL_WIDGET_SPRAYCAN_SIZE_H
