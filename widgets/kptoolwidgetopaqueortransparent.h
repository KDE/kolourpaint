
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#ifndef __kp_tool_widget_opaque_or_transparent_h__
#define __kp_tool_widget_opaque_or_transparent_h__


#include <kptoolwidgetbase.h>

class kpToolWidgetOpaqueOrTransparent : public kpToolWidgetBase
{
Q_OBJECT

public:
    kpToolWidgetOpaqueOrTransparent (QWidget *parent, const char *name);
    virtual ~kpToolWidgetOpaqueOrTransparent ();

    bool isOpaque () const;
    bool isTransparent () const;
    void setOpaque (bool yes = true);
    void setTransparent (bool yes = true);

signals:
    void isOpaqueChanged (bool isOpaque_);

protected slots:
    virtual bool setSelected (int row, int col, bool saveAsDefault);
};


#endif  // kp_tool_widget_opaque_or_transparent_h__
