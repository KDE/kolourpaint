
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT_H
#define KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT_H

#include "kpToolWidgetBase.h"

class kpToolWidgetOpaqueOrTransparent : public kpToolWidgetBase
{
    Q_OBJECT

public:
    kpToolWidgetOpaqueOrTransparent(QWidget *parent, const QString &name);
    ~kpToolWidgetOpaqueOrTransparent() override;

    bool isOpaque() const;
    bool isTransparent() const;
    void setOpaque(bool yes = true);
    void setTransparent(bool yes = true);

Q_SIGNALS:
    void isOpaqueChanged(bool isOpaque);

protected Q_SLOTS:
    bool setSelected(int row, int col, bool saveAsDefault) override;
};

#endif // KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT_H
