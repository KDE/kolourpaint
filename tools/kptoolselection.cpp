
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_SELECTION 1


#include <kptoolselection.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpviewmanager.h>


kpToolSelection::kpToolSelection (Mode mode,
                                  const QString &text,
                                  const QString &description,
                                  int key,
                                  kpMainWindow *mainWindow,
                                  const char *name)
    : kpTool (text, description, key, mainWindow, name),
      m_mode (mode),
      m_currentPullFromDocumentCommand (0),
      m_currentMoveCommand (0),
      m_currentResizeScaleCommand (0),
      m_toolWidgetOpaqueOrTransparent (0),
      m_currentCreateTextCommand (0)
{
}

kpToolSelection::~kpToolSelection ()
{
}


// private
void kpToolSelection::pushOntoDocument ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelection::pushOntoDocument() CALLED" << endl;
#endif
    mainWindow ()->slotDeselect ();
}


// public
QString kpToolSelection::haventBegunDrawUserMessage () const
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelection::haventBegunDrawUserMessage()"
                  " cancelledShapeButStillHoldingButtons="
               << m_cancelledShapeButStillHoldingButtons
               << endl;
#endif

    if (m_cancelledShapeButStillHoldingButtons)
        return i18n ("Let go of all the mouse buttons.");

    if (document () && document ()->selection () && document ()->selection ()->contains (m_currentPoint))
    {
        if (m_mode == Text)
        {
            if (document ()->selection ()->pointIsInTextArea (m_currentPoint))
                return i18n ("Left click to change cursor position.");
            else if (document ()->selection ()->pointOnResizeHandle (m_currentPoint))
                return i18n ("Left drag to resize text box.");
            else
                return i18n ("Left drag to move text box.");
        }
        else
        {
            if (document ()->selection ()->pointOnResizeHandle (m_currentPoint))
                return i18n ("Left drag to scale selection.");
            else
                return i18n ("Left drag to move selection.");
        }
    }
    else
    {
        if (m_mode == Text)
            return i18n ("Left drag to create text box.");
        else
            return i18n ("Left drag to create selection.");
    }
}


// virtual
void kpToolSelection::begin ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::begin()" << endl;
#endif

    kpToolToolBar *tb = toolToolBar ();

    if (tb)
    {
        m_toolWidgetOpaqueOrTransparent = tb->toolWidgetOpaqueOrTransparent ();

        if (m_toolWidgetOpaqueOrTransparent)
        {
            connect (m_toolWidgetOpaqueOrTransparent, SIGNAL (isOpaqueChanged (bool)),
                     this, SLOT (slotIsOpaqueChanged ()));
            m_toolWidgetOpaqueOrTransparent->show ();
        }
    }
    else
    {
        m_toolWidgetOpaqueOrTransparent = 0;
    }

    viewManager ()->setSelectionBorderVisible (true);
    viewManager ()->setSelectionBorderFinished (true);

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
    kdDebug () << "kpToolSelection::end()" << endl;
#endif

    if (document ()->selection ())
        pushOntoDocument ();

    if (m_toolWidgetOpaqueOrTransparent)
    {
        disconnect (m_toolWidgetOpaqueOrTransparent, SIGNAL (isOpaqueChanged (bool)),
                    this, SLOT (slotIsOpaqueChanged ()));
        m_toolWidgetOpaqueOrTransparent = 0;
    }
}

// virtual
void kpToolSelection::reselect ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::reselect()" << endl;
#endif

    if (document ()->selection ())
        pushOntoDocument ();
}


// virtual
void kpToolSelection::beginDraw ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::beginDraw() CALLED" << endl;
#endif

    // In case the cursor was wrong to start with
    // (forgot to call kpTool::somethingBelowTheCursorChanged()),
    // make sure it is correct during this operation.
    hover (m_currentPoint);

    // Dragging with the RMB would make no sense
    // TODO: RMB click brings up Image popupmenu
    if (m_mouseButton != 0/*left*/)
        return;

    // Currently used only to end the current text
    if (hasBegunShape ())
        endShape (m_currentPoint, QRect (m_startPoint/* TODO: wrong */, m_currentPoint).normalize ());

    m_dragType = Create;
    m_dragHasBegun = false;

    kpSelection *sel = document ()->selection ();
    m_hadSelectionBeforeDrag = bool (sel);

    if (sel)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\thas sel region" << endl;
    #endif
        QRect selectionRect = sel->boundingRect ();

        if (sel->contains (m_currentPoint))
        {
            if (m_mode == Text && sel->pointIsInTextArea (m_currentPoint))
            {
            #if DEBUG_KP_TOOL_SELECTION
                kdDebug () << "\t\tis select cursor pos" << endl;
            #endif

                m_dragType = SelectText;

                viewManager ()->setTextCursorPosition (sel->textRowForPoint (m_currentPoint),
                                                       sel->textColForPoint (m_currentPoint));
            }
            else if (document ()->selection ()->pointOnResizeHandle (m_currentPoint))
            {
            #if DEBUG_KP_TOOL_SELECTION
                kdDebug () << "\t\tis resize/scale" << endl;
            #endif

                m_startDragFromSelectionTopLeft = m_currentPoint - selectionRect.topLeft ();
                m_dragType = ResizeScale;
                m_resizeScaleType = document ()->selection ()->pointOnResizeHandle (m_currentPoint);

                viewManager ()->setSelectionBorderVisible (true);
                viewManager ()->setSelectionBorderFinished (true);
                viewManager ()->setTextCursorEnabled (false);
            }
            else
            {
            #if DEBUG_KP_TOOL_SELECTION
                kdDebug () << "\t\tis move" << endl;
            #endif

                m_startDragFromSelectionTopLeft = m_currentPoint - selectionRect.topLeft ();
                m_dragType = Move;

                // don't show border while moving (or when we start to move)
                viewManager ()->setSelectionBorderVisible (false);
                viewManager ()->setSelectionBorderFinished (true);
                viewManager ()->setTextCursorEnabled (false);
            }
        }
        else
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\t\tis new sel" << endl;
        #endif

            pushOntoDocument ();
        }
    }

    // creating new selection?
    if (m_dragType == Create)
    {
        viewManager ()->setSelectionBorderVisible (true);
        viewManager ()->setSelectionBorderFinished (false);
        viewManager ()->setTextCursorEnabled (false);
    }

    if (m_dragType != SelectText)
    {
        setUserMessage (i18n ("%1 to cancel.")
                            .arg (mouseClickText (true/*other mouse button*/,
                                                  true/*start of sentence*/)));
    }
}


// protected
const QCursor &kpToolSelection::cursorForPoint (const QPoint &point) const
{
    if (document () && document ()->selection () && document ()->selection ()->contains (point))
    {
        if (m_mode == Text && document ()->selection ()->pointIsInTextArea (point))
            return Qt::ibeamCursor;
        else
        {
            switch (document ()->selection ()->pointOnResizeHandle (point))
            {
            case 0:
            default:
                return Qt::sizeAllCursor;

            case (kpSelection::Top | kpSelection::Left):
            case (kpSelection::Bottom | kpSelection::Right):
                return Qt::sizeFDiagCursor;

            case (kpSelection::Bottom | kpSelection::Left):
            case (kpSelection::Top | kpSelection::Right):
                return Qt::sizeBDiagCursor;

            case kpSelection::Top:
            case kpSelection::Bottom:
                return Qt::sizeVerCursor;

            case kpSelection::Left:
            case kpSelection::Right:
                return Qt::sizeHorCursor;
            }
        }
    }
    else
    {
        return Qt::crossCursor;
    }
}

// virtual
void kpToolSelection::hover (const QPoint &point)
{
    viewManager ()->setCursor (cursorForPoint (point));

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

// virtual
void kpToolSelection::draw (const QPoint &thisPoint, const QPoint & /*lastPoint*/,
                            const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelection::draw() CALLED" << endl;
#endif

    // Dragging with the RMB would make no sense
    // TODO: RMB click brings up Image popupmenu
    if (m_mouseButton != 0/*left*/)
    {
        setUserShapePoints (thisPoint, KP_INVALID_POINT, false/*don't set size*/);
        setUserMessage (i18n ("Let go of all the mouse buttons."));
        return;
    }


    if (m_dragType == Create)
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kdDebug () << "\tnot moving - resizing rect to" << normalizedRect
                   << endl;
    #endif

        // Prevent unintentional 1-pixel selections
        if (!m_dragHasBegun && thisPoint == m_startPoint &&
            // However, with a forced minimum size, a single click to create
            // a text box is ok (as long as it wasn't an attempt to deselect).
            !(m_mode == kpToolSelection::Text && !m_hadSelectionBeforeDrag))
        {
            setUserShapePoints (thisPoint);
            return;
        }


        switch (m_mode)
        {
        case kpToolSelection::Rectangle:
        {
            const QRect usefulRect = normalizedRect.intersect (document ()->rect ());
            document ()->setSelection (kpSelection (kpSelection::Rectangle, usefulRect,
                                                    mainWindow ()->selectionTransparency ()));

            setUserShapePoints (m_startPoint,
                                QPoint (QMAX (0, QMIN (m_currentPoint.x (), document ()->width () - 1)),
                                        QMAX (0, QMIN (m_currentPoint.y (), document ()->height () - 1))));
            break;
        }
        case kpToolSelection::Text:
        {
            QRect usefulRect = normalizedRect;

            const kpTextStyle textStyle = mainWindow ()->textStyle ();

            int minimumWidth = kpSelection::preferredMinimumWidthForTextStyle (textStyle);
            if (thisPoint.x () >= m_startPoint.x ())
            {
                if (m_startPoint.x () + minimumWidth - 1 >= document ()->width ())
                {
                    minimumWidth = QMAX (kpSelection::minimumWidthForTextStyle (textStyle),
                                         document ()->width () - m_startPoint.x ());
                }
            }
            else
            {
                if (m_startPoint.x () - minimumWidth + 1 < 0)
                {
                    minimumWidth = QMAX (kpSelection::minimumWidthForTextStyle (textStyle),
                                         m_startPoint.x () + 1);
                }
            }

            int minimumHeight = kpSelection::preferredMinimumHeightForTextStyle (textStyle);
            if (thisPoint.y () >= m_startPoint.y ())
            {
                if (m_startPoint.y () + minimumHeight - 1 >= document ()->height ())
                {
                    minimumHeight = QMAX (kpSelection::minimumHeightForTextStyle (textStyle),
                                          document ()->height () - m_startPoint.y ());
                }
            }
            else
            {
                if (m_startPoint.y () - minimumHeight + 1 < 0)
                {
                    minimumHeight = QMAX (kpSelection::minimumHeightForTextStyle (textStyle),
                                          m_startPoint.y () + 1);
                }
            }

            if (usefulRect.width () < minimumWidth)
            {
                if (thisPoint.x () >= m_startPoint.x ())
                    usefulRect.setWidth (minimumWidth);
                else
                    usefulRect.setX (usefulRect.right () - minimumWidth + 1);
            }

            if (usefulRect.height () < minimumHeight)
            {
                if (thisPoint.y () >= m_startPoint.y ())
                    usefulRect.setHeight (minimumHeight);
                else
                    usefulRect.setY (usefulRect.bottom () - minimumHeight + 1);
            }
        #if DEBUG_KP_TOOL_SELECTION && 1
            kdDebug () << "\t\tnormalizedRect=" << normalizedRect
                       << " usedRect=" << usefulRect
                       << " kpSelection::preferredMinimumSize="
                           << QSize (minimumWidth, minimumHeight)
                       << endl;
        #endif

            QValueVector <QString> textLines (1, QString ());
            kpSelection sel (usefulRect, textLines, textStyle);

            if (!m_currentCreateTextCommand)
            {
                m_currentCreateTextCommand = new kpToolSelectionCreateCommand (
                    i18n ("Text: Create Box"),
                    sel,
                    mainWindow ());
            }
            else
                m_currentCreateTextCommand->setFromSelection (sel);

            viewManager ()->setTextCursorPosition (0, 0);
            document ()->setSelection (sel);

            QPoint actualEndPoint = KP_INVALID_POINT;
            if (m_startPoint == usefulRect.topLeft ())
                actualEndPoint = usefulRect.bottomRight ();
            else if (m_startPoint == usefulRect.bottomRight ())
                actualEndPoint = usefulRect.topLeft ();
            else if (m_startPoint == usefulRect.topRight ())
                actualEndPoint = usefulRect.bottomLeft ();
            else if (m_startPoint == usefulRect.bottomLeft ())
                actualEndPoint = usefulRect.topRight ();

            setUserShapePoints (m_startPoint, actualEndPoint);
            break;
        }
        case kpToolSelection::Ellipse:
            document ()->setSelection (kpSelection (kpSelection::Ellipse, normalizedRect,
                                                    mainWindow ()->selectionTransparency ()));
            setUserShapePoints (m_startPoint, m_currentPoint);
            break;
        case kpToolSelection::FreeForm:
            QPointArray points;

            if (document ()->selection ())
                points = document ()->selection ()->points ();


            // (not detached so will modify "points" directly but
            //  still need to call kpDocument::setSelection() to
            //  update screen)

            if (!m_dragHasBegun)
            {
                // We thought the drag at startPoint was a NOP
                // but it turns out that it wasn't...
                points.putPoints (points.count (), 1, m_startPoint.x (), m_startPoint.y ());
            }

            // TODO: there should be an upper limit on this before drawing the
            //       polygon becomes too slow
            points.putPoints (points.count (), 1, thisPoint.x (), thisPoint.y ());


            document ()->setSelection (kpSelection (points, mainWindow ()->selectionTransparency ()));
        #if DEBUG_KP_TOOL_SELECTION && 0
            kdDebug () << "\t\tfreeform; #points=" << document ()->selection ()->points ().count () << endl;
        #endif

            setUserShapePoints (m_currentPoint);
            break;
        }

        viewManager ()->setSelectionBorderVisible (true);
    }
    else if (m_dragType == Move)
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
       kdDebug () << "\tmoving selection" << endl;
    #endif

        kpSelection *sel = document ()->selection ();

        QRect targetSelRect = QRect (thisPoint.x () - m_startDragFromSelectionTopLeft.x (),
                                     thisPoint.y () - m_startDragFromSelectionTopLeft.y (),
                                     sel->width (),
                                     sel->height ());

    #if DEBUG_KP_TOOL_SELECTION && 1
        kdDebug () << "\t\tstartPoint=" << m_startPoint
                   << " thisPoint=" << thisPoint
                   << " startDragFromSel=" << m_startDragFromSelectionTopLeft
                   << " targetSelRect=" << targetSelRect
                   << endl;
    #endif

        // Try to make sure selection still intersects document so that it's
        // reachable.

        if (targetSelRect.right () < 0)
            targetSelRect.moveBy (-targetSelRect.right (), 0);
        else if (targetSelRect.left () >= document ()->width ())
            targetSelRect.moveBy (document ()->width () - targetSelRect.left () - 1, 0);

        if (targetSelRect.bottom () < 0)
            targetSelRect.moveBy (0, -targetSelRect.bottom ());
        else if (targetSelRect.top () >= document ()->height ())
            targetSelRect.moveBy (0, document ()->height () - targetSelRect.top () - 1);

    #if DEBUG_KP_TOOL_SELECTION && 1
        kdDebug () << "\t\t\tafter ensure sel rect clickable=" << targetSelRect << endl;
    #endif


        if (!m_dragHasBegun &&
            targetSelRect.topLeft () + m_startDragFromSelectionTopLeft == m_startPoint)
        {
        #if DEBUG_KP_TOOL_SELECTION && 1
            kdDebug () << "\t\t\t\tnop" << endl;
        #endif

            // Prevent both NOP drag-moves
            setUserShapePoints (sel->topLeft ());
            return;
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

        if (m_shiftPressed)
            m_currentMoveCommandIsSmear = true;

        if (!m_dragHasBegun && (m_controlPressed || m_shiftPressed))
            m_currentMoveCommand->copyOntoDocument ();

        m_currentMoveCommand->moveTo (targetSelRect.topLeft ());

        if (m_shiftPressed)
            m_currentMoveCommand->copyOntoDocument ();

        //viewManager ()->restoreFastUpdates ();
        //viewManager ()->restoreQueueUpdates ();

        QPoint start = m_currentMoveCommand->originalSelection ().topLeft ();
        QPoint end = targetSelRect.topLeft ();
        setUserShapePoints (start, end, false/*don't set size*/);
        setUserShapeSize (end.x () - start.x (), end.y () - start.y ());
    }
    else if (m_dragType == ResizeScale)
    {
    #if DEBUG_KP_TOOL_SELECTION && 1
        kdDebug () << "\tresize/scale" << endl;
    #endif

        kpSelection *sel = document ()->selection ();

        if (!m_dragHasBegun && thisPoint == m_startPoint)
        {
        #if DEBUG_KP_TOOL_SELECTION && 1
            kdDebug () << "\t\tnop" << endl;
        #endif

            setUserShapePoints (QPoint (sel->width (), sel->height ()));
            return;
        }

        if (!sel->pixmap () && !m_currentPullFromDocumentCommand)
        {
            m_currentPullFromDocumentCommand = new kpToolSelectionPullFromDocumentCommand (
                QString::null/*uninteresting child of macro cmd*/,
                mainWindow ());
            m_currentPullFromDocumentCommand->execute ();
        }

        if (!m_currentResizeScaleCommand)
        {
            m_currentResizeScaleCommand = new kpToolSelectionResizeScaleCommand (mainWindow ());
        }

        kpSelection originalSelection = m_currentResizeScaleCommand->originalSelection ();
        int newWidth = originalSelection.width ();
        int newHeight = originalSelection.height ();
        int newX = originalSelection.x ();
        int newY = originalSelection.y ();

        if (m_resizeScaleType & kpSelection::Left)
        {
            newWidth = QMAX (1, newWidth - (thisPoint.x () - m_startPoint.x ()));
            newX -= (newWidth - originalSelection.width ());
        }
        else if (m_resizeScaleType & kpSelection::Right)
        {
            newWidth = QMAX (1, newWidth + (thisPoint.x () - m_startPoint.x ()));
        }

        if (m_resizeScaleType & kpSelection::Top)
        {
            newHeight = QMAX (1, newHeight - (thisPoint.y () - m_startPoint.y ()));
            newY -= (newHeight - originalSelection.height ());
        }
        else if (m_resizeScaleType & kpSelection::Bottom)
        {
            newHeight = QMAX (1, newHeight + (thisPoint.y () - m_startPoint.y ()));
        }

        if (sel->isText ())
        {
            // TODO: feels wrong - what about newX & newY
            newWidth = QMAX (sel->minimumWidthForTextStyle (sel->textStyle ()),
                             newWidth);
            newHeight = QMAX (sel->minimumHeightForTextStyle (sel->textStyle ()),
                              newHeight);
        }

        viewManager ()->setFastUpdates ();
        m_currentResizeScaleCommand->resizeAndMoveTo (newWidth, newHeight,
                                                      QPoint (newX, newY),
                                                      true/*smooth scale delayed*/);
        viewManager ()->restoreFastUpdates ();

        setUserShapePoints (QPoint (originalSelection.width (),
                                    originalSelection.height ()),
                            QPoint (newWidth,
                                    newHeight),
                            false/*don't set size*/);
        setUserShapeSize (newWidth - originalSelection.width (),
                          newHeight - originalSelection.height ());
    }

    m_dragHasBegun = true;
}

// virtual
void kpToolSelection::cancelShape ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::cancelShape() mouseButton=" << m_mouseButton << endl;
#endif

    if (m_mouseButton == 0/*left*/)
    {
        if (m_dragType == Move)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\twas drag moving - undo drag and undo acquire" << endl;
        #endif

            if (m_currentMoveCommand)
            {
            #if DEBUG_KP_TOOL_SELECTION
                kdDebug () << "\t\tundo currentMoveCommand" << endl;
            #endif
                m_currentMoveCommand->finalize ();
                m_currentMoveCommand->unexecute ();
                delete m_currentMoveCommand;
                m_currentMoveCommand = 0;

                if (document ()->selection ()->isText ())
                    viewManager ()->setTextCursorBlinkState (true);
            }
        }
        else if (m_dragType == Create)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\twas creating sel - kill" << endl;
        #endif

            // TODO: should we give the user back the selection s/he had before (if any)?
            document ()->selectionDelete ();

            if (m_currentCreateTextCommand)
            {
                delete m_currentCreateTextCommand;
                m_currentCreateTextCommand = 0;
            }
        }
        else if (m_dragType == ResizeScale)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\twas resize/scale sel - kill" << endl;
        #endif

            if (m_currentResizeScaleCommand)
            {
            #if DEBUG_KP_TOOL_SELECTION
                kdDebug () << "\t\tundo currentResizeScaleCommand" << endl;
            #endif
                m_currentResizeScaleCommand->unexecute ();
                delete m_currentResizeScaleCommand;
                m_currentResizeScaleCommand = 0;

                if (document ()->selection ()->isText ())
                    viewManager ()->setTextCursorBlinkState (true);
            }
        }


        if (m_currentPullFromDocumentCommand)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\t\tundo pullFromDocumentCommand" << endl;
        #endif
            m_currentPullFromDocumentCommand->unexecute ();
            delete m_currentPullFromDocumentCommand;
            m_currentPullFromDocumentCommand = 0;
        }


        viewManager ()->setSelectionBorderVisible (true);
        viewManager ()->setSelectionBorderFinished (true);
        viewManager ()->setTextCursorEnabled (m_mode == Text && true);
    }
    else
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tstarted draw with right button (which is banned)" << endl;
    #endif
    }

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

// virtual
void kpToolSelection::endDraw (const QPoint & /*thisPoint*/, const QRect & /*normalizedRect*/)
{
    if (m_mouseButton != 0/*left*/)
        return;

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
            cmd = new kpMacroCommand (i18n ("%1: Smear")
                                         .arg (document ()->selection ()->name ()),
                                      mainWindow ());
        }
        else
        {
            cmd = new kpMacroCommand ((document ()->selection ()->isText () ?
                                        i18n ("Text: Move Box") :
                                        i18n ("Selection: Move")),
                                      mainWindow ());
        }

        if (document ()->selection ()->isText ())
            viewManager ()->setTextCursorBlinkState (true);
    }
    else if (m_currentResizeScaleCommand)
    {
        cmd = new kpMacroCommand (m_currentResizeScaleCommand->kpNamedCommand::name (),
                                  mainWindow ());

        if (document ()->selection ()->isText ())
            viewManager ()->setTextCursorBlinkState (true);
    }

    if (m_currentPullFromDocumentCommand)
    {
        if (!m_currentMoveCommand && !m_currentResizeScaleCommand)
        {
            kdError () << "kpToolSelection::endDraw() pull without move nor resize/scale" << endl;
            delete m_currentPullFromDocumentCommand;
            m_currentPullFromDocumentCommand = 0;
        }
        else
        {
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

            if (kpToolSelectionCreateCommand::nextUndoCommandIsCreateBorder (commandHistory ()))
                commandHistory ()->setNextUndoCommand (createCommand);
            else
                commandHistory ()->addCommand (createCommand,
                                               false/*no exec - user already dragged out sel*/);


            cmd->addCommand (m_currentPullFromDocumentCommand);
            m_currentPullFromDocumentCommand = 0;
        }
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

    viewManager ()->setSelectionBorderVisible (true);
    viewManager ()->setSelectionBorderFinished (true);
    viewManager ()->setTextCursorEnabled (m_mode == Text && true);

    m_dragType = Unknown;
    setUserMessage (haventBegunDrawUserMessage ());
}


// protected virtual [base kpTool]
void kpToolSelection::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::keyPressEvent(e->text='" << e->text () << "')" << endl;
#endif


    e->ignore ();


    if (document ()->selection () &&
        !hasBegunDraw () &&
         e->key () == Qt::Key_Escape)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tescape pressed with sel when not begun draw - deselecting" << endl;
    #endif

        pushOntoDocument ();
        e->accept ();
    }


    if (!e->isAccepted ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tkey processing did not accept (text was '"
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
    kdDebug () << "kpToolSelection::selectionTransparencyChanged(" << name << ")" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\thave sel - set transparency" << endl;
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
            kdDebug () << "\t\twhich changed the pixmap" << endl;
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
    kdDebug () << "kpToolSelection::slotIsOpaqueChanged()" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\thave sel - set transparency" << endl;
    #endif

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
    }
}

// protected slot virtual [base kpTool]
void kpToolSelection::slotBackgroundColorChanged (const kpColor &)
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::slotBackgroundColorChanged()" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\thave sel - set transparency" << endl;
    #endif

        kpSelectionTransparency st = mainWindow ()->selectionTransparency ();
        kpSelectionTransparency oldST = st;
        oldST.setTransparentColor (oldBackgroundColor ());

        document ()->selection ()->setTransparency (st);
        commandHistory ()->addCommand (new kpToolSelectionTransparencyCommand (
            i18n ("Selection: Transparency Color"),
            st, oldST,
            mainWindow ()),
            false/* no exec*/);
    }
}

// protected slot virtual [base kpTool]
void kpToolSelection::slotColorSimilarityChanged (double, int)
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::slotColorSimilarityChanged()" << endl;
#endif

    if (mainWindow ()->settingSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\trecursion - abort setting selection transparency: "
                   << mainWindow ()->settingSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\thave sel - set transparency" << endl;
    #endif

        kpSelectionTransparency st = mainWindow ()->selectionTransparency ();
        kpSelectionTransparency oldST = st;
        oldST.setColorSimilarity (oldColorSimilarity ());

        document ()->selection ()->setTransparency (st);
        commandHistory ()->addCommand (new kpToolSelectionTransparencyCommand (
            i18n ("Selection: Transparency Color Similarity"),
            st, oldST,
            mainWindow ()),
            false/* no exec*/);
    }
}


/*
 * kpToolSelectionCreateCommand
 */

kpToolSelectionCreateCommand::kpToolSelectionCreateCommand (const QString &name,
                                                            const kpSelection &fromSelection,
                                                            kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_fromSelection (0),
      m_textRow (0), m_textCol (0)
{
    setFromSelection (fromSelection);
}

kpToolSelectionCreateCommand::~kpToolSelectionCreateCommand ()
{
    delete m_fromSelection;
}


// public virtual [base kpCommand]
int kpToolSelectionCreateCommand::size () const
{
    return kpPixmapFX::selectionSize (m_fromSelection);
}


// public static
bool kpToolSelectionCreateCommand::nextUndoCommandIsCreateBorder (
    kpCommandHistory *commandHistory)
{
    if (!commandHistory)
        return false;

    kpCommand *cmd = commandHistory->nextUndoCommand ();
    if (!cmd)
        return false;

    kpToolSelectionCreateCommand *c = dynamic_cast <kpToolSelectionCreateCommand *> (cmd);
    if (!c)
        return false;

    const kpSelection *sel = c->fromSelection ();
    if (!sel)
        return false;

    return (!sel->pixmap ());
}


// public
const kpSelection *kpToolSelectionCreateCommand::fromSelection () const
{
    return m_fromSelection;
}

// public
void kpToolSelectionCreateCommand::setFromSelection (const kpSelection &fromSelection)
{
    delete m_fromSelection;
    m_fromSelection = new kpSelection (fromSelection);
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionCreateCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionCreateCommand::execute() without doc" << endl;
        return;
    }

    if (m_fromSelection)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tusing fromSelection" << endl;
        kdDebug () << "\t\thave sel=" << doc->selection ()
                   << " pixmap=" << (doc->selection () ? doc->selection ()->pixmap () : 0)
                   << endl;
    #endif
        if (!m_fromSelection->isText ())
        {
            if (m_fromSelection->transparency () != m_mainWindow->selectionTransparency ())
                m_mainWindow->setSelectionTransparency (m_fromSelection->transparency ());
        }
        else
        {
            if (m_fromSelection->textStyle () != m_mainWindow->textStyle ())
                m_mainWindow->setTextStyle (m_fromSelection->textStyle ());
        }

        m_mainWindow->viewManager ()->setTextCursorPosition (m_textRow, m_textCol);
        doc->setSelection (*m_fromSelection);

        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionCreateCommand::unexecute() without doc" << endl;
        return;
    }

    if (!doc->selection ())
    {
        kdError () << "kpToolSelectionCreateCommand::unexecute() without sel region" << endl;
        return;
    }

    m_textRow = m_mainWindow->viewManager ()->textCursorRow ();
    m_textCol = m_mainWindow->viewManager ()->textCursorCol ();

    doc->selectionDelete ();

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
}


/*
 * kpToolSelectionPullFromDocumentCommand
 */

kpToolSelectionPullFromDocumentCommand::kpToolSelectionPullFromDocumentCommand (const QString &name,
                                                                                kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : kpColor::invalid),
      m_originalSelectionRegion (0)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionPullFromDocumentCommand::<ctor>() mainWindow="
               << m_mainWindow
               << endl;
#endif
}

kpToolSelectionPullFromDocumentCommand::~kpToolSelectionPullFromDocumentCommand ()
{
    delete m_originalSelectionRegion;
}


// public virtual [base kpCommand]
int kpToolSelectionPullFromDocumentCommand::size () const
{
    return kpPixmapFX::selectionSize (m_originalSelectionRegion);
}


// public virtual [base kpCommand]
void kpToolSelectionPullFromDocumentCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionPullFromDocumentCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();

    if (!doc)
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::execute() without doc" << endl;
        return;
    }

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;
    if (vm)
        vm->setQueueUpdates ();

    // In case the user CTRL+Z'ed, selected a random region to throw us off
    // and then CTRL+Shift+Z'ed putting us here.  Make sure we pull from the
    // originally requested region - not the random one.
    if (m_originalSelectionRegion)
    {
        if (m_originalSelectionRegion->transparency () != m_mainWindow->selectionTransparency ())
            m_mainWindow->setSelectionTransparency (m_originalSelectionRegion->transparency ());

        doc->setSelection (*m_originalSelectionRegion);
    }
    else
    {
        // must have selection region but not pixmap
        if (!doc->selection () || doc->selection ()->pixmap ())
        {
            kdError () << "kpToolSelectionPullFromDocumentCommand::execute() sel="
                       << doc->selection ()
                       << " pixmap="
                       << (doc->selection () ? doc->selection ()->pixmap () : 0)
                       << endl;
            if (vm)
                vm->restoreQueueUpdates ();
            return;
        }
    }

    doc->selectionPullFromDocument (m_backgroundColor);

    if (vm)
        vm->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpToolSelectionPullFromDocumentCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionPullFromDocumentCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();

    if (!doc)
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::unexecute() without doc" << endl;
        return;
    }

    // must have selection pixmap
    if (!doc->selection () || !doc->selection ()->pixmap ())
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::unexecute() sel="
                   << doc->selection ()
                   << " pixmap="
                   << (doc->selection () ? doc->selection ()->pixmap () : 0)
                   << endl;
        return;
    }


    // We can have faith that this is the state of the selection after
    // execute(), rather than after the user tried to throw us off by
    // simply selecting another region as to do that, a destroy command
    // must have been used.
    doc->selectionCopyOntoDocument (false/*use opaque pixmap*/);
    doc->selection ()->setPixmap (QPixmap ());

    delete m_originalSelectionRegion;
    m_originalSelectionRegion = new kpSelection (*doc->selection ());
}


/*
 * kpToolSelectionTransparencyCommand
 */

kpToolSelectionTransparencyCommand::kpToolSelectionTransparencyCommand (const QString &name,
    const kpSelectionTransparency &st,
    const kpSelectionTransparency &oldST,
    kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_st (st),
      m_oldST (oldST)
{
}

kpToolSelectionTransparencyCommand::~kpToolSelectionTransparencyCommand ()
{
}


// public virtual [base kpCommand]
int kpToolSelectionTransparencyCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolSelectionTransparencyCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionTransparencyCommand::execute()" << endl;
#endif
    kpDocument *doc = document ();
    if (!doc)
        return;

    m_mainWindow->setSelectionTransparency (m_st, true/*force colour change*/);

    if (doc->selection ())
        doc->selection ()->setTransparency (m_st);
}

// public virtual [base kpCommand]
void kpToolSelectionTransparencyCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionTransparencyCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    m_mainWindow->setSelectionTransparency (m_oldST, true/*force colour change*/);

    if (doc->selection ())
        doc->selection ()->setTransparency (m_oldST);
}


/*
 * kpToolSelectionMoveCommand
 */

kpToolSelectionMoveCommand::kpToolSelectionMoveCommand (const QString &name,
                                                        kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow)
{
    kpDocument *doc = document ();
    if (doc && doc->selection ())
    {
        m_startPoint = m_endPoint = doc->selection ()->topLeft ();
    }
}

kpToolSelectionMoveCommand::~kpToolSelectionMoveCommand ()
{
}


// public
kpSelection kpToolSelectionMoveCommand::originalSelection () const
{
    kpDocument *doc = document ();
    if (!doc || !doc->selection ())
    {
        kdError () << "kpToolSelectionMoveCommand::originalSelection() doc="
                   << doc
                   << " sel="
                   << (doc ? doc->selection () : 0)
                   << endl;
        return kpSelection (kpSelection::Rectangle, QRect ());
    }

    kpSelection selection = *doc->selection();
    selection.moveTo (m_startPoint);

    return selection;
}


// public virtual [base kpComand]
int kpToolSelectionMoveCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldDocumentPixmap) +
           kpPixmapFX::pointArraySize (m_copyOntoDocumentPoints);
}


// public virtual [base kpCommand]
void kpToolSelectionMoveCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionMoveCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionMoveCommand::execute() no doc" << endl;
        return;
    }

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel || !sel->pixmap ())
    {
        kdError () << "kpToolSelectionMoveCommand::execute() but haven't pulled pixmap yet: "
                   << "sel=" << sel << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
                   << endl;
        return;
    }

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    if (vm)
        vm->setQueueUpdates ();

    QPointArray::ConstIterator copyOntoDocumentPointsEnd = m_copyOntoDocumentPoints.end ();
    for (QPointArray::ConstIterator it = m_copyOntoDocumentPoints.begin ();
         it != copyOntoDocumentPointsEnd;
         it++)
    {
        sel->moveTo (*it);
        doc->selectionCopyOntoDocument ();
    }

    sel->moveTo (m_endPoint);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    if (vm)
        vm->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpToolSelectionMoveCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionMoveCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionMoveCommand::unexecute() no doc" << endl;
        return;
    }

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel || !sel->pixmap ())
    {
        kdError () << "kpToolSelectionMoveCommand::unexecute() but haven't pulled pixmap yet: "
                   << "sel=" << sel << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
                   << endl;
        return;
    }

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    if (vm)
        vm->setQueueUpdates ();

    if (!m_oldDocumentPixmap.isNull ())
        doc->setPixmapAt (m_oldDocumentPixmap, m_documentBoundingRect.topLeft ());
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "\tmove to startPoint=" << m_startPoint << endl;
#endif
    sel->moveTo (m_startPoint);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    if (vm)
        vm->restoreQueueUpdates ();
}

// public
void kpToolSelectionMoveCommand::moveTo (const QPoint &point, bool moveLater)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    kdDebug () << "kpToolSelectionMoveCommand::moveTo" << point
               << " moveLater=" << moveLater
               <<endl;
#endif

    if (!moveLater)
    {
        kpDocument *doc = document ();
        if (!doc)
        {
            kdError () << "kpToolSelectionMoveCommand::moveTo() without doc" << endl;
            return;
        }

        kpSelection *sel = doc->selection ();

        // have to have pulled pixmap by now
        if (!sel)
        {
            kdError () << "kpToolSelectionMoveCommand::moveTo() no sel region" << endl;
            return;
        }

        if (!sel->pixmap ())
        {
            kdError () << "kpToolSelectionMoveCommand::moveTo() no sel pixmap" << endl;
            return;
        }

        if (point == sel->topLeft ())
            return;

        sel->moveTo (point);
    }

    m_endPoint = point;
}

// public
void kpToolSelectionMoveCommand::moveTo (int x, int y, bool moveLater)
{
    moveTo (QPoint (x, y), moveLater);
}

// public
void kpToolSelectionMoveCommand::copyOntoDocument ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionMoveCommand::copyOntoDocument()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel)
    {
        kdError () << "\tkpToolSelectionMoveCommand::copyOntoDocument() without sel region" << endl;
        return;
    }

    if (!sel->pixmap ())
    {
        kdError () << "kpToolSelectionMoveCommand::moveTo() no sel pixmap" << endl;
        return;
    }

    if (m_oldDocumentPixmap.isNull ())
        m_oldDocumentPixmap = *doc->pixmap ();

    QRect selBoundingRect = sel->boundingRect ();
    m_documentBoundingRect.unite (selBoundingRect);

    doc->selectionCopyOntoDocument ();

    m_copyOntoDocumentPoints.putPoints (m_copyOntoDocumentPoints.count (),
                                        1,
                                        selBoundingRect.x (),
                                        selBoundingRect.y ());
}

// public
void kpToolSelectionMoveCommand::finalize ()
{
    if (!m_oldDocumentPixmap.isNull () && !m_documentBoundingRect.isNull ())
    {
        m_oldDocumentPixmap = kpTool::neededPixmap (m_oldDocumentPixmap,
                                                    m_documentBoundingRect);
    }
}


/*
 * kpToolSelectionResizeScaleCommand
 */

kpToolSelectionResizeScaleCommand::kpToolSelectionResizeScaleCommand (
        kpMainWindow *mainWindow)
    : kpNamedCommand (mainWindow->document ()->selection ()->isText () ?
                         i18n ("Text: Resize Box") :
                         i18n ("Selection: Smooth Scale"),
                      mainWindow),
      m_smoothScaleTimer (new QTimer (this))
{
    m_originalSelection = *selection ();

    m_newTopLeft = selection ()->topLeft ();
    m_newWidth = selection ()->width ();
    m_newHeight = selection ()->height ();

    connect (m_smoothScaleTimer, SIGNAL (timeout ()),
             this, SLOT (resizeScaleAndMove ()));
}

kpToolSelectionResizeScaleCommand::~kpToolSelectionResizeScaleCommand ()
{
}


// public virtual
int kpToolSelectionResizeScaleCommand::size () const
{
    return m_originalSelection.size ();
}


// public
kpSelection kpToolSelectionResizeScaleCommand::originalSelection () const
{
    return m_originalSelection;
}


// public
QPoint kpToolSelectionResizeScaleCommand::topLeft () const
{
    return m_newTopLeft;
}

// public
void kpToolSelectionResizeScaleCommand::moveTo (const QPoint &point)
{
    if (point == m_newTopLeft)
        return;

    m_newTopLeft = point;
    selection ()->moveTo (m_newTopLeft);
}


// public
int kpToolSelectionResizeScaleCommand::width () const
{
    return m_newWidth;
}

// public
int kpToolSelectionResizeScaleCommand::height () const
{
    return m_newHeight;
}

// public
void kpToolSelectionResizeScaleCommand::resize (int width, int height,
                                                bool delayed)
{
    if (width == m_newWidth && height == m_newHeight)
        return;

    m_newWidth = width;
    m_newHeight = height;

    resizeScaleAndMove (delayed);
}


// public
void kpToolSelectionResizeScaleCommand::resizeAndMoveTo (int width, int height,
                                                         const QPoint &point,
                                                         bool delayed)
{
    if (width == m_newWidth && height == m_newHeight &&
        point == m_newTopLeft)
    {
        return;
    }

    m_newWidth = width;
    m_newHeight = height;
    m_newTopLeft = point;

    resizeScaleAndMove (delayed);
}


// protected
void kpToolSelectionResizeScaleCommand::killSmoothScaleTimer ()
{
    m_smoothScaleTimer->stop ();
}


// protected
void kpToolSelectionResizeScaleCommand::resizeScaleAndMove (bool delayed)
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionResizeScaleCommand::resizeScaleAndMove(delayed="
               << delayed << ")" << endl;
#endif

    killSmoothScaleTimer ();

    kpSelection newSel;

    if (selection ()->isText ())
    {
        newSel = m_originalSelection;
        newSel.textResize (m_newWidth, m_newHeight);
    }
    else
    {
        newSel = kpSelection (kpSelection::Rectangle,
            QRect (m_originalSelection.x (),
                   m_originalSelection.y (),
                   m_newWidth,
                   m_newHeight),
            kpPixmapFX::scale (*m_originalSelection.pixmap (),
                               m_newWidth, m_newHeight,
                               !delayed/*if not delayed, smooth*/),
            m_originalSelection.transparency ());

        if (delayed)
        {
            // Call self with delayed==false in 200ms
            m_smoothScaleTimer->start (200/*ms*/, true/*single shot*/);
        }
    }

    newSel.moveTo (m_newTopLeft);

    m_mainWindow->document ()->setSelection (newSel);
}

// protected slots
void kpToolSelectionResizeScaleCommand::resizeScaleAndMove ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionResizeScaleCommand::resizeScaleAndMove()" << endl;
#endif
    resizeScaleAndMove (false/*no delay*/);
}


// public virtual [base kpToolResizeScaleCommand]
void kpToolSelectionResizeScaleCommand::execute ()
{
    QApplication::setOverrideCursor (Qt::waitCursor);

    killSmoothScaleTimer ();

    resizeScaleAndMove ();

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpToolResizeScaleCommand]
void kpToolSelectionResizeScaleCommand::unexecute ()
{
    QApplication::setOverrideCursor (Qt::waitCursor);

    killSmoothScaleTimer ();

    m_mainWindow->document ()->setSelection (m_originalSelection);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    QApplication::restoreOverrideCursor ();
}


/*
 * kpToolSelectionDestroyCommand
 */

kpToolSelectionDestroyCommand::kpToolSelectionDestroyCommand (const QString &name,
                                                              bool pushOntoDocument,
                                                              kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_pushOntoDocument (pushOntoDocument),
      m_oldSelection (0)
{
}

kpToolSelectionDestroyCommand::~kpToolSelectionDestroyCommand ()
{
    delete m_oldSelection;
}


// public virtual [base kpCommand]
int kpToolSelectionDestroyCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldDocPixmap) +
           kpPixmapFX::selectionSize (m_oldSelection);
}


// public virtual [base kpCommand]
void kpToolSelectionDestroyCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionDestroyCommand::execute () CALLED" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionDestroyCommand::execute() without doc" << endl;
        return;
    }

    if (!doc->selection ())
    {
        kdError () << "kpToolSelectionDestroyCommand::execute() without sel region" << endl;
        return;
    }

    m_textRow = m_mainWindow->viewManager ()->textCursorRow ();
    m_textCol = m_mainWindow->viewManager ()->textCursorCol ();

    m_oldSelection = new kpSelection (*doc->selection ());
    if (m_pushOntoDocument)
    {
        m_oldDocPixmap = doc->getPixmapAt (doc->selection ()->boundingRect ());
        doc->selectionPushOntoDocument ();
    }
    else
        doc->selectionDelete ();

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
}

// public virtual [base kpCommand]
void kpToolSelectionDestroyCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionDestroyCommand::unexecute () CALLED" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionDestroyCommand::unexecute() without doc" << endl;
        return;
    }

    if (doc->selection ())
    {
        // not error because it's possible that the user dragged out a new
        // region (without pulling pixmap), and then CTRL+Z
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "kpToolSelectionDestroyCommand::unexecute() already has sel region" << endl;
    #endif

        if (doc->selection ()->pixmap ())
        {
            kdError () << "kpToolSelectionDestroyCommand::unexecute() already has sel pixmap" << endl;
            return;
        }
    }

    if (!m_oldSelection)
    {
        kdError () << "kpToolSelectionDestroyCommand::unexecute() without old sel" << endl;
        return;
    }

    if (m_pushOntoDocument)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tunpush oldDocPixmap onto doc first" << endl;
    #endif
        doc->setPixmapAt (m_oldDocPixmap, m_oldSelection->topLeft ());
    }

#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "\tsetting selection to: rect=" << m_oldSelection->boundingRect ()
               << " pixmap=" << m_oldSelection->pixmap ()
               << " pixmap.isNull()=" << (m_oldSelection->pixmap ()
                                              ?
                                          m_oldSelection->pixmap ()->isNull ()
                                              :
                                          true)
               << endl;
#endif
    if (!m_oldSelection->isText ())
    {
        if (m_oldSelection->transparency () != m_mainWindow->selectionTransparency ())
            m_mainWindow->setSelectionTransparency (m_oldSelection->transparency ());
    }
    else
    {
        if (m_oldSelection->textStyle () != m_mainWindow->textStyle ())
            m_mainWindow->setTextStyle (m_oldSelection->textStyle ());
    }

    m_mainWindow->viewManager ()->setTextCursorPosition (m_textRow, m_textCol);
    doc->setSelection (*m_oldSelection);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    delete m_oldSelection;
    m_oldSelection = 0;
}

#include <kptoolselection.moc>
