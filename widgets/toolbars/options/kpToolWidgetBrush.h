
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   
   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef KP_TOOL_WIDGET_BRUSH_H
#define KP_TOOL_WIDGET_BRUSH_H

#include "kpToolWidgetBase.h"
#include "imagelib/kpColor.h"
#include "layers/tempImage/kpTempImage.h"

#include <QPixmap>


class kpToolWidgetBrush : public kpToolWidgetBase
{
Q_OBJECT

public:
    kpToolWidgetBrush (QWidget *parent, const QString &name);
    ~kpToolWidgetBrush () override;

private:
    QString brushName (int shape, int whichSize) const;

public:
    int brushSize () const;
    bool brushIsDiagonalLine () const;

    struct DrawPackage
    {
        int row;
        int col;
        kpColor color;
    };

    // Call the function returned by <drawFunction> to render the current
    // brush onto an image/document, in <color>.  Pass the pointer returned by
    // <drawFunctionData> to it.
    //
    // TODO: change function + data -> object
    kpTempImage::UserFunctionType drawFunction () const;

    static DrawPackage drawFunctionDataForRowCol (const kpColor &color,
        int row, int col);
    DrawPackage drawFunctionData (const kpColor &color) const;

signals:
    void brushChanged ();

protected slots:
    bool setSelected (int row, int col, bool saveAsDefault) override;
};


#endif  // KP_TOOL_WIDGET_BRUSH_H
