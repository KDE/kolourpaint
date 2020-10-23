
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


#define DEBUG_KP_TOOL_FLOW_BASE 0

#include "kpToolFlowBase.h"

#include <cstdlib>

#include <QImage>
#include <QPainter>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "imagelib/kpColor.h"
#include "commands/kpCommandHistory.h"
#include "cursors/kpCursorProvider.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "imagelib/kpImage.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "environments/tools/kpToolEnvironment.h"
#include "commands/tools/flow/kpToolFlowCommand.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetBrush.h"
#include "widgets/toolbars/options/kpToolWidgetEraserSize.h"
#include "views/manager/kpViewManager.h"

//---------------------------------------------------------------------

struct kpToolFlowBasePrivate
{
    kpToolWidgetBrush *toolWidgetBrush{};
    kpToolWidgetEraserSize *toolWidgetEraserSize{};


    //
    // Cursor and Brush Data
    // (must be zero if unused)
    //

        kpTempImage::UserFunctionType brushDrawFunc{}, cursorDrawFunc{};

        // Can't use union since package types contain fields requiring
        // constructors.
        kpToolWidgetBrush::DrawPackage brushDrawPackageForMouseButton [2];
        kpToolWidgetEraserSize::DrawPackage eraserDrawPackageForMouseButton [2];

        // Each element points to one of the above (both elements from the same
        // array).
        void *drawPackageForMouseButton [2]{};

        int brushWidth{}, brushHeight{};
        int cursorWidth{}, cursorHeight{};

        bool brushIsDiagonalLine{};


    kpToolFlowCommand *currentCommand{};
};

//---------------------------------------------------------------------

kpToolFlowBase::kpToolFlowBase (const QString &text, const QString &description,
        int key,
        kpToolEnvironment *environ, QObject *parent, const QString &name)

    : kpTool (text, description, key, environ, parent, name),
      d (new kpToolFlowBasePrivate ())
{
    d->toolWidgetBrush = nullptr;
    d->toolWidgetEraserSize = nullptr;

    clearBrushCursorData ();

    d->currentCommand = nullptr;
}

//---------------------------------------------------------------------

kpToolFlowBase::~kpToolFlowBase ()
{
    delete d;
}

//---------------------------------------------------------------------

// private
void kpToolFlowBase::clearBrushCursorData ()
{
    d->brushDrawFunc = d->cursorDrawFunc = nullptr;

    memset (&d->brushDrawPackageForMouseButton,
        0,
        sizeof (d->brushDrawPackageForMouseButton));
    memset (&d->eraserDrawPackageForMouseButton,
        0,
        sizeof (d->eraserDrawPackageForMouseButton));

    memset (&d->drawPackageForMouseButton,
        0,
        sizeof (d->drawPackageForMouseButton));

    d->brushWidth = d->brushHeight = 0;
    d->cursorWidth = d->cursorHeight = 0;

    d->brushIsDiagonalLine = false;
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::begin ()
{
    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

    // TODO: Bad smell.  Mutually exclusive.  Use inheritance.
    if (haveSquareBrushes ())
    {
        d->toolWidgetEraserSize = tb->toolWidgetEraserSize ();
        connect (d->toolWidgetEraserSize, &kpToolWidgetEraserSize::eraserSizeChanged,
                 this, &kpToolFlowBase::updateBrushAndCursor);
        d->toolWidgetEraserSize->show ();

        updateBrushAndCursor ();

        viewManager ()->setCursor (kpCursorProvider::lightCross ());
    }
    else if (haveDiverseBrushes ())
    {
        d->toolWidgetBrush = tb->toolWidgetBrush ();
        connect (d->toolWidgetBrush, &kpToolWidgetBrush::brushChanged,
                 this, &kpToolFlowBase::updateBrushAndCursor);
        d->toolWidgetBrush->show ();

        updateBrushAndCursor ();

        viewManager ()->setCursor (kpCursorProvider::lightCross ());
    }

    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::end ()
{
    if (d->toolWidgetEraserSize)
    {
        disconnect (d->toolWidgetEraserSize, &kpToolWidgetEraserSize::eraserSizeChanged,
                    this, &kpToolFlowBase::updateBrushAndCursor);
        d->toolWidgetEraserSize = nullptr;
    }
    else if (d->toolWidgetBrush)
    {
        disconnect (d->toolWidgetBrush, &kpToolWidgetBrush::brushChanged,
                    this, &kpToolFlowBase::updateBrushAndCursor);
        d->toolWidgetBrush = nullptr;
    }

    kpViewManager *vm = viewManager ();
    Q_ASSERT (vm);

    if (vm->tempImage () && vm->tempImage ()->isBrush ()) {
        vm->invalidateTempImage ();
    }

    if (haveAnyBrushes ()) {
        vm->unsetCursor ();
    }

    clearBrushCursorData ();
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::beginDraw ()
{
    d->currentCommand = new kpToolFlowCommand (text (), environ ()->commandEnvironment ());

    // We normally show the brush cursor in the foreground colour but if the
    // user starts drawing in the background color, we don't want to leave
    // the brush cursor in the foreground colour -- just hide it in all cases
    // to avoid confusion.
    viewManager ()->invalidateTempImage ();

    setUserMessage (cancelUserMessage ());
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::hover (const QPoint &point)
{
#if DEBUG_KP_TOOL_FLOW_BASE && 0
    qCDebug(kpLogTools) << "kpToolFlowBase::hover(" << point << ")"
               << " hasBegun=" << hasBegun ()
               << " hasBegunDraw=" << hasBegunDraw ()
               << " cursorPixmap.isNull=" << m_cursorPixmap.isNull ()
               << endl;
#endif
    if (point != KP_INVALID_POINT && d->cursorDrawFunc)
    {
        viewManager ()->setFastUpdates ();

        viewManager ()->setTempImage (
            kpTempImage (true/*brush*/,
                hotRect ().topLeft (),
                d->cursorDrawFunc, d->drawPackageForMouseButton [0/*left button*/],
                d->cursorWidth, d->cursorHeight));

        viewManager ()->restoreFastUpdates ();
    }

    setUserShapePoints (point);
}

//---------------------------------------------------------------------

// virtual
QRect kpToolFlowBase::drawPoint (const QPoint &point)
{
    return drawLine (point, point);
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::draw (const QPoint &thisPoint, const QPoint &lastPoint, const QRect &normalizedRect)
{
    if (!/*virtual*/drawShouldProceed (thisPoint, lastPoint, normalizedRect)) {
        return;
    }

    // sync: remember to restoreFastUpdates() in all exit paths
    viewManager ()->setFastUpdates ();

    QRect dirtyRect;

    // TODO: I'm beginning to wonder this drawPoint() "optimization" actually
    //       optimises.  Is it worth the complexity?  Hence drawPoint() impl above.
    if (d->brushIsDiagonalLine ?
            currentPointCardinallyNextToLast () :
            currentPointNextToLast ())
    {
        dirtyRect = drawPoint (thisPoint);
    }
    // in reality, the system is too slow to give us all the MouseMove events
    // so we "interpolate" the missing points :)
    else
    {
        dirtyRect = drawLine (thisPoint, lastPoint);
    }

    d->currentCommand->updateBoundingRect (dirtyRect);

    viewManager ()->restoreFastUpdates ();
    setUserShapePoints (thisPoint);
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::cancelShape ()
{
    d->currentCommand->finalize ();
    d->currentCommand->cancel ();

    delete d->currentCommand;
    d->currentCommand = nullptr;

    updateBrushAndCursor ();

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

//---------------------------------------------------------------------

void kpToolFlowBase::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// virtual
void kpToolFlowBase::endDraw (const QPoint &, const QRect &)
{
    d->currentCommand->finalize ();
    environ ()->commandHistory ()->addCommand (d->currentCommand,
        false/*don't exec*/);

    // don't delete - it's up to the commandHistory
    d->currentCommand = nullptr;

    updateBrushAndCursor ();

    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// TODO: maybe the base should be virtual?
kpColor kpToolFlowBase::color (int which)
{
#if DEBUG_KP_TOOL_FLOW_BASE && 0
    qCDebug(kpLogTools) << "kpToolFlowBase::color (" << which << ")";
#endif

    // Pen & Brush
    if (!colorsAreSwapped ()) {
        return kpTool::color (which);
    }
    // only the (Color) Eraser uses the opposite color
    return kpTool::color (which ? 0 : 1);  // don't trust !0 == 1
}

//---------------------------------------------------------------------

// protected
kpTempImage::UserFunctionType kpToolFlowBase::brushDrawFunction () const
{
    return d->brushDrawFunc;
}

//---------------------------------------------------------------------

// protected
void *kpToolFlowBase::brushDrawFunctionData () const
{
    return d->drawPackageForMouseButton [mouseButton ()];
}


// protected
int kpToolFlowBase::brushWidth () const
{
    return d->brushWidth;
}

// protected
int kpToolFlowBase::brushHeight () const
{
    return d->brushHeight;
}

// protected
bool kpToolFlowBase::brushIsDiagonalLine () const
{
    return d->brushIsDiagonalLine;
}


// protected
kpToolFlowCommand *kpToolFlowBase::currentCommand () const
{
    return d->currentCommand;
}

//---------------------------------------------------------------------

// protected slot
void kpToolFlowBase::updateBrushAndCursor ()
{
#if DEBUG_KP_TOOL_FLOW_BASE && 1
    qCDebug(kpLogTools) << "kpToolFlowBase::updateBrushAndCursor()";
#endif

    if (haveSquareBrushes ())
    {
        d->brushDrawFunc = d->toolWidgetEraserSize->drawFunction ();
        d->cursorDrawFunc = d->toolWidgetEraserSize->drawCursorFunction ();

        for (int i = 0; i < 2; i++)
        {
            d->drawPackageForMouseButton [i] =
                &(d->eraserDrawPackageForMouseButton [i] =
                    d->toolWidgetEraserSize->drawFunctionData (color (i)));
        }

        d->brushWidth = d->brushHeight =
            d->cursorWidth = d->cursorHeight =
                d->toolWidgetEraserSize->eraserSize ();

        d->brushIsDiagonalLine = false;
    }
    else if (haveDiverseBrushes ())
    {
        d->brushDrawFunc = d->cursorDrawFunc = d->toolWidgetBrush->drawFunction ();

        for (int i = 0; i < 2; i++)
        {
            d->drawPackageForMouseButton [i] =
                &(d->brushDrawPackageForMouseButton [i] =
                    d->toolWidgetBrush->drawFunctionData (color (i)));
        }

        d->brushWidth = d->brushHeight =
            d->cursorWidth = d->cursorHeight =
                d->toolWidgetBrush->brushSize ();

        d->brushIsDiagonalLine = d->toolWidgetBrush->brushIsDiagonalLine ();
    }

    hover (hasBegun () ? currentPoint () : calculateCurrentPoint ());
}

//---------------------------------------------------------------------

// virtual private slot
void kpToolFlowBase::slotForegroundColorChanged (const kpColor & /*col*/)
{
#if DEBUG_KP_TOOL_FLOW_BASE
    qCDebug(kpLogTools) << "kpToolFlowBase::slotForegroundColorChanged()";
#endif

    updateBrushAndCursor ();
}

//---------------------------------------------------------------------

// virtual private slot
void kpToolFlowBase::slotBackgroundColorChanged (const kpColor & /*col*/)
{
#if DEBUG_KP_TOOL_FLOW_BASE
    qCDebug(kpLogTools) << "kpToolFlowBase::slotBackgroundColorChanged()";
#endif

    updateBrushAndCursor ();
}

//---------------------------------------------------------------------

// public static
QRect kpToolFlowBase::hotRectForMousePointAndBrushWidthHeight (
        const QPoint &mousePoint,
        int brushWidth, int brushHeight)
{
    /*
     * e.g.
     *    Width 5:
     *    0 1 2 3 4
     *        ^
     *        |
     *      Center
     */
    return  {mousePoint.x () - brushWidth / 2,
                mousePoint.y () - brushHeight / 2,
                brushWidth, brushHeight};
}

//---------------------------------------------------------------------

// protected
QRect kpToolFlowBase::hotRect () const
{
    return hotRectForMousePointAndBrushWidthHeight (currentPoint (),
        d->brushWidth, d->brushHeight);
}

//---------------------------------------------------------------------

