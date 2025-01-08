
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_LINE_WIDTH_H
#define KP_TOOL_WIDGET_LINE_WIDTH_H

#include "kpToolWidgetBase.h"

class kpToolWidgetLineWidth : public kpToolWidgetBase
{
    Q_OBJECT

public:
    kpToolWidgetLineWidth(QWidget *parent, const QString &name);
    ~kpToolWidgetLineWidth() override;

    int lineWidth() const;

Q_SIGNALS:
    void lineWidthChanged(int width);

protected Q_SLOTS:
    bool setSelected(int row, int col, bool saveAsDefault) override;
};

#endif // KP_TOOL_WIDGET_LINE_WIDTH_H
