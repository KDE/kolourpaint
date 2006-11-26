
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_SELECTION 0


#include <kptoolselection.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionDestroyCommand.h>
#include <kpToolSelectionMoveCommand.h>
#include <kpToolSelectionPullFromDocumentCommand.h>
#include <kpToolSelectionResizeScaleCommand.h>
#include <kpToolSelectionTransparencyCommand.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpToolSelection::kpToolSelection (Mode mode,
        const QString &text,
        const QString &description,
        int key,
        kpMainWindow *mainWindow,
        const QString &name)
    : kpTool (text, description, key, mainWindow, name),
      m_mode (mode),
      m_currentPullFromDocumentCommand (0),
      m_currentMoveCommand (0),
      m_currentResizeScaleCommand (0),
      m_toolWidgetOpaqueOrTransparent (0),
      m_currentCreateTextCommand (0),
      m_createNOPTimer (new QTimer (this)),
      m_RMBMoveUpdateGUITimer (new QTimer (this))
{
    m_createNOPTimer->setSingleShot (true);
    connect (m_createNOPTimer, SIGNAL (timeout ()),
             this, SLOT (delayedDraw ()));

    m_RMBMoveUpdateGUITimer->setSingleShot (true);
    connect (m_RMBMoveUpdateGUITimer, SIGNAL (timeout ()),
             this, SLOT (slotRMBMoveUpdateGUI ()));
}

kpToolSelection::~kpToolSelection ()
{
}


// private
void kpToolSelection::pushOntoDocument ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelection::pushOntoDocument() CALLED" << endl;
#endif
    mainWindow ()->slotDeselect ();
}


// protected
bool kpToolSelection::onSelectionToMove () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v)
        return 0;

    return v->mouseOnSelectionToMove (currentViewPoint ());
}

// protected
int kpToolSelection::onSelectionResizeHandle () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v)
        return 0;

    return v->mouseOnSelectionResizeHandle (currentViewPoint ());
}


// protected virtual
QString kpToolSelection::haventBegunDrawUserMessageOnResizeHandle () const
{
    return i18n ("Left drag to scale selection.");
}

// protected virtual
QString kpToolSelection::haventBegunDrawUserMessageInsideSelection () const
{
    return i18n ("Left drag to move selection.");
}

// protected virtual
QString kpToolSelection::haventBegunDrawUserMessageOutsideSelection () const
{
    return i18n ("Left drag to create selection.");
}


// public
QString kpToolSelection::haventBegunDrawUserMessage () const
{
#if DEBUG_KP_TOOL_SELECTION && 0
    kDebug () << "kpToolSelection::haventBegunDrawUserMessage()"
                  " cancelledShapeButStillHoldingButtons="
               << m_cancelledShapeButStillHoldingButtons
               << endl;
#endif

    if (m_cancelledShapeButStillHoldingButtons)
        return i18n ("Let go of all the mouse buttons.");

    kpSelection *sel = document ()->selection ();
    if (sel && onSelectionResizeHandle () && !controlOrShiftPressed ())
    {
        return /*virtual*/haventBegunDrawUserMessageOnResizeHandle ();
    }
    else if (sel && sel->contains (currentPoint ()))
    {
        return /*virtual*/haventBegunDrawUserMessageInsideSelection ();
    }
    else
    {
        return /*virtual*/haventBegunDrawUserMessageOutsideSelection ();
    }
}


// virtual
void kpToolSelection::begin ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::begin()" << endl;
#endif

    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

    m_toolWidgetOpaqueOrTransparent = tb->toolWidgetOpaqueOrTransparent ();
    Q_ASSERT (m_toolWidgetOpaqueOrTransparent);
    connect (m_toolWidgetOpaqueOrTransparent, SIGNAL (isOpaqueChanged (bool)),
             this, SLOT (slotIsOpaqueChanged ()));
    m_toolWidgetOpaqueOrTransparent->show ();

    viewManager ()->setQueueUpdates ();
    {
        viewManager ()->setSelectionBorderVisible (true);
        viewManager ()->setSelectionBorderFinished (true);
    }
    viewManager ()->restoreQueueUpdates ();

    m_startDragFromSelectionTopLeft = QPoint ();
    m_dragType = Unknown;
    m_dragHasBegun = false;
    m_hadSelectionBeforeDrag = false;  // arbitrary
    m_resizeScaleType = 0;

    m_currentPullFromDocumentCommand = 0;
    m_currentMoveCommand = 0;
    m_currentResizeScaleCommand = 0;
    m_currentCreateTextCommand = 0;

    m_cancelledShapeButStillHoldingButtons = false;

    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolSelection::end ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::end()" << endl;
#endif

    if (document ()->selection ())
        pushOntoDocument ();

    Q_ASSERT (m_toolWidgetOpaqueOrTransparent);
    disconnect (m_toolWidgetOpaqueOrTransparent, SIGNAL (isOpaqueChanged (bool)),
                this, SLOT (slotIsOpaqueChanged ()));
    m_toolWidgetOpaqueOrTransparent = 0;

    viewManager ()->unsetCursor ();
}

// virtual
void kpToolSelection::reselect ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::reselect()" << endl;
#endif

    if (document ()->selection ())
        pushOntoDocument ();
}


// protected virtual
kpToolSelection::DragType kpToolSelection::beginDrawInsideSelection ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "\t\tis move" << endl;
#endif

    m_startDragFromSelectionTopLeft =
        currentPoint () - document ()->selection ()->topLeft ();

    if (mouseButton () == 0)
    {
        setSelectionBorderForMove ();
    }
    else
    {
        // Don't hide sel border momentarily if user is just
        // right _clicking_ selection.
        // (single shot timer)
        m_RMBMoveUpdateGUITimer->start (100/*ms*/);
    }

    return kpToolSelection::Move;
}


// virtual
void kpToolSelection::beginDraw ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::beginDraw() startPoint ()="
               << startPoint ()
               << " QCursor::pos() view startPoint="
               << viewUnderStartPoint ()->mapFromGlobal (QCursor::pos ())
               << endl;
#endif

    m_createNOPTimer->stop ();
    m_RMBMoveUpdateGUITimer->stop ();


    // In case the cursor was wrong to start with
    // (forgot to call kpTool::somethingBelowTheCursorChanged()),
    // make sure it is correct during this operation.
    hover (currentPoint ());

    // Currently used only to end the current text
    if (hasBegunShape ())
    {
        endShape (currentPoint (),
            kpBug::QRect_Normalized (
                QRect (startPoint ()/* TODO: wrong */, currentPoint ())));
    }

    m_dragType = Create;
    m_dragHasBegun = false;

    kpSelection *sel = document ()->selection ();
    m_hadSelectionBeforeDrag = bool (sel);

    if (sel)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thas sel region rect=" << sel->boundingRect () << endl;
    #endif
        QRect selectionRect = sel->boundingRect ();

        if (onSelectionResizeHandle () && !controlOrShiftPressed ())
        {
        #if DEBUG_KP_TOOL_SELECTION
            kDebug () << "\t\tis resize/scale" << endl;
        #endif

            m_startDragFromSelectionTopLeft = currentPoint () - selectionRect.topLeft ();
            m_dragType = ResizeScale;
            m_resizeScaleType = onSelectionResizeHandle ();

            viewManager ()->setQueueUpdates ();
            {
                viewManager ()->setSelectionBorderVisible (true);
                viewManager ()->setSelectionBorderFinished (true);
                viewManager ()->setTextCursorEnabled (false);
            }
            viewManager ()->restoreQueueUpdates ();
        }
        else if (sel->contains (currentPoint ()))
        {
            m_dragType = /*virtual*/beginDrawInsideSelection ();
        }
        else
        {
        #if DEBUG_KP_TOOL_SELECTION
            kDebug () << "\t\tis new sel" << endl;
        #endif

            pushOntoDocument ();
        }
    }

    // creating new selection?
    if (m_dragType == Create)
    {
        viewManager ()->setQueueUpdates ();
        {
            viewManager ()->setSelectionBorderVisible (true);
            viewManager ()->setSelectionBorderFinished (false);
            viewManager ()->setTextCursorEnabled (false);
        }
        viewManager ()->restoreQueueUpdates ();

        // (single shot)
        m_createNOPTimer->start (200/*ms*/);
    }

    if (m_dragType != SelectText)
    {
        setUserMessage (cancelUserMessage ());
    }
}


// protected virtual
QCursor kpToolSelection::cursorInsideSelection () const
{
    return Qt::SizeAllCursor;
}


// protected
QCursor kpToolSelection::cursor () const
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelection::cursor()"
               << " currentPoint ()=" << currentPoint ()
               << " QCursor::pos() view under cursor="
               << (viewUnderCursor () ?
                    viewUnderCursor ()->mapFromGlobal (QCursor::pos ()) :
                    KP_INVALID_POINT)
               << " controlOrShiftPressed=" << controlOrShiftPressed ()
               << endl;
#endif

    kpSelection *sel = document () ? document ()->selection () : 0;

    if (sel && onSelectionResizeHandle () && !controlOrShiftPressed ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kDebug () << "\tonSelectionResizeHandle="
                   << onSelectionResizeHandle () << endl;
    #endif
        switch (onSelectionResizeHandle ())
        {
        case (kpView::Top | kpView::Left):
        case (kpView::Bottom | kpView::Right):
            return Qt::SizeFDiagCursor;

        case (kpView::Bottom | kpView::Left):
        case (kpView::Top | kpView::Right):
            return Qt::SizeBDiagCursor;

        case kpView::Top:
        case kpView::Bottom:
            return Qt::SizeVerCursor;

        case kpView::Left:
        case kpView::Right:
            return Qt::SizeHorCursor;
        }

        return Qt::ArrowCursor;
    }
    else if (sel && sel->contains (currentPoint ()))
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kDebug () << "\tsel contains currentPoint" << endl;
    #endif
        return /*virtual*/cursorInsideSelection ();
    }
    else
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kDebug () << "\tnot on sel" << endl;
    #endif
        return Qt::CrossCursor;
    }
}

// virtual
void kpToolSelection::hover (const QPoint &point)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelection::hover" << point << endl;
#endif

    viewManager ()->setCursor (cursor ());

    setUserShapePoints (point, KP_INVALID_POINT, false/*don't set size*/);
    if (document () && document ()->selection ())
    {
        setUserShapeSize (document ()->selection ()->width (),
                          document ()->selection ()->height ());
    }
    else
    {
        setUserShapeSize (KP_INVALID_SIZE);
    }

    QString mess = haventBegunDrawUserMessage ();
    if (mess != userMessage ())
        setUserMessage (mess);
}

// protected
void kpToolSelection::popupRMBMenu ()
{
    QMenu *pop = mainWindow () ? mainWindow ()->selectionToolRMBMenu () : 0;
    Q_ASSERT (pop);

    // WARNING: enters event loop - may re-enter view/tool event handlers
    pop->exec (QCursor::pos ());

    // Cursor may have moved while menu up, triggering mouseMoveEvents
    // for the menu - not the view.  Update cursor position now.
    somethingBelowTheCursorChanged ();
}

// protected
void kpToolSelection::setSelectionBorderForMove ()
{
    // don't show border while moving
    viewManager ()->setQueueUpdates ();
    {
        viewManager ()->setSelectionBorderVisible (false);
        viewManager ()->setSelectionBorderFinished (true);
        viewManager ()->setTextCursorEnabled (false);
    }
    viewManager ()->restoreQueueUpdates ();
}

// protected slot
void kpToolSelection::slotRMBMoveUpdateGUI ()
{
    // (just in case not called from single shot)
    m_RMBMoveUpdateGUITimer->stop ();

    setSelectionBorderForMove ();

    kpSelection * const sel = document () ? document ()->selection () : 0;
    if (sel)
        setUserShapePoints (sel->topLeft ());
}

// protected slot
void kpToolSelection::delayedDraw ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelection::delayedDraw() hasBegunDraw="
               << hasBegunDraw ()
               << " currentPoint=" << currentPoint ()
               << " lastPoint=" << lastPoint ()
               << " startPoint=" << startPoint ()
               << endl;
#endif

    // (just in case not called from single shot)
    m_createNOPTimer->stop ();

    if (hasBegunDraw ())
    {
        draw (currentPoint (), lastPoint (),
              kpBug::QRect_Normalized (QRect (startPoint (), currentPoint ())));
    }
}

// private
void kpToolSelection::create (const QPoint &thisPoint, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\tnot moving - resizing rect to" << normalizedRect
                << endl;
    kDebug () << "\t\tcreateNOPTimer->isActive()="
                << m_createNOPTimer->isActive ()
                << " viewManhattanLength from startPoint="
                << viewUnderStartPoint ()->transformDocToViewX ((thisPoint - startPoint ()).manhattanLength ())
                << endl;
#endif

    bool nextDragHasBegun = true;


    QPoint accidentalDragAdjustedPoint = thisPoint;
    
    if (m_createNOPTimer->isActive ())
    {
        if (viewUnderStartPoint ()->transformDocToViewX (
                (accidentalDragAdjustedPoint - startPoint ()).manhattanLength ()) <= 6)
        {
        #if DEBUG_KP_TOOL_SELECTION && 1
            kDebug () << "\t\tsuppress accidental movement" << endl;
        #endif
            accidentalDragAdjustedPoint = startPoint ();
        }
        else
        {
        #if DEBUG_KP_TOOL_SELECTION && 1
            kDebug () << "\t\tit's a \"big\" intended move - stop timer" << endl;
        #endif
            m_createNOPTimer->stop ();
        }
    }


    // Prevent unintentional 1-pixel selections
    if (!m_dragHasBegun && accidentalDragAdjustedPoint == startPoint ())
    {
        if (m_mode != kpToolSelection::Text)
        {
        #if DEBUG_KP_TOOL_SELECTION && 1
            kDebug () << "\tnon-text NOP - return" << endl;
        #endif
            setUserShapePoints (accidentalDragAdjustedPoint);
            return;
        }
        else  // m_mode == kpToolSelection::Text
        {
            // Attempt to deselect text box by clicking?
            if (m_hadSelectionBeforeDrag)
            {
            #if DEBUG_KP_TOOL_SELECTION && 1
                kDebug () << "\ttext box deselect - NOP - return" << endl;
            #endif
                setUserShapePoints (accidentalDragAdjustedPoint);
                return;
            }

            // Drag-wise, this is a NOP so we'd normally return (hence
            // m_dragHasBegun would not change).  However, as a special
            // case, allow user to create a text box using a single
            // click.  But don't set m_dragHasBegun for next iteration
            // since it would be untrue.
            //
            // This makes sure that a single click creation of text box
            // works even if draw() is invoked more than once at the
            // same position (esp. with accidental drag suppression
            // (above)).
            nextDragHasBegun = false;
        }
    }


    /*virtual*/createMoreSelectionAndUpdateStatusBar (accidentalDragAdjustedPoint,
        normalizedRect);

    viewManager ()->setSelectionBorderVisible (true);


    m_dragHasBegun = nextDragHasBegun;
}

// private
void kpToolSelection::move (const QPoint &thisPoint, const QRect &/*normalizedRect*/)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\tmoving selection" << endl;
#endif

    kpSelection *sel = document ()->selection ();

    QRect targetSelRect (thisPoint.x () - m_startDragFromSelectionTopLeft.x (),
        thisPoint.y () - m_startDragFromSelectionTopLeft.y (),
        sel->width (),
        sel->height ());

#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\t\tstartPoint=" << startPoint ()
                << " thisPoint=" << thisPoint
                << " startDragFromSel=" << m_startDragFromSelectionTopLeft
                << " targetSelRect=" << targetSelRect
                << endl;
#endif

    // Try to make sure selection still intersects document so that it's
    // reachable.

    if (targetSelRect.right () < 0)
        targetSelRect.translate (-targetSelRect.right (), 0);
    else if (targetSelRect.left () >= document ()->width ())
        targetSelRect.translate (document ()->width () - targetSelRect.left () - 1, 0);

    if (targetSelRect.bottom () < 0)
        targetSelRect.translate (0, -targetSelRect.bottom ());
    else if (targetSelRect.top () >= document ()->height ())
        targetSelRect.translate (0, document ()->height () - targetSelRect.top () - 1);

#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\t\t\tafter ensure sel rect clickable=" << targetSelRect << endl;
#endif


    if (!m_dragHasBegun &&
        targetSelRect.topLeft () + m_startDragFromSelectionTopLeft == startPoint ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kDebug () << "\t\t\t\tnop" << endl;
    #endif


        if (!m_RMBMoveUpdateGUITimer->isActive ())
        {
            // (slotRMBMoveUpdateGUI() calls similar line)
            setUserShapePoints (sel->topLeft ());
        }

        // Prevent both NOP drag-moves
        return;
    }


    if (m_RMBMoveUpdateGUITimer->isActive ())
    {
        m_RMBMoveUpdateGUITimer->stop ();
        slotRMBMoveUpdateGUI ();
    }


    if (!sel->pixmap () && !m_currentPullFromDocumentCommand)
    {
        m_currentPullFromDocumentCommand = new kpToolSelectionPullFromDocumentCommand (
            QString::null/*uninteresting child of macro cmd*/,
            mainWindow ());
        m_currentPullFromDocumentCommand->execute ();
    }

    if (!m_currentMoveCommand)
    {
        m_currentMoveCommand = new kpToolSelectionMoveCommand (
            QString::null/*uninteresting child of macro cmd*/,
            mainWindow ());
        m_currentMoveCommandIsSmear = false;
    }


    //viewManager ()->setQueueUpdates ();
    //viewManager ()->setFastUpdates ();

    if (shiftPressed ())
        m_currentMoveCommandIsSmear = true;

    if (!m_dragHasBegun && (controlPressed () || shiftPressed ()))
        m_currentMoveCommand->copyOntoDocument ();

    m_currentMoveCommand->moveTo (targetSelRect.topLeft ());

    if (shiftPressed ())
        m_currentMoveCommand->copyOntoDocument ();

    //viewManager ()->restoreFastUpdates ();
    //viewManager ()->restoreQueueUpdates ();

    QPoint start = m_currentMoveCommand->originalSelection ().topLeft ();
    QPoint end = targetSelRect.topLeft ();
    setUserShapePoints (start, end, false/*don't set size*/);
    setUserShapeSize (end.x () - start.x (), end.y () - start.y ());
}

// private
void kpToolSelection::resizeScaleTryKeepAspect (int newWidth, int newHeight,
        bool horizontalGripDragged, bool verticalGripDragged,
        const kpSelection &originalSelection,
        int *newWidthOut, int *newHeightOut)
{
    const int oldWidth = originalSelection.width (),
        oldHeight = originalSelection.height ();
        
    // Width changed more than height?  At equality, favor width.
    // Fix width, change height.
    //
    // We use <horizontalGripDragged> and <verticalGripDragged> to prevent
    // e.g. the situation where we've dragged such that newWidth < oldWidth but
    // we're not dragging a vertical grip.  We certainly don't want this
    // code to modify the width - we want to fix the width and change the
    // height.
    if ((horizontalGripDragged ? double (newWidth) / oldWidth : 0) >=
        (verticalGripDragged ? double (newHeight) / oldHeight : 0))
    {
        *newHeightOut = newWidth * oldHeight / oldWidth;
        *newHeightOut = qMax (originalSelection.minimumHeight (), *newHeightOut);
    }
    // Height changed more than width?
    // Fix height, change width.
    else
    {
        *newWidthOut = newHeight * oldWidth / oldHeight;
        *newWidthOut = qMax (originalSelection.minimumWidth (), *newWidthOut);
    }
}

// private
void kpToolSelection::resizeScaleCalculateNewSelectionPosSize (
        const kpSelection &originalSelection,
        int *newX, int *newY,
        int *newWidth, int *newHeight)
{
    //
    // Determine new width.
    //

    // Dragging left or right grip?
    // If left, positive X drags decrease width.
    // If right, positive X drags increase width.
    int userXSign = 0;
    if (m_resizeScaleType & kpView::Left)
        userXSign = -1;
    else if (m_resizeScaleType & kpView::Right)
        userXSign = +1;

    // Calcluate new width.
    *newWidth = originalSelection.width () +
        userXSign * (currentPoint ().x () - startPoint ().x ());

    // Don't allow new width to be less than that kind of selection type's
    // minimum.
    *newWidth = qMax (originalSelection.minimumWidth (), *newWidth);


    //
    // Determine new height.
    //

    // Dragging top or bottom grip?
    // If top, positive Y drags decrease height.
    // If bottom, positive Y drags increase height.
    int userYSign = 0;
    if (m_resizeScaleType & kpView::Top)
        userYSign = -1;
    else if (m_resizeScaleType & kpView::Bottom)
        userYSign = +1;

    // Calcluate new height.
    *newHeight = originalSelection.height () +
        userYSign * (currentPoint ().y () - startPoint ().y ());

    // Don't allow new height to be less than that kind of selection type's
    // minimum.
    *newHeight = qMax (originalSelection.minimumHeight (), *newHeight);


    // Keep aspect ratio?
    if (shiftPressed ())
    {
        resizeScaleTryKeepAspect (*newWidth, *newHeight,
            (userXSign != 0)/*X or XY grip dragged*/,
                (userYSign != 0)/*Y or XY grip dragged*/,
            originalSelection,
            newWidth/*ptr*/, newHeight/*ptr*/);
    }


    *newX = originalSelection.x ();
    *newY = originalSelection.y ();


    //
    // Adjust x/y to new width/height for left/top resizes.
    //
    
    if (m_resizeScaleType & kpView::Left)
    {
        *newX -= (*newWidth - originalSelection.width ());
    }

    if (m_resizeScaleType & kpView::Top)
    {
        *newY -= (*newHeight - originalSelection.height ());
    }

#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\t\tnewX=" << *newX
                << " newY=" << *newY
                << " newWidth=" << *newWidth
                << " newHeight=" << *newHeight
                << endl;
#endif
}

// private
void kpToolSelection::resizeScale (
        const QPoint &thisPoint,
        const QRect &/*normalizedRect*/)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\tresize/scale" << endl;
#endif

    kpSelection *sel = document ()->selection ();

    if (!m_dragHasBegun && thisPoint == startPoint ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kDebug () << "\t\tnop" << endl;
    #endif

        setUserShapePoints (QPoint (sel->width (), sel->height ()));
        return;
    }


    if (!sel->pixmap () && !m_currentPullFromDocumentCommand)
    {
        m_currentPullFromDocumentCommand =
            new kpToolSelectionPullFromDocumentCommand (
                QString::null/*uninteresting child of macro cmd*/,
                mainWindow ());
        m_currentPullFromDocumentCommand->execute ();
    }

    if (!m_currentResizeScaleCommand)
    {
        m_currentResizeScaleCommand
            = new kpToolSelectionResizeScaleCommand (mainWindow ());
    }


    kpSelection originalSelection = m_currentResizeScaleCommand->originalSelection ();


    // There is nothing illegal about position (-1,-1) but why not.
    int newX = -1, newY = -1,
        newWidth = 0, newHeight = 0;

    // This should change all of the above values.
    resizeScaleCalculateNewSelectionPosSize (
        originalSelection,
        &newX, &newY,
        &newWidth, &newHeight);


    viewManager ()->setFastUpdates ();
    {
        m_currentResizeScaleCommand->resizeAndMoveTo (
            newWidth, newHeight,
            QPoint (newX, newY),
            true/*smooth scale delayed*/);
    }
    viewManager ()->restoreFastUpdates ();

    setUserShapePoints (QPoint (originalSelection.width (),
                                originalSelection.height ()),
                        QPoint (newWidth,
                                newHeight),
                        false/*don't set size*/);
    setUserShapeSize (newWidth - originalSelection.width (),
                        newHeight - originalSelection.height ());
}

// virtual
void kpToolSelection::draw (const QPoint &thisPoint, const QPoint & /*lastPoint*/,
                            const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelection::draw" << thisPoint
               << " startPoint=" << startPoint ()
               << " normalizedRect=" << normalizedRect << endl;
#endif


    // OPT: return when thisPoint == lastPoint () so that e.g. when creating
    //      Points sel, press modifiers doesn't add multiple points in same
    //      place


    if (m_dragType == Create)
    {
        create (thisPoint, normalizedRect);
    }
    else if (m_dragType == Move)
    {
        move (thisPoint, normalizedRect);
    }
    else if (m_dragType == ResizeScale)
    {
        resizeScale (thisPoint, normalizedRect);
    }
}

// protected virtual
void kpToolSelection::setSelectionBorderForHaventBegunDraw ()
{
    viewManager ()->setSelectionBorderVisible (true);
    viewManager ()->setSelectionBorderFinished (true);
}

// private
void kpToolSelection::cancelMove ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "\twas drag moving - undo drag and undo acquire" << endl;
#endif

    if (m_currentMoveCommand)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\t\tundo currentMoveCommand" << endl;
    #endif
        m_currentMoveCommand->finalize ();
        m_currentMoveCommand->unexecute ();
        delete m_currentMoveCommand;
        m_currentMoveCommand = 0;

        if (document ()->selection ()->isText ())
            viewManager ()->setTextCursorBlinkState (true);
    }
}

// private
void kpToolSelection::cancelCreate ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "\twas creating sel - kill" << endl;
#endif

    // TODO: should we give the user back the selection s/he had before (if any)?
    document ()->selectionDelete ();

    if (m_currentCreateTextCommand)
    {
        delete m_currentCreateTextCommand;
        m_currentCreateTextCommand = 0;
    }
}

// private
void kpToolSelection::cancelResizeScale ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "\twas resize/scale sel - kill" << endl;
#endif

    if (m_currentResizeScaleCommand)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\t\tundo currentResizeScaleCommand" << endl;
    #endif
        m_currentResizeScaleCommand->unexecute ();
        delete m_currentResizeScaleCommand;
        m_currentResizeScaleCommand = 0;

        if (document ()->selection ()->isText ())
            viewManager ()->setTextCursorBlinkState (true);
    }
}

// virtual
void kpToolSelection::cancelShape ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::cancelShape() mouseButton="
              << mouseButton () << endl;
#endif

    m_createNOPTimer->stop ();
    m_RMBMoveUpdateGUITimer->stop ();


    viewManager ()->setQueueUpdates ();
    {
        if (m_dragType == Move)
        {
            cancelMove ();
        }
        else if (m_dragType == Create)
        {
            cancelCreate ();
        }
        else if (m_dragType == ResizeScale)
        {
            cancelResizeScale ();
        }


        if (m_currentPullFromDocumentCommand)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kDebug () << "\t\tundo pullFromDocumentCommand" << endl;
        #endif
            m_currentPullFromDocumentCommand->unexecute ();
            delete m_currentPullFromDocumentCommand;
            m_currentPullFromDocumentCommand = 0;
        }


        /*virtual*/setSelectionBorderForHaventBegunDraw ();
    }
    viewManager ()->restoreQueueUpdates ();


    m_dragType = Unknown;
    m_cancelledShapeButStillHoldingButtons = true;
    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

// virtual
void kpToolSelection::releasedAllButtons ()
{
    m_cancelledShapeButStillHoldingButtons = false;
    setUserMessage (haventBegunDrawUserMessage ());
}

// protected virtual
QString kpToolSelection::nonSmearMoveCommandName () const
{
    return i18n ("Selection: Move");
}

// virtual
void kpToolSelection::endDraw (const QPoint & /*thisPoint*/,
        const QRect & /*normalizedRect*/)
{
    m_createNOPTimer->stop ();
    m_RMBMoveUpdateGUITimer->stop ();


    viewManager ()->setQueueUpdates ();
    {
        if (m_currentCreateTextCommand)
        {
            commandHistory ()->addCommand (m_currentCreateTextCommand, false/*no exec*/);
            m_currentCreateTextCommand = 0;
        }

        kpMacroCommand *cmd = 0;
        if (m_currentMoveCommand)
        {
            if (m_currentMoveCommandIsSmear)
            {
                cmd = new kpMacroCommand (i18n ("%1: Smear",
                    document ()->selection ()->name ()),
                    mainWindow ());
            }
            else
            {
                cmd = new kpMacroCommand (
                    /*virtual*/nonSmearMoveCommandName (),
                    mainWindow ());
            }

            if (document ()->selection ()->isText ())
                viewManager ()->setTextCursorBlinkState (true);
        }
        else if (m_currentResizeScaleCommand)
        {
            cmd = new kpMacroCommand (
                m_currentResizeScaleCommand->kpNamedCommand::name (),
                mainWindow ());

            if (document ()->selection ()->isText ())
                viewManager ()->setTextCursorBlinkState (true);
        }

        if (m_currentPullFromDocumentCommand)
        {
            Q_ASSERT (m_currentMoveCommand || m_currentResizeScaleCommand);
            kpSelection selection;

            if (m_currentMoveCommand)
                selection = m_currentMoveCommand->originalSelection ();
            else if (m_currentResizeScaleCommand)
                selection = m_currentResizeScaleCommand->originalSelection ();

            // just the border
            selection.setPixmap (QPixmap ());

            kpCommand *createCommand = new kpToolSelectionCreateCommand (
                i18n ("Selection: Create"),
                selection,
                mainWindow ());

            if (kpToolSelectionCreateCommand::nextUndoCommandIsCreateBorder (
                    commandHistory ()))
            {
                commandHistory ()->setNextUndoCommand (createCommand);
            }
            else
            {
                commandHistory ()->addCommand (createCommand,
                    false/*no exec - user already dragged out sel*/);
            }


            cmd->addCommand (m_currentPullFromDocumentCommand);
            m_currentPullFromDocumentCommand = 0;
        }

        if (m_currentMoveCommand)
        {
            m_currentMoveCommand->finalize ();
            cmd->addCommand (m_currentMoveCommand);
            m_currentMoveCommand = 0;

            if (document ()->selection ()->isText ())
                viewManager ()->setTextCursorBlinkState (true);
        }

        if (m_currentResizeScaleCommand)
        {
            cmd->addCommand (m_currentResizeScaleCommand);
            m_currentResizeScaleCommand = 0;

            if (document ()->selection ()->isText ())
                viewManager ()->setTextCursorBlinkState (true);
        }

        if (cmd)
            commandHistory ()->addCommand (cmd, false/*no exec*/);

        /*virtual*/setSelectionBorderForHaventBegunDraw ();
    }
    viewManager ()->restoreQueueUpdates ();


    m_dragType = Unknown;
    setUserMessage (haventBegunDrawUserMessage ());


    if (mouseButton () == 1/*right*/)
        popupRMBMenu ();
}


// protected virtual [base kpTool]
void kpToolSelection::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    kDebug () << "kpToolSelection::keyPressEvent(e->text='"
              << e->text () << "')" << endl;
#endif


    e->ignore ();


    if (document ()->selection () &&
        !hasBegunDraw () &&
         e->key () == Qt::Key_Escape)
    {
    #if DEBUG_KP_TOOL_SELECTION && 0
        kDebug () << "\tescape pressed with sel when not begun draw - deselecting"
                  << endl;
    #endif

        pushOntoDocument ();
        e->accept ();
    }


    if (!e->isAccepted ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 0
        kDebug () << "\tkey processing did not accept (text was '"
                   << e->text ()
                   << "') - passing on event to kpTool"
                   << endl;
    #endif

        kpTool::keyPressEvent (e);
        return;
    }
}


// private slot
void kpToolSelection::selectionTransparencyChanged (const QString & /*name*/)
{
#if 0
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::selectionTransparencyChanged(" << name << ")" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave sel - set transparency" << endl;
    #endif

        kpSelectionTransparency oldST = document ()->selection ()->transparency ();
        kpSelectionTransparency st = mainWindow ()->selectionTransparency ();

        // TODO: This "NOP" check causes us a great deal of trouble e.g.:
        //
        //       Select a solid red rectangle.
        //       Switch to transparent and set red as the background colour.
        //       (the selection is now invisible)
        //       Invert Colours.
        //       (the selection is now cyan)
        //       Change the background colour to green.
        //       (no command is added to undo this as the selection does not change)
        //       Undo.
        //       The rectangle is no longer invisible.
        //
        //if (document ()->selection ()->setTransparency (st, true/*check harder for no change in mask*/))

        document ()->selection ()->setTransparency (st);
        if (true)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kDebug () << "\t\twhich changed the pixmap" << endl;
        #endif

            commandHistory ()->addCommand (new kpToolSelectionTransparencyCommand (
                i18n ("Selection: Transparency"), // name,
                st, oldST,
                mainWindow ()),
                false/* no exec*/);
        }
    }
#endif

    // TODO: I've duplicated the code (see below 3x) to make sure
    //       kpSelectionTransparency(oldST)::transparentColor() is defined
    //       and not taken from kpDocument (where it may not be defined because
    //       the transparency may be opaque).
    //
    //       That way kpToolSelectionTransparencyCommand can force set colours.
}


// protected slot virtual
void kpToolSelection::slotIsOpaqueChanged ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::slotIsOpaqueChanged()" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave sel - set transparency" << endl;
    #endif

        QApplication::setOverrideCursor (Qt::WaitCursor);

        if (hasBegunShape ())
            endShapeInternal ();

        kpSelectionTransparency st = mainWindow ()->selectionTransparency ();
        kpSelectionTransparency oldST = st;
        oldST.setOpaque (!oldST.isOpaque ());

        document ()->selection ()->setTransparency (st);
        commandHistory ()->addCommand (new kpToolSelectionTransparencyCommand (
            st.isOpaque () ?
                i18n ("Selection: Opaque") :
                i18n ("Selection: Transparent"),
            st, oldST,
            mainWindow ()),
            false/* no exec*/);

        QApplication::restoreOverrideCursor ();
    }
}

// protected slot virtual [base kpTool]
void kpToolSelection::slotBackgroundColorChanged (const kpColor &)
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::slotBackgroundColorChanged()" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave sel - set transparency" << endl;
    #endif

        QApplication::setOverrideCursor (Qt::WaitCursor);

        kpSelectionTransparency st = mainWindow ()->selectionTransparency ();
        kpSelectionTransparency oldST = st;
        oldST.setTransparentColor (oldBackgroundColor ());

        document ()->selection ()->setTransparency (st);
        commandHistory ()->addCommand (new kpToolSelectionTransparencyCommand (
            i18n ("Selection: Transparency Color"),
            st, oldST,
            mainWindow ()),
            false/* no exec*/);

        QApplication::restoreOverrideCursor ();
    }
}

// protected slot virtual [base kpTool]
void kpToolSelection::slotColorSimilarityChanged (double, int)
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelection::slotColorSimilarityChanged()" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave sel - set transparency" << endl;
    #endif

        QApplication::setOverrideCursor (Qt::WaitCursor);

        kpSelectionTransparency st = mainWindow ()->selectionTransparency ();
        kpSelectionTransparency oldST = st;
        oldST.setColorSimilarity (oldColorSimilarity ());

        document ()->selection ()->setTransparency (st);
        commandHistory ()->addCommand (new kpToolSelectionTransparencyCommand (
            i18n ("Selection: Transparency Color Similarity"),
            st, oldST,
            mainWindow ()),
            false/* no exec*/);

        QApplication::restoreOverrideCursor ();
    }
}


#include <kptoolselection.moc>
