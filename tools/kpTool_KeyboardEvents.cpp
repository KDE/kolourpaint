
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
// Tool reaction to view keyboard input.
//


#define DEBUG_KP_TOOL 0


// TODO: reduce number of includes
#include "tools/kpTool.h"
#include "kpToolPrivate.h"

#include <climits>

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QKeyEvent>

#include <KActionCollection>
#include "kpLogCategories.h"
#include <KLocalizedString>

#include "imagelib/kpColor.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "kpDefs.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpToolAction.h"
#include "environments/tools/kpToolEnvironment.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

//---------------------------------------------------------------------

void kpTool::seeIfAndHandleModifierKey (QKeyEvent *e)
{
    switch (e->key ())
    {
    case 0:
    case Qt::Key_unknown:
    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "kpTool::seeIfAndHandleModifierKey() picked up unknown key!";
    #endif
        // HACK: around Qt bug: if you hold a modifier before you start the
        //                      program and then release it over the view,
        //                      Qt reports it as the release of an unknown key
        //       Qt4 update: I don't think this happens anymore...
        // --- fall thru and update all modifiers ---

    case Qt::Key_Alt:
    case Qt::Key_Shift:
    case Qt::Key_Control:
    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "kpTool::setIfAndHandleModifierKey() accepting";
    #endif
        keyUpdateModifierState (e);

        e->accept ();
        break;
    }
}

//---------------------------------------------------------------------

// Returns in <dx> and <dy> the direction the arrow key "e->key()" is
// pointing in or (0,0) if it's not a recognised arrow key.
void kpTool::arrowKeyPressDirection (const QKeyEvent *e, int *dx, int *dy)
{
    int dxLocal = 0, dyLocal = 0;

    switch (e->key ())
    {
    case Qt::Key_Home:      dxLocal = -1;   dyLocal = -1;   break;
    case Qt::Key_Up:                        dyLocal = -1;   break;
    case Qt::Key_PageUp:    dxLocal = +1;   dyLocal = -1;   break;

    case Qt::Key_Left:      dxLocal = -1;                   break;
    case Qt::Key_Right:     dxLocal = +1;                   break;

    case Qt::Key_End:       dxLocal = -1;   dyLocal = +1;   break;
    case Qt::Key_Down:                      dyLocal = +1;   break;
    case Qt::Key_PageDown:  dxLocal = +1;   dyLocal = +1;   break;
    }

    if (dx) {
        *dx = dxLocal;
    }
    if (dy) {
        *dy = dyLocal;
    }
}

//---------------------------------------------------------------------

void kpTool::seeIfAndHandleArrowKeyPress (QKeyEvent *e)
{
    int dx, dy;

    arrowKeyPressDirection (e, &dx, &dy);
    if (dx == 0 && dy == 0) {
        return;
    }


    kpView * const view = viewUnderCursor ();
    if (!view) {
        return;
    }


    const QPoint oldPoint = view->mapFromGlobal (QCursor::pos ());
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "\toldPoint=" << oldPoint
                << " dx=" << dx << " dy=" << dy << endl;
#endif


    const int viewIncX = (dx ? qMax (1, view->zoomLevelX () / 100) * dx : 0);
    const int viewIncY = (dy ? qMax (1, view->zoomLevelY () / 100) * dy : 0);

    int newViewX = oldPoint.x () + viewIncX;
    int newViewY = oldPoint.y () + viewIncY;


#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "\tnewPoint=" << QPoint (newViewX, newViewY);
#endif

    // Make sure we really moved at least one doc point (needed due to
    // rounding error).

    if (view->transformViewToDoc (QPoint (newViewX, newViewY)) ==
        view->transformViewToDoc (oldPoint))
    {
        newViewX += viewIncX;
        newViewY += viewIncY;

    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "\tneed adjust for doc - newPoint="
                    << QPoint (newViewX, newViewY) << endl;
    #endif
    }


    // TODO: visible width/height (e.g. with scrollbars)
    const int x = qMin (qMax (newViewX, 0), view->width () - 1);
    const int y = qMin (qMax (newViewY, 0), view->height () - 1);

    // QCursor::setPos conveniently causes mouseMoveEvents
    QCursor::setPos (view->mapToGlobal (QPoint (x, y)));
    e->accept ();
}

//---------------------------------------------------------------------

bool kpTool::isDrawKey (int key)
{
    return (key == Qt::Key_Enter ||
            key == Qt::Key_Return ||
            key == Qt::Key_Insert ||
            key == Qt::Key_Clear/*Numpad 5 Key*/ ||
            key == Qt::Key_L);
}

//---------------------------------------------------------------------

void kpTool::seeIfAndHandleBeginDrawKeyPress (QKeyEvent *e)
{
    if (e->isAutoRepeat ()) {
        return;
    }

    if (!isDrawKey (e->key ())) {
        return;
    }

#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::seeIfAndHandleBeginDrawKeyPress() accept";
#endif


    // TODO: wrong for dragging lines outside of view (for e.g.)
    kpView * const view = viewUnderCursor ();
    if (!view) {
        return;
    }


    // TODO: what about the modifiers?
    QMouseEvent me (QEvent::MouseButtonPress,
                    view->mapFromGlobal (QCursor::pos ()),
                    Qt::LeftButton,
                    Qt::LeftButton/*button state after event*/,
                    Qt::NoModifier);
    mousePressEvent (&me);
    e->accept ();
}

void kpTool::seeIfAndHandleEndDrawKeyPress (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::setIfAndHandleEndDrawKeyPress() key=" << e->key ()
               << " isAutoRepeat=" << e->isAutoRepeat ()
               << " isDrawKey=" << isDrawKey (e->key ())
               << " view=" << viewUnderCursor ()
               << endl;
#endif

    if (e->isAutoRepeat ()) {
        return;
    }

    if (!isDrawKey (e->key ())) {
        return;
    }

#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::seeIfAndHandleEndDrawKeyPress() accept";
#endif


    kpView * const view = viewUnderCursor ();
    if (!view) {
        return;
    }


    // TODO: what about the modifiers?
    QMouseEvent me (QEvent::MouseButtonRelease,
                    view->mapFromGlobal (QCursor::pos ()),
                    Qt::LeftButton,
                    Qt::NoButton/*button state after event*/,
                    Qt::NoModifier);
    mouseReleaseEvent (&me);

    e->accept ();
}

//---------------------------------------------------------------------

void kpTool::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::keyPressEvent() key=" << (int *) e->key ()
              << " stateAfter: modifiers=" << (int *) (int) e->modifiers ()
              << " isAutoRep=" << e->isAutoRepeat ();
#endif

    e->ignore ();


    seeIfAndHandleModifierKey (e);
    if (e->isAccepted ()) {
        return;
    }

    seeIfAndHandleArrowKeyPress (e);
    if (e->isAccepted ()) {
        return;
    }

    seeIfAndHandleBeginDrawKeyPress (e);
    if (e->isAccepted ()) {
        return;
    }


    switch (e->key ())
    {
    case Qt::Key_Delete:
        d->environ->deleteSelection ();
        break;

    case Qt::Key_Escape:
        if (hasBegunDraw ())
        {
            cancelShapeInternal ();
            e->accept ();
        }

        break;
    }
}

//---------------------------------------------------------------------

void kpTool::keyReleaseEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::keyReleaseEvent() key=" << (int *) e->key ()
              << " stateAfter: modifiers=" << (int *) (int) e->modifiers ()
              << " isAutoRep=" << e->isAutoRepeat ();
#endif

    e->ignore ();

    seeIfAndHandleModifierKey (e);
    if (e->isAccepted ()) {
        return;
    }

    seeIfAndHandleEndDrawKeyPress (e);
    if (e->isAccepted ()) {
        return;
    }
}

//---------------------------------------------------------------------

// private
void kpTool::keyUpdateModifierState (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::keyUpdateModifierState() e->key=" << (int *) e->key ();
    qCDebug(kpLogTools) << "\tshift="
               << (e->modifiers () & Qt::ShiftModifier)
               << " control="
               << (e->modifiers () & Qt::ControlModifier)
               << " alt="
               << (e->modifiers () & Qt::AltModifier)
               << endl;
#endif
    if (e->key () & (Qt::Key_Alt | Qt::Key_Shift | Qt::Key_Control))
    {
    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "\t\tmodifier changed - use e's claims";
    #endif
        setShiftPressed (e->modifiers () & Qt::ShiftModifier);
        setControlPressed (e->modifiers () & Qt::ControlModifier);
        setAltPressed (e->modifiers () & Qt::AltModifier);
    }
    // See seeIfAndHandleModifierKey() for why this code path exists.
    else
    {
    #if DEBUG_KP_TOOL && 0
        qCDebug(kpLogTools) << "\t\tmodifiers not changed - figure out the truth";
    #endif
        const Qt::KeyboardModifiers keyState = QApplication::keyboardModifiers ();

        setShiftPressed (keyState & Qt::ShiftModifier);
        setControlPressed (keyState & Qt::ControlModifier);
        setAltPressed (keyState & Qt::AltModifier);
    }
}

//---------------------------------------------------------------------

void kpTool::notifyModifierStateChanged ()
{
    if (careAboutModifierState ())
    {
        if (d->beganDraw) {
            draw (d->currentPoint, d->lastPoint, normalizedRect ());
        }
        else
        {
            d->currentPoint = calculateCurrentPoint ();
            d->currentViewPoint = calculateCurrentPoint (false/*view point*/);
            hover (d->currentPoint);
        }
    }
}

//---------------------------------------------------------------------

void kpTool::setShiftPressed (bool pressed)
{
    if (pressed == d->shiftPressed) {
        return;
    }

    d->shiftPressed = pressed;

    notifyModifierStateChanged ();
}

//---------------------------------------------------------------------

void kpTool::setControlPressed (bool pressed)
{
    if (pressed == d->controlPressed) {
        return;
    }

    d->controlPressed = pressed;

    notifyModifierStateChanged ();
}

//---------------------------------------------------------------------

void kpTool::setAltPressed (bool pressed)
{
    if (pressed == d->altPressed) {
        return;
    }

    d->altPressed = pressed;

    notifyModifierStateChanged ();
}

//---------------------------------------------------------------------
