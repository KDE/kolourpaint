
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

#define DEBUG_KP_TOOL_TEXT 0


#include "tools/selection/text/kpToolText.h"
#include "kpToolTextPrivate.h"

#include <QList>

#include <KLocalizedString>
#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "views/manager/kpViewManager.h"


// protected virtual [kpAbstractSelectionTool]
QString kpToolText::haventBegunDrawUserMessageCreate () const
{
    return i18n ("Left drag to create text box.");
}


// protected virtual [base kpAbstractSelectionTool]
void kpToolText::setSelectionBorderForBeginDrawCreate ()
{
    viewManager ()->setQueueUpdates ();
    {
        kpAbstractSelectionTool::setSelectionBorderForBeginDrawCreate ();
        viewManager ()->setTextCursorEnabled (false);
    }
    viewManager ()->restoreQueueUpdates ();
}


// private
int kpToolText::calcClickCreateDimension (int mouseStart, int mouseEnd,
    int preferredMin, int smallestMin,
    int docSize)
{
    Q_ASSERT (preferredMin >= smallestMin);
    Q_ASSERT (docSize > 0);

    // Get reasonable width/height for a text box.
    int ret = preferredMin;

    // X or Y increasing?
    if (mouseEnd >= mouseStart)
    {
        // Text box extends past document width/height?
        if (mouseStart + ret - 1 >= docSize)
        {
            // Cap width/height to not extend past but not below smallest
            // possible selection width/height
            ret = qMax (smallestMin, docSize - mouseStart);
        }
    }
    // X or Y decreasing
    else
    {
        // Text box extends past document start?
        // TODO: I doubt this code can be invoked for a click.
        //       Maybe very tricky interplay with accidental drag detection?
        if (mouseStart - ret + 1 < 0)
        {
            // Cap width/height to not extend past but not below smallest
            // possible selection width/height.
            ret = qMax (smallestMin, mouseStart + 1);
        }
    }

    return ret;
}

// private
bool kpToolText::shouldCreate (bool dragAccepted,
        const QPoint &accidentalDragAdjustedPoint,
        const kpTextStyle &textStyle,
        int *minimumWidthOut, int *minimumHeightOut,
        bool *newDragAccepted)
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "CALL(dragAccepted=" << dragAccepted
              << ",accidentalDragAdjustedPoint=" << accidentalDragAdjustedPoint
              << ")";
#endif
    *newDragAccepted = dragAccepted;

    // Is the drag so short that we're essentially just clicking?
    // Basically, we're trying to prevent unintentional creation of 1-pixel
    // selections.
    if (!dragAccepted && accidentalDragAdjustedPoint == startPoint ())
    {
        // We had an existing text box before the click?
        if (hadSelectionBeforeDraw ())
        {
        #if DEBUG_KP_TOOL_TEXT && 1
            qCDebug(kpLogTools) << "\ttext box deselect - NOP - return";
        #endif
            // We must be attempting to deselect the text box.
            // This deselection has already been done by kpAbstractSelectionTool::beginDraw().
            // Therefore, we are not doing a drag.
            return false;
        }
        // We are probably creating a new box.


        // This drag is currently a click -- not a drag.
        // As a special case, allow user to create a text box,
        // of reasonable ("preferred minimum") size, using a single
        // click.
        //
        // If the user drags further, the normal drag-to-create-a-textbox
        // branch [x] will execute and the size will be determined based on
        // the size of the drag instead.

#if DEBUG_KP_TOOL_TEXT && 1
        qCDebug(kpLogTools) << "\tclick creating text box";
#endif

        // (Click creating text box with RMB would not be obvious
        //  since RMB menu most likely hides text box immediately
        //  afterwards)
        // TODO: I suspect this logic is simply too late
        // TODO: We setUserShapePoints() on return but didn't before.
        if (mouseButton () == 1) {
            return false/*do not create text box*/;
        }


        // Calculate suggested width.
        *minimumWidthOut = calcClickCreateDimension (
                    startPoint ().x (),
                    accidentalDragAdjustedPoint.x (),
                    kpTextSelection::PreferredMinimumWidthForTextStyle (textStyle),
                    kpTextSelection::MinimumWidthForTextStyle (textStyle),
                    document ()->width ());

        // Calculate suggested height.
        *minimumHeightOut = calcClickCreateDimension (
                    startPoint ().y (),
                    accidentalDragAdjustedPoint.y (),
                    kpTextSelection::PreferredMinimumHeightForTextStyle (textStyle),
                    kpTextSelection::MinimumHeightForTextStyle (textStyle),
                    document ()->height ());


        // Do _not_ set "newDragAccepted" to true as we want
        // this text box to remain at the click-given size, in the absence
        // of any dragging.  In other words, if draw() is called again
        // and therefore, we are called again, but the mouse has not
        // moved, we do want this branch to execute again, not
        // Branch [x].
        return true/*do create text box*/;
    }
    // Dragging to create a text box [x].
    //
    // The size will be determined based on the size of the drag.


#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "\tdrag creating text box";
#endif
    *minimumWidthOut = kpTextSelection::MinimumWidthForTextStyle (textStyle);
    *minimumHeightOut = kpTextSelection::MinimumHeightForTextStyle (textStyle);

    *newDragAccepted = true;
    return true/*do create text box*/;
}

// protected virtual [kpAbstractSelectionTool]
bool kpToolText::drawCreateMoreSelectionAndUpdateStatusBar (
        bool dragAccepted,
        const QPoint &accidentalDragAdjustedPoint,
        const QRect &normalizedRectIn)
{
    // (will mutate this)
    QRect normalizedRect = normalizedRectIn;

    const kpTextStyle textStyle = environ ()->textStyle ();


    //
    // Calculate Text Box Rectangle.
    //

    bool newDragAccepted = dragAccepted;

    // (will set both variables)
    int minimumWidth = 0, minimumHeight = 0;
    if (!shouldCreate (dragAccepted, accidentalDragAdjustedPoint, textStyle,
            &minimumWidth, &minimumHeight, &newDragAccepted))
    {
        setUserShapePoints (accidentalDragAdjustedPoint);
        return newDragAccepted;
    }


    // Make sure the dragged out rectangle is of the minimum width we just
    // calculated.
    if (normalizedRect.width () < minimumWidth)
    {
        if (accidentalDragAdjustedPoint.x () >= startPoint ().x ()) {
            normalizedRect.setWidth (minimumWidth);
        }
        else {
            normalizedRect.setX (normalizedRect.right () - minimumWidth + 1);
        }
    }

    // Make sure the dragged out rectangle is of the minimum height we just
    // calculated.
    if (normalizedRect.height () < minimumHeight)
    {
        if (accidentalDragAdjustedPoint.y () >= startPoint ().y ()) {
            normalizedRect.setHeight (minimumHeight);
        }
        else {
            normalizedRect.setY (normalizedRect.bottom () - minimumHeight + 1);
        }
    }

#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "\t\tnormalizedRect=" << normalizedRect
                << " kpTextSelection::preferredMinimumSize="
                    << QSize (minimumWidth, minimumHeight);
#endif


    //
    // Construct and Deploy Text Box.
    //

    // Create empty text box.
    QList <QString> textLines;
    kpTextSelection textSel (normalizedRect, textLines, textStyle);

    // Render.
    viewManager ()->setTextCursorPosition (0, 0);
    document ()->setSelection (textSel);


    //
    // Update Status Bar.
    //

    QPoint actualEndPoint = KP_INVALID_POINT;
    if (startPoint () == normalizedRect.topLeft ()) {
        actualEndPoint = normalizedRect.bottomRight ();
    }
    else if (startPoint () == normalizedRect.bottomRight ()) {
        actualEndPoint = normalizedRect.topLeft ();
    }
    else if (startPoint () == normalizedRect.topRight ()) {
        actualEndPoint = normalizedRect.bottomLeft ();
    }
    else if (startPoint () == normalizedRect.bottomLeft ()) {
        actualEndPoint = normalizedRect.topRight ();
    }

    setUserShapePoints (startPoint (), actualEndPoint);

    return newDragAccepted;
}

