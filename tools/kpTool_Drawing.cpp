
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
// Tool methods for drawing shapes (subclasses reimplement most of these).
//


#define DEBUG_KP_TOOL 0


#include "tools/kpTool.h"
#include "kpToolPrivate.h"

#include <QApplication>

#include "kpLogCategories.h"

#include "environments/tools/kpToolEnvironment.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "imagelib/kpPainter.h"

#undef environ  // macro on win32

//---------------------------------------------------------------------

// protected
int kpTool::mouseButton () const
{
    return d->mouseButton;
}

//---------------------------------------------------------------------

// protected
bool kpTool::shiftPressed () const
{
    return d->shiftPressed;
}

//---------------------------------------------------------------------

// protected
bool kpTool::controlPressed () const
{
    return d->controlPressed;
}

//---------------------------------------------------------------------

// protected
bool kpTool::altPressed () const
{
    return d->altPressed;
}


// protected
QPoint kpTool::startPoint () const
{
    return d->startPoint;
}

//---------------------------------------------------------------------

// protected
QPoint kpTool::currentPoint () const
{
    // TODO: Q_ASSERT (hasBegun()) and similar in other accessors.
    //       We currently violate these kinds of invariants.
    return d->currentPoint;
}

//---------------------------------------------------------------------

// protected
QPoint kpTool::currentViewPoint () const
{
    return d->currentViewPoint;
}

//---------------------------------------------------------------------

// protected
QRect kpTool::normalizedRect () const
{
    return kpPainter::normalizedRect(d->startPoint, d->currentPoint);
}

//---------------------------------------------------------------------

// protected
QPoint kpTool::lastPoint () const
{
    return d->lastPoint;
}

//---------------------------------------------------------------------

// protected
kpView *kpTool::viewUnderStartPoint () const
{
    return d->viewUnderStartPoint;
}

//---------------------------------------------------------------------

// protected
kpView *kpTool::viewUnderCursor () const
{
    kpViewManager *vm = viewManager ();
    return vm ? vm->viewUnderCursor () : nullptr;
}

//---------------------------------------------------------------------

void kpTool::beginInternal ()
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool::beginInternal()";
#endif

    if (!d->began)
    {
        // clear leftover statusbar messages
        setUserMessage ();
        d->currentPoint = calculateCurrentPoint ();
        d->currentViewPoint = calculateCurrentPoint (false/*view point*/);
        setUserShapePoints (d->currentPoint);

        // TODO: Audit all the code in this file - states like "d->began" &
        //       "d->beganDraw" should be set before calling user func.
        //       Also, d->currentPoint should be more frequently initialised.

        // call user virtual func
        begin ();

        // we've starting using the tool...
        d->began = true;

        // but we haven't started drawing with it
        d->beganDraw = false;


        uint keyState = QApplication::keyboardModifiers ();

        d->shiftPressed = (keyState & Qt::ShiftModifier);
        d->controlPressed = (keyState & Qt::ControlModifier);

        // TODO: Can't do much about ALT - unless it's always KApplication::Modifier1?
        //       Ditto for everywhere else where I set SHIFT & CTRL but not alt.
        //       COMPAT: Later: This is now supported by Qt.
        d->altPressed = false;
    }
}

//---------------------------------------------------------------------

void kpTool::endInternal ()
{
    if (d->began)
    {
        // before we can stop using the tool, we must stop the current drawing operation (if any)
        if (hasBegunShape ()) {
            endShapeInternal (d->currentPoint, normalizedRect ());
        }

        // call user virtual func
        end ();

        // clear leftover statusbar messages
        setUserMessage ();
        setUserShapePoints (calculateCurrentPoint ());

        // we've stopped using the tool...
        d->began = false;

        // and so we can't be drawing with it
        d->beganDraw = false;

        d->environ->hideAllToolWidgets ();
    }
}

//---------------------------------------------------------------------

// virtual
void kpTool::begin ()
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool::begin() base implementation";
#endif
}

//---------------------------------------------------------------------

// virtual
void kpTool::end ()
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool::end() base implementation";
#endif
}

//---------------------------------------------------------------------


bool kpTool::hasBegun () const { return d->began; }

//---------------------------------------------------------------------

bool kpTool::hasBegunDraw () const { return d->beganDraw; }

//---------------------------------------------------------------------

// virtual
bool kpTool::hasBegunShape () const { return hasBegunDraw (); }

//---------------------------------------------------------------------


void kpTool::beginDrawInternal ()
{
    if (!d->beganDraw)
    {
        beginDraw ();

        d->beganDraw = true;
        emit beganDraw (d->currentPoint);
    }
}

//---------------------------------------------------------------------

// virtual
void kpTool::beginDraw ()
{
}

//---------------------------------------------------------------------

// virtual
void kpTool::hover (const QPoint &point)
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool::hover" << point
               << " base implementation";
#endif

    setUserShapePoints (point);
}

//---------------------------------------------------------------------

// virtual
void kpTool::globalDraw ()
{
}

//---------------------------------------------------------------------

// virtual
void kpTool::reselect ()
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool::reselect() base implementation";
#endif
}

//---------------------------------------------------------------------


// virtual
void kpTool::draw (const QPoint &, const QPoint &, const QRect &)
{
}

//---------------------------------------------------------------------

// private
void kpTool::drawInternal ()
{
    draw (d->currentPoint, d->lastPoint, normalizedRect ());
}

//---------------------------------------------------------------------


// also called by kpView
void kpTool::cancelShapeInternal ()
{
    if (hasBegunShape ())
    {
        d->beganDraw = false;
        cancelShape ();
        d->viewUnderStartPoint = nullptr;

        emit cancelledShape (viewUnderCursor () ? d->currentPoint : KP_INVALID_POINT);

        if (viewUnderCursor ()) {
            hover (d->currentPoint);
        }
        else
        {
            d->currentPoint = KP_INVALID_POINT;
            d->currentViewPoint = KP_INVALID_POINT;
            hover (d->currentPoint);
        }

        if (returnToPreviousToolAfterEndDraw ())
        {
            d->environ->selectPreviousTool ();
        }
    }
}

//---------------------------------------------------------------------

// virtual
void kpTool::cancelShape ()
{
    qCWarning(kpLogTools) << "Tool cannot cancel operation!" ;
}

//---------------------------------------------------------------------

void kpTool::releasedAllButtons ()
{
}

//---------------------------------------------------------------------

void kpTool::endDrawInternal (const QPoint &thisPoint, const QRect &normalizedRect,
                              bool wantEndShape)
{
#if DEBUG_KP_TOOL && 1
    qCDebug(kpLogTools) << "kpTool::endDrawInternal() wantEndShape=" << wantEndShape;
#endif

    if (wantEndShape && !hasBegunShape ()) {
        return;
    }

    if (!wantEndShape && !hasBegunDraw ()) {
        return;
    }

    d->beganDraw = false;

    if (wantEndShape)
    {
    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "\tcalling endShape()";
    #endif
        endShape (thisPoint, normalizedRect);
    }
    else
    {
    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "\tcalling endDraw()";
    #endif
        endDraw (thisPoint, normalizedRect);
    }
    d->viewUnderStartPoint = nullptr;

    emit endedDraw (d->currentPoint);
    if (viewUnderCursor ()) {
        hover (d->currentPoint);
    }
    else
    {
        d->currentPoint = KP_INVALID_POINT;
        d->currentViewPoint = KP_INVALID_POINT;
        hover (d->currentPoint);
    }

    if (returnToPreviousToolAfterEndDraw ())
    {
        d->environ->selectPreviousTool ();
    }
}

//---------------------------------------------------------------------

// private
void kpTool::endShapeInternal (const QPoint &thisPoint, const QRect &normalizedRect)
{
    endDrawInternal (thisPoint, normalizedRect, true/*end shape*/);
}

//---------------------------------------------------------------------

// virtual
void kpTool::endDraw (const QPoint &, const QRect &)
{
}

//---------------------------------------------------------------------
