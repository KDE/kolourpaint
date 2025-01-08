
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_BRUSH_H
#define KP_TOOL_WIDGET_BRUSH_H

#include "imagelib/kpColor.h"
#include "kpToolWidgetBase.h"
#include "layers/tempImage/kpTempImage.h"

class kpToolWidgetBrush : public kpToolWidgetBase
{
    Q_OBJECT

public:
    kpToolWidgetBrush(QWidget *parent, const QString &name);
    ~kpToolWidgetBrush() override;

private:
    QString brushName(int shape, int whichSize) const;

public:
    int brushSize() const;
    bool brushIsDiagonalLine() const;

    struct DrawPackage {
        int row;
        int col;
        kpColor color;
    };

    // Call the function returned by <drawFunction> to render the current
    // brush onto an image/document, in <color>.  Pass the pointer returned by
    // <drawFunctionData> to it.
    //
    // TODO: change function + data -> object
    kpTempImage::UserFunctionType drawFunction() const;

    static DrawPackage drawFunctionDataForRowCol(const kpColor &color, int row, int col);
    DrawPackage drawFunctionData(const kpColor &color) const;

Q_SIGNALS:
    void brushChanged();

protected Q_SLOTS:
    bool setSelected(int row, int col, bool saveAsDefault) override;
};

#endif // KP_TOOL_WIDGET_BRUSH_H
