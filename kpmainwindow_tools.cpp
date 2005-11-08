
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kpmainwindow.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <kpcolortoolbar.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpselectiontransparency.h>
#include <kpsinglekeytriggersaction.h>
#include <kptool.h>
#include <kptoolaction.h>
#include <kptoolairspray.h>
#include <kptoolbrush.h>
#include <kptoolcolorpicker.h>
#include <kptoolcolorwasher.h>
#include <kptoolcurve.h>
#include <kptoolellipticalselection.h>
#include <kptoolellipse.h>
#include <kptooleraser.h>
#include <kptoolfloodfill.h>
#include <kptoolfreeformselection.h>
#include <kptoolline.h>
#include <kptoolpen.h>
#include <kptoolpolygon.h>
#include <kptoolpolyline.h>
#include <kptoolrectangle.h>
#include <kptoolrectselection.h>
#include <kptoolresizescale.h>
#include <kptoolroundedrectangle.h>
#include <kptooltext.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpviewscrollablecontainer.h>
#include <kpzoomedview.h>


// private
void kpMainWindow::setupToolActions ()
{
    m_tools.setAutoDelete (true);

    m_tools.append (m_toolFreeFormSelection = new kpToolFreeFormSelection (this));
    m_tools.append (m_toolRectSelection = new kpToolRectSelection (this));

    m_tools.append (m_toolEllipticalSelection = new kpToolEllipticalSelection (this));
    m_tools.append (m_toolText = new kpToolText (this));

    m_tools.append (m_toolLine = new kpToolLine (this));
    m_tools.append (m_toolPen = new kpToolPen (this));

    m_tools.append (m_toolEraser = new kpToolEraser (this));
    m_tools.append (m_toolBrush = new kpToolBrush (this));

    m_tools.append (m_toolFloodFill = new kpToolFloodFill (this));
    m_tools.append (m_toolColorPicker = new kpToolColorPicker (this));

    m_tools.append (m_toolColorWasher = new kpToolColorWasher (this));
    m_tools.append (m_toolAirSpray = new kpToolAirSpray (this));

    m_tools.append (m_toolRoundedRectangle = new kpToolRoundedRectangle (this));
    m_tools.append (m_toolRectangle = new kpToolRectangle (this));

    m_tools.append (m_toolPolygon = new kpToolPolygon (this));
    m_tools.append (m_toolEllipse = new kpToolEllipse (this));

    m_tools.append (m_toolPolyline = new kpToolPolyline (this));
    m_tools.append (m_toolCurve = new kpToolCurve (this));


    KActionCollection *ac = actionCollection ();

    m_actionPrevToolOptionGroup1 = new kpSingleKeyTriggersAction (
        i18n ("Previous Tool Option (Group #1)"),
        kpTool::shortcutForKey (Qt::Key_1),
        this, SLOT (slotActionPrevToolOptionGroup1 ()),
        ac, "prev_tool_option_group_1");
    m_actionNextToolOptionGroup1 = new kpSingleKeyTriggersAction (
        i18n ("Next Tool Option (Group #1)"),
        kpTool::shortcutForKey (Qt::Key_2),
        this, SLOT (slotActionNextToolOptionGroup1 ()),
        ac, "next_tool_option_group_1");

    m_actionPrevToolOptionGroup2 = new kpSingleKeyTriggersAction (
        i18n ("Previous Tool Option (Group #2)"),
        kpTool::shortcutForKey (Qt::Key_3),
        this, SLOT (slotActionPrevToolOptionGroup2 ()),
        ac, "prev_tool_option_group_2");
    m_actionNextToolOptionGroup2 = new kpSingleKeyTriggersAction (
        i18n ("Next Tool Option (Group #2)"),
        kpTool::shortcutForKey (Qt::Key_4),
        this, SLOT (slotActionNextToolOptionGroup2 ()),
        ac, "next_tool_option_group_2");
}

// private
void kpMainWindow::createToolBox ()
{
    m_toolToolBar = new kpToolToolBar (i18n ("Tool Box"), this, 2/*columns/rows*/, "Tool Box");
    connect (m_toolToolBar, SIGNAL (sigToolSelected (kpTool *)),
             this, SLOT (slotToolSelected (kpTool *)));
    connect (m_toolToolBar, SIGNAL (toolWidgetOptionSelected ()),
             this, SLOT (updateToolOptionPrevNextActionsEnabled ()));

    for (QPtrList <kpTool>::const_iterator it = m_tools.begin ();
         it != m_tools.end ();
         it++)
    {
        m_toolToolBar->registerTool (*it);
    }


    // (from config file)
    readLastTool ();


    enableToolsDocumentActions (false);
}

// private
void kpMainWindow::enableToolsDocumentActions (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::enableToolsDocumentsAction(" << enable << ")" << endl;
#endif

    m_toolActionsEnabled = enable;


    if (enable && !m_toolToolBar->isEnabled ())
    {
        kpTool *previousTool = m_toolToolBar->previousTool ();

        // select tool for enabled Tool Box

        if (previousTool)
            m_toolToolBar->selectPreviousTool ();
        else
        {
            if (m_lastToolNumber >= 0 && m_lastToolNumber < (int) m_tools.count ())
                m_toolToolBar->selectTool (m_tools.at (m_lastToolNumber));
            else
                m_toolToolBar->selectTool (m_toolPen);
        }
    }
    else if (!enable && m_toolToolBar->isEnabled ())
    {
        // don't have a disabled Tool Box with an enabled Tool
        m_toolToolBar->selectTool (0);
    }


    m_toolToolBar->setEnabled (enable);


    for (QPtrList <kpTool>::const_iterator it = m_tools.begin ();
         it != m_tools.end ();
         it++)
    {
        kpToolAction *action = (*it)->action ();
        if (action)
        {
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tchanging enabled state of " << (*it)->name () << endl;
        #endif

            if (!enable && action->isChecked ())
                action->setChecked (false);

            action->setEnabled (enable);
        }
        else
        {
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tno action for " << (*it)->name () << endl;
        #endif
        }
    }


    updateToolOptionPrevNextActionsEnabled ();
}

// private slot
void kpMainWindow::updateToolOptionPrevNextActionsEnabled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::updateToolOptionPrevNextActionsEnabled()"
               << " numShownToolWidgets="
               << m_toolToolBar->numShownToolWidgets ()
               << endl;
#endif

    const bool enable = m_toolActionsEnabled;


    m_actionPrevToolOptionGroup1->setEnabled (enable &&
        m_toolToolBar->shownToolWidget (0) &&
        m_toolToolBar->shownToolWidget (0)->hasPreviousOption ());
    m_actionNextToolOptionGroup1->setEnabled (enable &&
        m_toolToolBar->shownToolWidget (0) &&
        m_toolToolBar->shownToolWidget (0)->hasNextOption ());

    m_actionPrevToolOptionGroup2->setEnabled (enable &&
        m_toolToolBar->shownToolWidget (1) &&
        m_toolToolBar->shownToolWidget (1)->hasPreviousOption ());
    m_actionNextToolOptionGroup2->setEnabled (enable &&
        m_toolToolBar->shownToolWidget (1) &&
        m_toolToolBar->shownToolWidget (1)->hasNextOption ());
}


// public
kpTool *kpMainWindow::tool () const
{
    return m_toolToolBar ? m_toolToolBar->tool () : 0;
}

// public
bool kpMainWindow::toolHasBegunShape () const
{
    kpTool *currentTool = tool ();
    return (currentTool && currentTool->hasBegunShape ());
}

// public
bool kpMainWindow::toolIsASelectionTool (bool includingTextTool) const
{
    kpTool *currentTool = tool ();

    return ((currentTool == m_toolFreeFormSelection) ||
            (currentTool == m_toolRectSelection) ||
            (currentTool == m_toolEllipticalSelection) ||
            (currentTool == m_toolText && includingTextTool));
}

// public
bool kpMainWindow::toolIsTextTool () const
{
    return (tool () == m_toolText);
}


// public
kpSelectionTransparency kpMainWindow::selectionTransparency () const
{
    kpToolWidgetOpaqueOrTransparent *oot = m_toolToolBar->toolWidgetOpaqueOrTransparent ();
    if (!oot)
    {
        kdError () << "kpMainWindow::selectionTransparency() without opaqueOrTransparent widget" << endl;
        return kpSelectionTransparency ();
    }

    return kpSelectionTransparency (oot->isOpaque (), backgroundColor (), m_colorToolBar->colorSimilarity ());
}

// public
void kpMainWindow::setSelectionTransparency (const kpSelectionTransparency &transparency, bool forceColorChange)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::setSelectionTransparency() isOpaque=" << transparency.isOpaque ()
               << " color=" << (transparency.transparentColor ().isValid () ? (int *) transparency.transparentColor ().toQRgb () : 0)
               << " forceColorChange=" << forceColorChange
               << endl;
#endif

    kpToolWidgetOpaqueOrTransparent *oot = m_toolToolBar->toolWidgetOpaqueOrTransparent ();
    if (!oot)
    {
        kdError () << "kpMainWindow::setSelectionTransparency() without opaqueOrTransparent widget" << endl;
        return;
    }

    m_settingSelectionTransparency++;

    oot->setOpaque (transparency.isOpaque ());
    if (transparency.isTransparent () || forceColorChange)
    {
        m_colorToolBar->setColor (1, transparency.transparentColor ());
        m_colorToolBar->setColorSimilarity (transparency.colorSimilarity ());
    }

    m_settingSelectionTransparency--;
}

// public
int kpMainWindow::settingSelectionTransparency () const
{
    return m_settingSelectionTransparency;
}


// private slot
void kpMainWindow::slotToolSelected (kpTool *tool)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotToolSelected (" << tool << ")" << endl;
#endif

    kpTool *previousTool = m_toolToolBar ? m_toolToolBar->previousTool () : 0;

    if (previousTool)
    {
        disconnect (previousTool, SIGNAL (movedAndAboutToDraw (const QPoint &, const QPoint &, int, bool *)),
                    this, SLOT (slotDragScroll (const QPoint &, const QPoint &, int, bool *)));
        disconnect (previousTool, SIGNAL (endedDraw (const QPoint &)),
                    this, SLOT (slotEndDragScroll ()));
        disconnect (previousTool, SIGNAL (cancelledShape (const QPoint &)),
                    this, SLOT (slotEndDragScroll ()));

        disconnect (previousTool, SIGNAL (userMessageChanged (const QString &)),
                    this, SLOT (recalculateStatusBarMessage ()));
        disconnect (previousTool, SIGNAL (userShapePointsChanged (const QPoint &, const QPoint &)),
                    this, SLOT (recalculateStatusBarShape ()));
        disconnect (previousTool, SIGNAL (userShapeSizeChanged (const QSize &)),
                    this, SLOT (recalculateStatusBarShape ()));

        disconnect (m_colorToolBar, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)),
                    previousTool, SLOT (slotColorsSwappedInternal (const kpColor &, const kpColor &)));
        disconnect (m_colorToolBar, SIGNAL (foregroundColorChanged (const kpColor &)),
                    previousTool, SLOT (slotForegroundColorChangedInternal (const kpColor &)));
        disconnect (m_colorToolBar, SIGNAL (backgroundColorChanged (const kpColor &)),
                    previousTool, SLOT (slotBackgroundColorChangedInternal (const kpColor &)));
        disconnect (m_colorToolBar, SIGNAL (colorSimilarityChanged (double, int)),
                    previousTool, SLOT (slotColorSimilarityChangedInternal (double, int)));
    }

    if (tool)
    {
        connect (tool, SIGNAL (movedAndAboutToDraw (const QPoint &, const QPoint &, int, bool *)),
                 this, SLOT (slotDragScroll (const QPoint &, const QPoint &, int, bool *)));
        connect (tool, SIGNAL (endedDraw (const QPoint &)),
                 this, SLOT (slotEndDragScroll ()));
        connect (tool, SIGNAL (cancelledShape (const QPoint &)),
                 this, SLOT (slotEndDragScroll ()));

        connect (tool, SIGNAL (userMessageChanged (const QString &)),
                 this, SLOT (recalculateStatusBarMessage ()));
        connect (tool, SIGNAL (userShapePointsChanged (const QPoint &, const QPoint &)),
                 this, SLOT (recalculateStatusBarShape ()));
        connect (tool, SIGNAL (userShapeSizeChanged (const QSize &)),
                 this, SLOT (recalculateStatusBarShape ()));
        recalculateStatusBar ();

        connect (m_colorToolBar, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)),
                 tool, SLOT (slotColorsSwappedInternal (const kpColor &, const kpColor &)));
        connect (m_colorToolBar, SIGNAL (foregroundColorChanged (const kpColor &)),
                 tool, SLOT (slotForegroundColorChangedInternal (const kpColor &)));
        connect (m_colorToolBar, SIGNAL (backgroundColorChanged (const kpColor &)),
                 tool, SLOT (slotBackgroundColorChangedInternal (const kpColor &)));
        connect (m_colorToolBar, SIGNAL (colorSimilarityChanged (double, int)),
                 tool, SLOT (slotColorSimilarityChangedInternal (double, int)));


        saveLastTool ();
    }

    updateToolOptionPrevNextActionsEnabled ();
}


// private
void kpMainWindow::readLastTool ()
{
    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupTools);
    KConfigBase *cfg = cfgGroupSaver.config ();

    m_lastToolNumber = cfg->readNumEntry (kpSettingLastTool, -1);
}


// private
int kpMainWindow::toolNumber () const
{
    int number = 0;
    for (QPtrList <kpTool>::const_iterator it = m_tools.begin ();
         it != m_tools.end ();
         it++)
    {
        if (*it == tool ())
            return number;

        number++;
    }

    return -1;
}

// private
void kpMainWindow::saveLastTool ()
{
    int number = toolNumber ();
    if (number < 0 || number >= (int) m_tools.count ())
        return;


    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupTools);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingLastTool, number);
    cfg->sync ();
}


// private
bool kpMainWindow::maybeDragScrollingMainView () const
{
    return (tool () && m_mainView &&
            tool ()->viewUnderStartPoint () == m_mainView);
}

// private slot
bool kpMainWindow::slotDragScroll (const QPoint &docPoint,
                                   const QPoint &docLastPoint,
                                   int zoomLevel,
                                   bool *scrolled)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotDragScroll() maybeDragScrolling="
               << maybeDragScrollingMainView ()
               << endl;
#endif

    if (maybeDragScrollingMainView ())
    {
        return m_scrollView->beginDragScroll (docPoint, docLastPoint, zoomLevel, scrolled);
    }
    else
    {
        return false;
    }
}

// private slot
bool kpMainWindow::slotEndDragScroll ()
{
    // (harmless if haven't started drag scroll)
    return m_scrollView->endDragScroll ();
}


// private slot
void kpMainWindow::slotBeganDocResize ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    recalculateStatusBarShape ();
}

// private slot
void kpMainWindow::slotContinuedDocResize (const QSize &)
{
    recalculateStatusBarShape ();
}

// private slot
void kpMainWindow::slotCancelledDocResize ()
{
    recalculateStatusBar ();
}

// private slot
void kpMainWindow::slotEndedDocResize (const QSize &size)
{
#define DOC_RESIZE_COMPLETED()           \
{                                        \
    m_docResizeToBeCompleted = false;    \
    recalculateStatusBar ();             \
}

    // Prevent statusbar updates
    m_docResizeToBeCompleted = true;

    m_docResizeWidth = (size.width () > 0 ? size.width () : 1),
    m_docResizeHeight = (size.height () > 0 ? size.height () : 1);

    if (m_docResizeWidth == m_document->width () &&
        m_docResizeHeight == m_document->height ())
    {
        DOC_RESIZE_COMPLETED ();
        return;
    }


    // Blank status to avoid confusion if dialog comes up
    setStatusBarMessage ();
    setStatusBarShapePoints ();
    setStatusBarShapeSize ();


    if (kpTool::warnIfBigImageSize (m_document->width (),
            m_document->height (),
            m_docResizeWidth, m_docResizeHeight,
            i18n ("<qt><p>Resizing the image to"
                    " %1x%2 may take a substantial amount of memory."
                    " This can reduce system"
                    " responsiveness and cause other application resource"
                    " problems.</p>"

                    "<p>Are you sure want to resize the"
                    " image?</p></qt>")
                .arg (m_docResizeWidth)
                .arg (m_docResizeHeight),
            i18n ("Resize Image?"),
            i18n ("R&esize Image"),
            this))
    {
        m_commandHistory->addCommand (
            new kpToolResizeScaleCommand (
                false/*doc, not sel*/,
                m_docResizeWidth, m_docResizeHeight,
                kpToolResizeScaleCommand::Resize,
                this));

        saveDefaultDocSize (QSize (m_docResizeWidth, m_docResizeHeight));
    }


    DOC_RESIZE_COMPLETED ();

#undef DOC_RESIZE_COMPLETED
}

// private slot
void kpMainWindow::slotDocResizeMessageChanged (const QString &string)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotDocResizeMessageChanged(" << string
               << ") docResizeToBeCompleted=" << m_docResizeToBeCompleted
               << endl;
#else
    (void) string;
#endif

    if (m_docResizeToBeCompleted)
        return;

    recalculateStatusBarMessage ();
}


// private slot
void kpMainWindow::slotActionPrevToolOptionGroup1 ()
{
    if (!m_toolToolBar->shownToolWidget (0))
        return;

    m_toolToolBar->shownToolWidget (0)->selectPreviousOption ();
    updateToolOptionPrevNextActionsEnabled ();
}

// private slot
void kpMainWindow::slotActionNextToolOptionGroup1 ()
{
    if (!m_toolToolBar->shownToolWidget (0))
        return;

    m_toolToolBar->shownToolWidget (0)->selectNextOption ();
    updateToolOptionPrevNextActionsEnabled ();
}


// private slot
void kpMainWindow::slotActionPrevToolOptionGroup2 ()
{
    if (!m_toolToolBar->shownToolWidget (1))
        return;

    m_toolToolBar->shownToolWidget (1)->selectPreviousOption ();
    updateToolOptionPrevNextActionsEnabled ();
}

// private slot
void kpMainWindow::slotActionNextToolOptionGroup2 ()
{
    if (!m_toolToolBar->shownToolWidget (1))
        return;

    m_toolToolBar->shownToolWidget (1)->selectNextOption ();
    updateToolOptionPrevNextActionsEnabled ();
}


// public slots

#define SLOT_TOOL(toolName)                       \
void kpMainWindow::slotTool##toolName ()          \
{                                                 \
    if (!m_toolToolBar)                           \
        return;                                   \
                                                  \
    if (tool () == m_tool##toolName)              \
        return;                                   \
                                                  \
    m_toolToolBar->selectTool (m_tool##toolName); \
}

SLOT_TOOL (AirSpray)
SLOT_TOOL (Brush)
SLOT_TOOL (ColorPicker)
SLOT_TOOL (ColorWasher)
SLOT_TOOL (Curve)
SLOT_TOOL (Ellipse)
SLOT_TOOL (EllipticalSelection)
SLOT_TOOL (Eraser)
SLOT_TOOL (FloodFill)
SLOT_TOOL (FreeFormSelection)
SLOT_TOOL (Line)
SLOT_TOOL (Pen)
SLOT_TOOL (Polygon)
SLOT_TOOL (Polyline)
SLOT_TOOL (Rectangle)
SLOT_TOOL (RectSelection)
SLOT_TOOL (RoundedRectangle)
SLOT_TOOL (Text)
