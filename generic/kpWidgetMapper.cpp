
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


#include "kpWidgetMapper.h"

#include <QPoint>
#include <QRect>
#include <QWidget>


namespace kpWidgetMapper
{


QPoint fromGlobal (const QWidget *widget, const QPoint &point)
{
    if (!widget) {
        return point;
    }

    return widget->mapFromGlobal (point);
}

QRect fromGlobal (const QWidget *widget, const QRect &rect)
{
    if (!widget || !rect.isValid ()) {
        return rect;
    }

    auto topLeft = fromGlobal (widget, rect.topLeft ());
    return  {topLeft.x (), topLeft.y (), rect.width (), rect.height ()};
}


QPoint toGlobal (const QWidget *widget, const QPoint &point)
{
    if (!widget) {
        return point;
    }

    return widget->mapToGlobal (point);
}

QRect toGlobal (const QWidget *widget, const QRect &rect)
{
    if (!widget || !rect.isValid ()) {
        return rect;
    }

    auto topLeft = toGlobal (widget, rect.topLeft ());
    return  {topLeft.x (), topLeft.y (), rect.width (), rect.height ()};
}


}  // namespace kpWidgetMapper
