
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

//
// Tool reaction to all remaining events.
//
// 1. View events
// 2. Non-view events
//

#define DEBUG_KP_TOOL 0

#include "kpToolPrivate.h"
#include "tools/kpTool.h"

#include "kpLogCategories.h"

#include "imagelib/kpColor.h"

//---------------------------------------------------------------------

//
// 1. View Events
//

bool kpTool::viewEvent(QEvent *e)
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool<" << objectName() << "," << this << ">::viewEvent(type=" << e->type() << ") returning false" << endl;
#else
    (void)e;
#endif

    // Don't handle.
    return false;
}

//---------------------------------------------------------------------

void kpTool::focusInEvent(QFocusEvent *)
{
}

//---------------------------------------------------------------------

void kpTool::focusOutEvent(QFocusEvent *)
{
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::focusOutEvent() beganDraw=" << d->beganDraw;
#endif

    if (d->beganDraw) {
        endDrawInternal(d->currentPoint, normalizedRect());
    }
}

//---------------------------------------------------------------------

void kpTool::enterEvent(QEvent *)
{
#if DEBUG_KP_TOOL && 1
    qCDebug(kpLogTools) << "kpTool::enterEvent() beganDraw=" << d->beganDraw;
#endif
}

//---------------------------------------------------------------------

void kpTool::leaveEvent(QEvent *)
{
#if DEBUG_KP_TOOL && 1
    qCDebug(kpLogTools) << "kpTool::leaveEvent() beganDraw=" << d->beganDraw;
#endif

    // if we haven't started drawing (e.g. dragging a rectangle)...
    if (!d->beganDraw) {
        d->currentPoint = KP_INVALID_POINT;
        d->currentViewPoint = KP_INVALID_POINT;
        hover(d->currentPoint);
    }
}

//---------------------------------------------------------------------
//
// 2. Non-view events
//

void kpTool::slotColorsSwappedInternal(const kpColor &newForegroundColor, const kpColor &newBackgroundColor)
{
    if (careAboutColorsSwapped()) {
        slotColorsSwapped(newForegroundColor, newBackgroundColor);
        d->ignoreColorSignals = 2;
    } else {
        d->ignoreColorSignals = 0;
    }
}

//---------------------------------------------------------------------

void kpTool::slotForegroundColorChangedInternal(const kpColor &color)
{
    if (d->ignoreColorSignals > 0) {
#if DEBUG_KP_TOOL && 1
        qCDebug(kpLogTools) << "kpTool::slotForegroundColorChangedInternal() ignoreColorSignals=" << d->ignoreColorSignals;
#endif
        d->ignoreColorSignals--;
        return;
    }

    slotForegroundColorChanged(color);
}

//---------------------------------------------------------------------

void kpTool::slotBackgroundColorChangedInternal(const kpColor &color)
{
    if (d->ignoreColorSignals > 0) {
#if DEBUG_KP_TOOL && 1
        qCDebug(kpLogTools) << "kpTool::slotBackgroundColorChangedInternal() ignoreColorSignals=" << d->ignoreColorSignals;
#endif
        d->ignoreColorSignals--;
        return;
    }

    slotBackgroundColorChanged(color);
}

//---------------------------------------------------------------------

void kpTool::slotColorSimilarityChangedInternal(double similarity, int processedSimilarity)
{
    slotColorSimilarityChanged(similarity, processedSimilarity);
}

//---------------------------------------------------------------------
