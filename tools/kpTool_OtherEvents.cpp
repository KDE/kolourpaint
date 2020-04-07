
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

//
// Tool reaction to all remaining events.
//
// 1. View events
// 2. Non-view events
//



#include "tools/kpTool.h"
#include "kpToolPrivate.h"

#include "kpLogCategories.h"

#include "imagelib/kpColor.h"

#include <QEvent>

//---------------------------------------------------------------------

//
// 1. View Events
//

bool kpTool::viewEvent (QEvent *e)
{
    qCDebug(kpLogTools) << "kpTool<" << objectName ()
              << "," << this << ">::viewEvent(type=" << e->type ()
              << ") returning false" << endl;

    // Don't handle.
    return false;
}

//---------------------------------------------------------------------

void kpTool::focusInEvent (QFocusEvent *)
{
}

//---------------------------------------------------------------------

void kpTool::focusOutEvent (QFocusEvent *)
{
#if 0
    qCDebug(kpLogTools) << "kpTool::focusOutEvent() beganDraw=" << d->beganDraw;
#endif

    if (d->beganDraw) {
        endDrawInternal (d->currentPoint, normalizedRect ());
    }
}

//---------------------------------------------------------------------

void kpTool::enterEvent (QEvent *)
{
    qCDebug(kpLogTools) << "kpTool::enterEvent() beganDraw=" << d->beganDraw;
}

//---------------------------------------------------------------------

void kpTool::leaveEvent (QEvent *)
{
    qCDebug(kpLogTools) << "kpTool::leaveEvent() beganDraw=" << d->beganDraw;

    // if we haven't started drawing (e.g. dragging a rectangle)...
    if (!d->beganDraw)
    {
        d->currentPoint = KP_INVALID_POINT;
        d->currentViewPoint = KP_INVALID_POINT;
        hover (d->currentPoint);
    }
}

//---------------------------------------------------------------------
//
// 2. Non-view events
//

void kpTool::slotColorsSwappedInternal (const kpColor &newForegroundColor,
                                        const kpColor &newBackgroundColor)
{
    if (careAboutColorsSwapped ())
    {
        slotColorsSwapped (newForegroundColor, newBackgroundColor);
        d->ignoreColorSignals = 2;
    }
    else {
        d->ignoreColorSignals = 0;
    }
}

//---------------------------------------------------------------------

void kpTool::slotForegroundColorChangedInternal (const kpColor &color)
{
    if (d->ignoreColorSignals > 0)
    {
        qCDebug(kpLogTools) << "kpTool::slotForegroundColorChangedInternal() ignoreColorSignals=" << d->ignoreColorSignals;

        d->ignoreColorSignals--;
        return;
    }

    slotForegroundColorChanged (color);
}

//---------------------------------------------------------------------

void kpTool::slotBackgroundColorChangedInternal (const kpColor &color)
{
    if (d->ignoreColorSignals > 0)
    {
        qCDebug(kpLogTools) << "kpTool::slotBackgroundColorChangedInternal() ignoreColorSignals=" << d->ignoreColorSignals;

        d->ignoreColorSignals--;
        return;
    }

    slotBackgroundColorChanged (color);
}

//---------------------------------------------------------------------

void kpTool::slotColorSimilarityChangedInternal (double similarity, int processedSimilarity)
{
    slotColorSimilarityChanged (similarity, processedSimilarity);
}

//---------------------------------------------------------------------
