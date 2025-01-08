
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_FILL_STYLE_H
#define KP_TOOL_WIDGET_FILL_STYLE_H

#include "kpToolWidgetBase.h"

class QPixmap;

class kpColor;

class kpToolWidgetFillStyle : public kpToolWidgetBase
{
    Q_OBJECT

public:
    kpToolWidgetFillStyle(QWidget *parent, const QString &name);
    ~kpToolWidgetFillStyle() override;

    enum FillStyle {
        NoFill,
        FillWithBackground,
        FillWithForeground,
        FillStyleNum /* not (a valid FillStyle) */
    };

private:
    QPixmap fillStylePixmap(FillStyle fs, int width, int height);
    QString fillStyleName(FillStyle fs) const;

public:
    FillStyle fillStyle() const;

    // Returns the actual fill color.
    // e.g. "FillWithBackground" fillStyle() -> <backgroundColor>,
    //      "FillWithForeground" fillStyle() -> <foregroundColor>.
    kpColor drawingBackgroundColor(const kpColor &foregroundColor, const kpColor &backgroundColor) const;

Q_SIGNALS:
    void fillStyleChanged(kpToolWidgetFillStyle::FillStyle fillStyle);

protected Q_SLOTS:
    bool setSelected(int row, int col, bool saveAsDefault) override;
};

#endif // KP_TOOL_WIDGET_FILL_STYLE_H
