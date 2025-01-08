
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_ERASER_SIZE_H
#define KP_TOOL_WIDGET_ERASER_SIZE_H

#include "imagelib/kpColor.h"
#include "kpToolWidgetBase.h"
#include "layers/tempImage/kpTempImage.h"

class kpColor;

class kpToolWidgetEraserSize : public kpToolWidgetBase
{
    Q_OBJECT

public:
    kpToolWidgetEraserSize(QWidget *parent, const QString &name);
    ~kpToolWidgetEraserSize() override;

    int eraserSize() const;

    struct DrawPackage {
        int selected;
        kpColor color;
    };

    // Call the function returned by <drawFunction> to render the current
    // brush onto an image/document, in <color>.  Pass the pointer returned by
    // <drawFunctionData> to it.
    //
    // <drawCursorFunction> is to same as <drawFunction> but adds a black
    // border suitable as a cursor only.
    //
    // TODO: change function + data -> object
    kpTempImage::UserFunctionType drawFunction() const;
    kpTempImage::UserFunctionType drawCursorFunction() const;

    static DrawPackage drawFunctionDataForSelected(const kpColor &color, int selectedIndex);
    DrawPackage drawFunctionData(const kpColor &color) const;

Q_SIGNALS:
    void eraserSizeChanged(int size);

protected Q_SLOTS:
    bool setSelected(int row, int col, bool saveAsDefault) override;
};

#endif // KP_TOOL_WIDGET_ERASER_SIZE_H
