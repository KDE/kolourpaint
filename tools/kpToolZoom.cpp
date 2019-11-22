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


#define DEBUG_KP_TOOL_ZOOM 0


#include "kpToolZoom.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "pixmapfx/kpPixmapFX.h"
#include "generic/kpSetOverrideCursorSaver.h"
#include "layers/tempImage/kpTempImage.h"
#include "environments/tools/kpToolEnvironment.h"
#include "tools/kpToolAction.h"
#include "views/manager/kpViewManager.h"
#include "kpLogCategories.h"

#include <QWidget>

#include <KLocalizedString>

//---------------------------------------------------------------------

struct DrawZoomRectPackage
{
    QRect normalizedRect;
};

static void DrawZoomRect (kpImage *destImage,
        const QPoint &topLeft,
        void *userData)
{
    auto *pack = static_cast <DrawZoomRectPackage *> (userData);

    kpPixmapFX::drawStippleRect(destImage,
        topLeft.x (), topLeft.y (), pack->normalizedRect.width (), pack->normalizedRect.height (),
        kpColor::Yellow,
        kpColor::Green);
}


struct kpToolZoomPrivate
{
    bool dragHasBegun{}, dragCompleted{};
    DrawZoomRectPackage drawPackage;
};

kpToolZoom::kpToolZoom (kpToolEnvironment *environ, QWidget *parent)
    : kpTool (i18n ("Zoom"), i18n ("Zooms in and out of the image"),
              Qt::Key_Z,
              environ, parent, QStringLiteral("tool_zoom")),
      d (new kpToolZoomPrivate ())
{
  // different from objectName()
  action()->setIcon(QIcon::fromTheme(QStringLiteral("zoom-original")));
}

//---------------------------------------------------------------------

kpToolZoom::~kpToolZoom ()
{
    delete d;
}

//---------------------------------------------------------------------
// public virtual [base kpTool]

bool kpToolZoom::returnToPreviousToolAfterEndDraw () const
{
    // If the user clicks to zoom in or out, s/he generally wants to click
    // some more to get the exact zoom level wanted.
    //
    // However, if they drag out a rectangle to zoom into a particular area,
    // they probably don't need to do any further zooming so we can return
    // them to their previous tool.
    //
    // Note that if they cancel a drag (cancelShape()), we do _not_ return
    // them to their previous tool, unlike the Color Picker.  This is because
    // cancelling a drag generally means that the user got the top-left of
    // the drag wrong and wants to try a different top-left.  In contrast,
    // with the Color Picket, if you've made a mistake while pressing the
    // mouse, you can just keep holding down the mouse and drag to the intended
    // color -- a cancel with a Color Picker really means "I've decided not
    // to pick another color after all", not "I got the start of the drag wrong"
    // because you can correct that drag.
    return d->dragCompleted;
}


// private
QString kpToolZoom::haventBegunDrawUserMessage () const
{
    return i18n ("Click to zoom in/out or left drag to zoom into a specific area.");
}


// public virtual [base kpTool]
void kpToolZoom::begin ()
{
    viewManager ()->setCursor (Qt::CrossCursor);

    setUserMessage (haventBegunDrawUserMessage ());
}

// public virtual [base kpTool]
void kpToolZoom::end ()
{
    viewManager ()->unsetCursor ();
}


// public virtual [base kpTool]
void kpToolZoom::globalDraw ()
{
#if DEBUG_KP_TOOL_ZOOM
    qCDebug(kpLogTools) << "CALL";
#endif
    environ ()->fitToPage ();
}


// public virtual [base kpTool]
void kpToolZoom::beginDraw ()
{
    d->dragHasBegun = false;
    d->dragCompleted = false;

    setUserMessage (cancelUserMessage ());
}

// public virtual [base kpTool]
void kpToolZoom::draw (const QPoint &thisPoint, const QPoint &, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_ZOOM
    qCDebug(kpLogTools) << "kpToomZoom::draw() currentPoint=" << currentPoint ()
              << " lastPoint=" << lastPoint ()
              << endl;
#endif

    // TODO: Need accidental drag detection from selection tool (when dragging
    //       out new selection)

    if (!d->dragHasBegun)
    {
        if (thisPoint == startPoint ()) {
            return;
        }

        // Left mouse drags select an area to zoom into.
        // However, it wouldn't make sense to select an area to "zoom out of"
        // (using the right mouse button).  Therefore, make RMB drags do the
        // same as RMB clicks i.e. a simple zoom out, with no "area" to worry
        // about.
        if (mouseButton () == 1/*RMB*/) {
            return;
        }

        d->dragHasBegun = true;
    }


    d->drawPackage.normalizedRect = normalizedRect;

    kpTempImage newTempImage (false/*always display*/,
        normalizedRect.topLeft (),
        &::DrawZoomRect, &d->drawPackage,
        normalizedRect.width (), normalizedRect.height ());

    viewManager ()->setFastUpdates ();
    {
        viewManager ()->setTempImage (newTempImage);
    }
    viewManager ()->restoreFastUpdates ();
}

// public virtual [base kpTool]
void kpToolZoom::cancelShape ()
{
    viewManager ()->invalidateTempImage ();

    // LOREFACTOR: A lot of tools use this - push up to kpTool?
    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

// public virtual [base kpTool]
void kpToolZoom::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// public virtual [base kpTool]
void kpToolZoom::endDraw (const QPoint &, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_ZOOM
    qCDebug(kpLogTools) << "kpToolZoom::endDraw(rect=" << normalizedRect << ")"
        << " dragHasBegun=" << d->dragHasBegun << endl;
#endif

    // TODO: This cursor doesn't stay on for long enough because zooming uses
    //       event loop tricks.
    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);

    viewManager ()->invalidateTempImage ();

    // Click?
    if (!d->dragHasBegun)
    {
        if (mouseButton () == 0/*LMB*/) {
            environ ()->zoomIn (true/*center under cursor*/);
        }
        else {
            environ ()->zoomOut (false/*don't center under cursor - as is
                                        confusing behaviour when zooming
                                        out*/);
        }
    }
    // Drag?
    else if (normalizedRect.isValid())
    {
        environ ()->zoomToRect (
            normalizedRect,
            false/*don't account for grips*/,
            true/*care about width*/, true/*care about height*/);

        d->dragCompleted = true;
    }
}


