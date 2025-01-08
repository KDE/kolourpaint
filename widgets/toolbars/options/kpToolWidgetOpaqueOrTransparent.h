
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
