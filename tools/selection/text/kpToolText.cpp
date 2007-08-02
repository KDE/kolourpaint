
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


#include <kpToolText.h>

#include <qevent.h>
#include <qlist.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpBug.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpTextSelection.h>
#include <kpToolTextBackspaceCommand.h>
#include <kpToolTextChangeStyleCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionEnvironment.h>
#include <kpToolTextDeleteCommand.h>
#include <kpToolTextEnterCommand.h>
#include <kpToolTextInsertCommand.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


kpToolText::kpToolText (kpToolSelectionEnvironment *environ, QObject *parent)
    : kpToolSelection (Text,
                       i18n ("Text"), i18n ("Writes text"),
                       Qt::Key_T,
                       environ, parent, "tool_text"),
      m_isIMStarted (false),
      m_IMStartCursorRow (0),
      m_IMStartCursorCol (0)
{
}

kpToolText::~kpToolText ()
{
}


// private
bool kpToolText::onSelectionToSelectText () const
{
    kpView *v = viewManager ()->viewUnderCursor ();
    if (!v)
        return 0;

    return v->mouseOnSelectionToSelectText (currentViewPoint ());
}


// protected virtual [base kpToolSelection]
QString kpToolText::haventBegunDrawUserMessageOnResizeHandle () const
{
    return i18n ("Left drag to resize text box.");
}

// protected virtual [base kpToolSelection]
QString kpToolText::haventBegunDrawUserMessageInsideSelection () const
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
        return i18n ("Left click to change cursor position.");
    else
        return i18n ("Left drag to move text box.");
}

// protected virtual [base kpToolSelection]
QString kpToolText::haventBegunDrawUserMessageOutsideSelection () const
{
    return i18n ("Left drag to create text box.");
}


//
// Command Handling
//


// protected
void kpToolText::setAllCommandPointersToZero ()
{
    m_insertCommand = 0;
    m_enterCommand = 0;

    m_backspaceCommand = 0;
    m_backspaceWordCommand = 0;

    m_deleteCommand = 0;
    m_deleteWordCommand = 0;
}


// protected
void kpToolText::addNewBackspaceCommand (kpToolTextBackspaceCommand **cmd)
{
    if (hasBegunShape ())
    {
        endShape (currentPoint (),
            kpBug::QRect_Normalized (QRect (startPoint (), currentPoint ())));
    }

    *cmd = new kpToolTextBackspaceCommand (i18n ("Text: Backspace"),
                viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                kpToolTextBackspaceCommand::DontAddBackspaceYet,
                environ ()->commandEnvironment ());
    commandHistory ()->addCommand (*cmd, false/*no exec*/);
}

// protected
void kpToolText::addNewDeleteCommand (kpToolTextDeleteCommand **cmd)
{
    if (hasBegunShape ())
    {
        endShape (currentPoint (),
            kpBug::QRect_Normalized (QRect (startPoint (), currentPoint ())));
    }

    *cmd = new kpToolTextDeleteCommand (i18n ("Text: Delete"),
                viewManager ()->textCursorRow (), viewManager ()->textCursorCol (),
                kpToolTextDeleteCommand::DontAddDeleteYet,
                environ ()->commandEnvironment ());
    commandHistory ()->addCommand (*cmd, false/*no exec*/);
}


// public virtual [base kpToolSelection]
void kpToolText::begin ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::begin()";
#endif

    environ ()->enableTextToolBarActions (true);
    viewManager ()->setTextCursorEnabled (true);

    setAllCommandPointersToZero ();

    kpToolSelection::begin ();
}

// public virtual [base kpToolSelection]
void kpToolText::end ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "kpToolText::end()";
#endif

    kpToolSelection::end ();

    viewManager ()->setTextCursorEnabled (false);
    environ ()->enableTextToolBarActions (false);
}


// public
bool kpToolText::hasBegunText () const
{
    return (m_insertCommand ||
            m_enterCommand ||
            m_backspaceCommand ||
            m_backspaceWordCommand ||
            m_deleteCommand ||
            m_deleteWordCommand);
}

// public virtual [base kpTool]
bool kpToolText::hasBegunShape () const
{
    return (hasBegunDraw () || hasBegunText ());
}


// protected virtual [base kpToolSelection]
kpToolSelection::DragType kpToolText::beginDrawInsideSelection ()
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
    {
        // This path is a bit dangerous since we don't call the base
        // implementation.
        //
        // However, we are doing something unusual here in that we aren't
        // drag-moving the selection - therefore it makes sense to not
        // call the base.
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\t\tis select cursor pos";
    #endif

        Q_ASSERT (document ()->textSelection ());
        viewManager ()->setTextCursorPosition (
            document ()->textSelection ()->closestTextRowForPoint (currentPoint ()),
            document ()->textSelection ()->closestTextColForPoint (currentPoint ()));

        return kpToolSelection::SelectText;
    }

    return kpToolSelection::beginDrawInsideSelection ();
}

// protected virtual [base kpToolSelection]
QCursor kpToolText::cursorInsideSelection () const
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
        return Qt::IBeamCursor;

    return kpToolSelection::cursorInsideSelection ();
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
bool kpToolText::shouldCreate (bool dragHasBegun,
        const QPoint &accidentalDragAdjustedPoint,
        const kpTextStyle &textStyle,
        int *minimumWidthOut, int *minimumHeightOut,
        bool *newDragHasBegun)
{
    *newDragHasBegun = dragHasBegun;

    // Is the drag so short that we're essentially just clicking?
    // Basically, we're trying to prevent unintentional creation of 1-pixel
    // selections.
    if (!dragHasBegun && accidentalDragAdjustedPoint == startPoint ())
    {
        // We had an existing text box before the click?
        if (m_hadSelectionBeforeDrag)
        {
        #if DEBUG_KP_TOOL_TEXT && 1
            kDebug () << "\ttext box deselect - NOP - return";
        #endif
            // We must be attempting to deselect the text box.
            // This deselection has already been done by kpToolSelection::beginDraw().
            // Therefore, we are not doing a drag.
            return false;
        }
        // We must be creating a new box.
        else
        {
            // This drag is currently a click - not a drag.
            //
            // However, as a special case, allow user to create a text box using a single
            // click.  But don't set newDragHasBegun since it would be untrue.
            //
            // This makes sure that a single click creation of text box
            // works even if draw() is invoked more than once at the
            // same position (esp. with accidental drag suppression
            // (above)).

            //
            // User is possibly clicking to create a text box.
            //
            // Create a text box of reasonable ("preferred minimum") size.
            //
            // However, if it turns out that this is just the start of the drag,
            // we will be called again but the above code will execute instead,
            // ignoring this resonable size.
            //

        #if DEBUG_KP_TOOL_TEXT && 1
            kDebug () << "\tclick creating text box";
        #endif

            // (Click creating text box with RMB would not be obvious
            //  since RMB menu most likely hides text box immediately
            //  afterwards)
            // TODO: I suspect this logic is simply too late
            // TODO: We setUserShapePoints() on return but didn't before.
            if (mouseButton () == 1)
                return false/*do not create text box*/;


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


            return true/*do create text box*/;
        }
    }
    else
    {
    #if DEBUG_KP_TOOL_TEXT && 1
        kDebug () << "\tdrag creating text box";
    #endif
        *minimumWidthOut = kpTextSelection::MinimumWidthForTextStyle (textStyle);
        *minimumHeightOut = kpTextSelection::MinimumHeightForTextStyle (textStyle);
        *newDragHasBegun = true;
        return true/*do create text box*/;
    }

}

// protected virtual [base kpToolSelection]
bool kpToolText::createMoreSelectionAndUpdateStatusBar (
        bool dragHasBegun,
        const QPoint &accidentalDragAdjustedPoint,
        const QRect &normalizedRectIn)
{
    // (will mutate this)
    QRect normalizedRect = normalizedRectIn;

    const kpTextStyle textStyle = environ ()->textStyle ();


    //
    // Calculate Text Box Rectangle.
    //

    bool newDragHasBegun = dragHasBegun;

    // (will set both variables)
    int minimumWidth = 0, minimumHeight = 0;
    if (!shouldCreate (dragHasBegun, accidentalDragAdjustedPoint, textStyle,
            &minimumWidth, &minimumHeight, &newDragHasBegun))
    {
        setUserShapePoints (accidentalDragAdjustedPoint);
        return newDragHasBegun;
    }


    // Make sure the dragged out rectangle is of the minimum width we just
    // calculated.
    if (normalizedRect.width () < minimumWidth)
    {
        if (accidentalDragAdjustedPoint.x () >= startPoint ().x ())
            normalizedRect.setWidth (minimumWidth);
        else
            normalizedRect.setX (normalizedRect.right () - minimumWidth + 1);
    }

    // Make sure the dragged out rectangle is of the minimum height we just
    // calculated.
    if (normalizedRect.height () < minimumHeight)
    {
        if (accidentalDragAdjustedPoint.y () >= startPoint ().y ())
            normalizedRect.setHeight (minimumHeight);
        else
            normalizedRect.setY (normalizedRect.bottom () - minimumHeight + 1);
    }

#if DEBUG_KP_TOOL_TEXT && 1
    kDebug () << "\t\tnormalizedRect=" << normalizedRect
                << " kpTextSelection::preferredMinimumSize="
                    << QSize (minimumWidth, minimumHeight)
                << endl;
#endif


    //
    // Construct and Deploy Text Box.
    //

    // Create empty text box.
    QList <QString> textLines;
    textLines.append (QString ());
    kpTextSelection textSel (normalizedRect, textLines, textStyle);

    // Create command containing text box.
    if (!m_currentCreateTextCommand)
    {
        m_currentCreateTextCommand = new kpToolSelectionCreateCommand (
            i18n ("Text: Create Box"),
            textSel,
            environ ()->commandEnvironment ());
    }
    else
        m_currentCreateTextCommand->setFromSelection (textSel);

    // Render.
    viewManager ()->setTextCursorPosition (0, 0);
    document ()->setSelection (textSel);


    //
    // Update Status Bar.
    //

    QPoint actualEndPoint = KP_INVALID_POINT;
    if (startPoint () == normalizedRect.topLeft ())
        actualEndPoint = normalizedRect.bottomRight ();
    else if (startPoint () == normalizedRect.bottomRight ())
        actualEndPoint = normalizedRect.topLeft ();
    else if (startPoint () == normalizedRect.topRight ())
        actualEndPoint = normalizedRect.bottomLeft ();
    else if (startPoint () == normalizedRect.bottomLeft ())
        actualEndPoint = normalizedRect.topRight ();

    setUserShapePoints (startPoint (), actualEndPoint);

    return newDragHasBegun;
}

// protected virtual [base kpToolSelection]
void kpToolText::setSelectionBorderForHaventBegunDraw ()
{
    kpToolSelection::setSelectionBorderForHaventBegunDraw ();
    viewManager ()->setTextCursorEnabled (true);
}


// public virtual [base kpToolSelection]
void kpToolText::cancelShape ()
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::cancelShape()";
#endif

    if (m_dragType != Unknown)
        kpToolSelection::cancelShape ();
    else if (hasBegunText ())
    {
        setAllCommandPointersToZero ();

        commandHistory ()->undo ();
    }
    else
        kpToolSelection::cancelShape ();
}

// protected virtual [base kpToolSelection]
QString kpToolText::nonSmearMoveCommandName () const
{
    return i18n ("Text: Move Box");
}

// public virtual [base kpTool]
void kpToolText::endShape (const QPoint &thisPoint, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::endShape()";
#endif

    if (m_dragType != Unknown)
        kpToolSelection::endDraw (thisPoint, normalizedRect);
    else if (hasBegunText ())
        setAllCommandPointersToZero ();
    else
        kpToolSelection::endDraw (thisPoint, normalizedRect);
}


//
// User Changing Text Style Elements
//


// protected
bool kpToolText::shouldChangeTextStyle () const
{
    if (environ ()->settingTextStyle ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\trecursion - abort setting text style: "
                   << environ ()->settingTextStyle ()
                   << endl;
    #endif
        return false;
    }

    if (!document ()->textSelection ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kDebug () << "\tno text selection - abort setting text style";
    #endif
        return false;
    }

    return true;
}

// protected
void kpToolText::changeTextStyle (const QString &name,
                                  const kpTextStyle &newTextStyle,
                                  const kpTextStyle &oldTextStyle)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::changeTextStyle(" << name << ")";
#endif

    if (hasBegunShape ())
        endShape (currentPoint (), kpBug::QRect_Normalized (QRect (startPoint (), currentPoint ())));

    commandHistory ()->addCommand (
        new kpToolTextChangeStyleCommand (
            name,
            newTextStyle,
            oldTextStyle,
            environ ()->commandEnvironment ()));
}


// protected slot virtual [base kpToolSelection]
void kpToolText::slotIsOpaqueChanged ()
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotIsOpaqueChanged()";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBackgroundOpaque (!m_toolWidgetOpaqueOrTransparent->isOpaque ());

    changeTextStyle (newTextStyle.isBackgroundOpaque () ?
                         i18n ("Text: Opaque Background") :
                         i18n ("Text: Transparent Background"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpTool]
void kpToolText::slotColorsSwapped (const kpColor &newForegroundColor,
                                    const kpColor &newBackgroundColor)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotColorsSwapped()";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setForegroundColor (newBackgroundColor);
    oldTextStyle.setBackgroundColor (newForegroundColor);

    changeTextStyle (i18n ("Text: Swap Colors"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpTool]
void kpToolText::slotForegroundColorChanged (const kpColor & /*color*/)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotForegroundColorChanged()";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setForegroundColor (oldForegroundColor ());

    changeTextStyle (i18n ("Text: Foreground Color"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpToolSelection]
void kpToolText::slotBackgroundColorChanged (const kpColor & /*color*/)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotBackgroundColorChanged()";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBackgroundColor (oldBackgroundColor ());

    changeTextStyle (i18n ("Text: Background Color"),
                     newTextStyle,
                     oldTextStyle);
}

// protected slot virtual [base kpToolSelection]
void kpToolText::slotColorSimilarityChanged (double, int)
{
    // --- don't pass on event to kpToolSelection which would have set the
    //     SelectionTransparency - not relevant to the Text Tool ---
}


// public slot
void kpToolText::slotFontFamilyChanged (const QString &fontFamily,
                                        const QString &oldFontFamily)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotFontFamilyChanged() new="
               << fontFamily
               << " old="
               << oldFontFamily
               << endl;
#else
    (void) fontFamily;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setFontFamily (oldFontFamily);

    changeTextStyle (i18n ("Text: Font"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotFontSizeChanged (int fontSize, int oldFontSize)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotFontSizeChanged() new="
               << fontSize
               << " old="
               << oldFontSize
               << endl;
#else
    (void) fontSize;
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setFontSize (oldFontSize);

    changeTextStyle (i18n ("Text: Font Size"),
                     newTextStyle,
                     oldTextStyle);
}


// public slot
void kpToolText::slotBoldChanged (bool isBold)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotBoldChanged(" << isBold << ")";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBold (!isBold);

    changeTextStyle (i18n ("Text: Bold"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotItalicChanged (bool isItalic)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotItalicChanged(" << isItalic << ")";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setItalic (!isItalic);

    changeTextStyle (i18n ("Text: Italic"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotUnderlineChanged (bool isUnderline)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotUnderlineChanged(" << isUnderline << ")";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setUnderline (!isUnderline);

    changeTextStyle (i18n ("Text: Underline"),
                     newTextStyle,
                     oldTextStyle);
}

// public slot
void kpToolText::slotStrikeThruChanged (bool isStrikeThru)
{
#if DEBUG_KP_TOOL_TEXT
    kDebug () << "kpToolText::slotStrikeThruChanged(" << isStrikeThru << ")";
#endif

    if (!shouldChangeTextStyle ())
        return;

    kpTextStyle newTextStyle = environ ()->textStyle ();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setStrikeThru (!isStrikeThru);

    changeTextStyle (i18n ("Text: Strike Through"),
                     newTextStyle,
                     oldTextStyle);
}


#include <kpToolText.moc>
