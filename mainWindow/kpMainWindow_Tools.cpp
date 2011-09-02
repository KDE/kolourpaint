/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2011 Martin Koller <kollix@aon.at>
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


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <QActionGroup>
#include <qlist.h>

#include <kapplication.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <kpColorToolBar.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpImageSelectionTransparency.h>
#include <kpTool.h>
#include <kpToolAction.h>
#include <kpToolBrush.h>
#include <kpToolColorEraser.h>
#include <kpToolColorPicker.h>
#include <kpToolCurve.h>
#include <kpToolEllipticalSelection.h>
#include <kpToolEllipse.h>
#include <kpToolEraser.h>
#include <kpToolFloodFill.h>
#include <kpToolFreeFormSelection.h>
#include <kpToolLine.h>
#include <kpToolPen.h>
#include <kpToolPolygon.h>
#include <kpToolPolyline.h>
#include <kpToolRectangle.h>
#include <kpToolRectSelection.h>
#include <kpToolRoundedRectangle.h>
#include <kpToolSelectionEnvironment.h>
#include <kpToolSpraycan.h>
#include <kpToolText.h>
#include <kpToolToolBar.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpToolZoom.h>
#include <kpTransformResizeScaleCommand.h>
#include <kpViewScrollableContainer.h>
#include <kpZoomedView.h>

//---------------------------------------------------------------------

// private
kpToolSelectionEnvironment *kpMainWindow::toolSelectionEnvironment ()
{
    if (!d->toolSelectionEnvironment)
        d->toolSelectionEnvironment = new kpToolSelectionEnvironment (this);

    return d->toolSelectionEnvironment;
}

//---------------------------------------------------------------------

// private
kpToolEnvironment *kpMainWindow::toolEnvironment ()
{
    // It's fine to return a more complex environment than required.
    return toolSelectionEnvironment ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setupToolActions ()
{
    kpToolSelectionEnvironment *toolSelEnv = toolSelectionEnvironment ();
    kpToolEnvironment *toolEnv = toolEnvironment ();

    d->tools.append (d->toolFreeFormSelection = new kpToolFreeFormSelection (toolSelEnv, this));
    d->tools.append (d->toolRectSelection = new kpToolRectSelection (toolSelEnv, this));

    d->tools.append (d->toolEllipticalSelection = new kpToolEllipticalSelection (toolSelEnv, this));
    d->tools.append (d->toolText = new kpToolText (toolSelEnv, this));

    d->tools.append (d->toolLine = new kpToolLine (toolEnv, this));
    d->tools.append (d->toolPen = new kpToolPen (toolEnv, this));

    d->tools.append (d->toolEraser = new kpToolEraser (toolEnv, this));
    d->tools.append (d->toolBrush = new kpToolBrush (toolEnv, this));

    d->tools.append (d->toolFloodFill = new kpToolFloodFill (toolEnv, this));
    d->tools.append (d->toolColorPicker = new kpToolColorPicker (toolEnv, this));

    d->tools.append (d->toolColorEraser = new kpToolColorEraser (toolEnv, this));
    d->tools.append (d->toolSpraycan = new kpToolSpraycan (toolEnv, this));

    d->tools.append (d->toolRoundedRectangle = new kpToolRoundedRectangle (toolEnv, this));
    d->tools.append (d->toolRectangle = new kpToolRectangle (toolEnv, this));

    d->tools.append (d->toolPolygon = new kpToolPolygon (toolEnv, this));
    d->tools.append (d->toolEllipse = new kpToolEllipse (toolEnv, this));

    d->tools.append (d->toolPolyline = new kpToolPolyline (toolEnv, this));
    d->tools.append (d->toolCurve = new kpToolCurve (toolEnv, this));

    d->tools.append (d->toolZoom = new kpToolZoom (toolEnv, this));


    KActionCollection *ac = actionCollection ();

    d->actionPrevToolOptionGroup1 = ac->addAction ("prev_tool_option_group_1");
    d->actionPrevToolOptionGroup1->setText (i18n ("Previous Tool Option (Group #1)"));
    d->actionPrevToolOptionGroup1->setShortcuts (kpTool::shortcutForKey (Qt::Key_1));
    connect (d->actionPrevToolOptionGroup1, SIGNAL (triggered (bool)),
        SLOT (slotActionPrevToolOptionGroup1 ()));

    d->actionNextToolOptionGroup1 = ac->addAction ("next_tool_option_group_1");
    d->actionNextToolOptionGroup1->setText (i18n ("Next Tool Option (Group #1)"));
    d->actionNextToolOptionGroup1->setShortcuts (kpTool::shortcutForKey (Qt::Key_2));
    connect (d->actionNextToolOptionGroup1, SIGNAL (triggered (bool)),
        SLOT (slotActionNextToolOptionGroup1 ()));

    d->actionPrevToolOptionGroup2 = ac->addAction ("prev_tool_option_group_2");
    d->actionPrevToolOptionGroup2->setText (i18n ("Previous Tool Option (Group #2)"));
    d->actionPrevToolOptionGroup2->setShortcuts (kpTool::shortcutForKey (Qt::Key_3));
    connect (d->actionPrevToolOptionGroup2, SIGNAL (triggered (bool)),
        SLOT (slotActionPrevToolOptionGroup2 ()));

    d->actionNextToolOptionGroup2 = ac->addAction ("next_tool_option_group_2");
    d->actionNextToolOptionGroup2->setText (i18n ("Next Tool Option (Group #2)"));
    d->actionNextToolOptionGroup2->setShortcuts (kpTool::shortcutForKey (Qt::Key_4));
    connect (d->actionNextToolOptionGroup2, SIGNAL (triggered (bool)),
        SLOT (slotActionNextToolOptionGroup2 ()));


    //
    // Implemented in this file (kpMainWindow_Tools.cpp), not
    // kpImageWindow_Image.cpp since they're really setting tool options.
    //

    d->actionDrawOpaque = ac->add <KToggleAction> ("image_draw_opaque");
    d->actionDrawOpaque->setText (i18n ("&Draw Opaque"));
    connect (d->actionDrawOpaque, SIGNAL (triggered (bool)),
        SLOT (slotActionDrawOpaqueToggled ()));

    d->actionDrawColorSimilarity = ac->addAction ("image_draw_color_similarity");
    d->actionDrawColorSimilarity->setText (i18n ("Draw With Color Similarity..."));
    connect (d->actionDrawColorSimilarity, SIGNAL (triggered (bool)),
        SLOT (slotActionDrawColorSimilarity ()));
}

//---------------------------------------------------------------------

// private
void kpMainWindow::createToolBox ()
{
    d->toolToolBar = new kpToolToolBar(QLatin1String("Tool Box"), 2/*columns/rows*/, this);
    d->toolToolBar->setWindowTitle(i18n("Tool Box"));

    connect (d->toolToolBar, SIGNAL (sigToolSelected (kpTool *)),
             this, SLOT (slotToolSelected (kpTool *)));
    connect (d->toolToolBar, SIGNAL (toolWidgetOptionSelected ()),
             this, SLOT (updateToolOptionPrevNextActionsEnabled ()));

    connect (d->toolToolBar->toolWidgetOpaqueOrTransparent (),
             SIGNAL (isOpaqueChanged (bool)),
             SLOT (updateActionDrawOpaqueChecked ()));
    updateActionDrawOpaqueChecked ();

    foreach (kpTool *tool, d->tools)
      d->toolToolBar->registerTool(tool);

    // (from config file)
    readLastTool ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableToolsDocumentActions (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::enableToolsDocumentsAction(" << enable << ")";
#endif

    d->toolActionsEnabled = enable;

    if (enable && !d->toolToolBar->isEnabled ())
    {
        kpTool *previousTool = d->toolToolBar->previousTool ();

        // select tool for enabled Tool Box

        if (previousTool)
            d->toolToolBar->selectPreviousTool ();
        else
        {
            if (d->lastToolNumber >= 0 && d->lastToolNumber < (int) d->tools.count ())
                d->toolToolBar->selectTool (d->tools.at (d->lastToolNumber));
            else
                d->toolToolBar->selectTool (d->toolPen);
        }
    }
    else if (!enable && d->toolToolBar->isEnabled ())
    {
        // don't have a disabled Tool Box with a checked Tool
        d->toolToolBar->selectTool (0);
    }


    d->toolToolBar->setEnabled (enable);


    foreach (kpTool *tool, d->tools)
    {
      kpToolAction *action = tool->action();
      if (!enable && action->isChecked())
          action->setChecked(false);

      action->setEnabled(enable);
    }


    updateToolOptionPrevNextActionsEnabled ();
    updateActionDrawOpaqueEnabled ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::updateToolOptionPrevNextActionsEnabled ()
{
    const bool enable = d->toolActionsEnabled;


    d->actionPrevToolOptionGroup1->setEnabled (enable &&
        d->toolToolBar->shownToolWidget (0) &&
        d->toolToolBar->shownToolWidget (0)->hasPreviousOption ());
    d->actionNextToolOptionGroup1->setEnabled (enable &&
        d->toolToolBar->shownToolWidget (0) &&
        d->toolToolBar->shownToolWidget (0)->hasNextOption ());

    d->actionPrevToolOptionGroup2->setEnabled (enable &&
        d->toolToolBar->shownToolWidget (1) &&
        d->toolToolBar->shownToolWidget (1)->hasPreviousOption ());
    d->actionNextToolOptionGroup2->setEnabled (enable &&
        d->toolToolBar->shownToolWidget (1) &&
        d->toolToolBar->shownToolWidget (1)->hasNextOption ());
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::updateActionDrawOpaqueChecked ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::updateActionDrawOpaqueChecked()";
#endif

    const bool drawOpaque =
        (d->toolToolBar->toolWidgetOpaqueOrTransparent ()->selectedRow () == 0);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdrawOpaque=" << drawOpaque;
#endif

    d->actionDrawOpaque->setChecked (drawOpaque);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::updateActionDrawOpaqueEnabled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::updateActionDrawOpaqueEnabled()";
#endif

    const bool enable = d->toolActionsEnabled;

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tenable=" << enable
              << " tool=" << (tool () ? tool ()->objectName () : 0)
              << " (is selection=" << toolIsASelectionTool () << ")"
              << endl;
#endif

    d->actionDrawOpaque->setEnabled (enable && toolIsASelectionTool ());
}

//---------------------------------------------------------------------

// public
QActionGroup *kpMainWindow::toolsActionGroup ()
{
    if (!d->toolsActionGroup)
        d->toolsActionGroup = new QActionGroup (this);

    return d->toolsActionGroup;
}

//---------------------------------------------------------------------

// public
kpTool *kpMainWindow::tool () const
{
    return d->toolToolBar ? d->toolToolBar->tool () : 0;
}

//---------------------------------------------------------------------

// public
bool kpMainWindow::toolHasBegunShape () const
{
    kpTool *currentTool = tool ();
    return (currentTool && currentTool->hasBegunShape ());
}

//---------------------------------------------------------------------

// public
bool kpMainWindow::toolIsASelectionTool (bool includingTextTool) const
{
    kpTool *currentTool = tool ();

    return ((currentTool == d->toolFreeFormSelection) ||
            (currentTool == d->toolRectSelection) ||
            (currentTool == d->toolEllipticalSelection) ||
            (currentTool == d->toolText && includingTextTool));
}

//---------------------------------------------------------------------

// public
bool kpMainWindow::toolIsTextTool () const
{
    return (tool () == d->toolText);
}

//---------------------------------------------------------------------


// private
void kpMainWindow::toolEndShape ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();
}

//---------------------------------------------------------------------

// public
kpImageSelectionTransparency kpMainWindow::imageSelectionTransparency () const
{
    kpToolWidgetOpaqueOrTransparent *oot = d->toolToolBar->toolWidgetOpaqueOrTransparent ();
    Q_ASSERT (oot);

    return kpImageSelectionTransparency (oot->isOpaque (), backgroundColor (), d->colorToolBar->colorSimilarity ());
}

//---------------------------------------------------------------------

// public
void kpMainWindow::setImageSelectionTransparency (const kpImageSelectionTransparency &transparency, bool forceColorChange)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::setImageSelectionTransparency() isOpaque=" << transparency.isOpaque ()
               << " color=" << (transparency.transparentColor ().isValid () ? (int *) transparency.transparentColor ().toQRgb () : 0)
               << " forceColorChange=" << forceColorChange
               << endl;
#endif

    kpToolWidgetOpaqueOrTransparent *oot = d->toolToolBar->toolWidgetOpaqueOrTransparent ();
    Q_ASSERT (oot);

    d->settingImageSelectionTransparency++;

    oot->setOpaque (transparency.isOpaque ());
    if (transparency.isTransparent () || forceColorChange)
    {
        d->colorToolBar->setColor (1, transparency.transparentColor ());
        d->colorToolBar->setColorSimilarity (transparency.colorSimilarity ());
    }

    d->settingImageSelectionTransparency--;
}

//---------------------------------------------------------------------

// public
int kpMainWindow::settingImageSelectionTransparency () const
{
    return d->settingImageSelectionTransparency;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotToolSelected (kpTool *tool)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotToolSelected (" << tool << ")";
#endif

    kpTool *previousTool = d->toolToolBar ? d->toolToolBar->previousTool () : 0;

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

        disconnect (d->colorToolBar, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)),
                    previousTool, SLOT (slotColorsSwappedInternal (const kpColor &, const kpColor &)));
        disconnect (d->colorToolBar, SIGNAL (foregroundColorChanged (const kpColor &)),
                    previousTool, SLOT (slotForegroundColorChangedInternal (const kpColor &)));
        disconnect (d->colorToolBar, SIGNAL (backgroundColorChanged (const kpColor &)),
                    previousTool, SLOT (slotBackgroundColorChangedInternal (const kpColor &)));
        disconnect (d->colorToolBar, SIGNAL (colorSimilarityChanged (double, int)),
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

        connect (d->colorToolBar, SIGNAL (colorsSwapped (const kpColor &, const kpColor &)),
                 tool, SLOT (slotColorsSwappedInternal (const kpColor &, const kpColor &)));
        connect (d->colorToolBar, SIGNAL (foregroundColorChanged (const kpColor &)),
                 tool, SLOT (slotForegroundColorChangedInternal (const kpColor &)));
        connect (d->colorToolBar, SIGNAL (backgroundColorChanged (const kpColor &)),
                 tool, SLOT (slotBackgroundColorChangedInternal (const kpColor &)));
        connect (d->colorToolBar, SIGNAL (colorSimilarityChanged (double, int)),
                 tool, SLOT (slotColorSimilarityChangedInternal (double, int)));


        saveLastTool ();
    }

    updateToolOptionPrevNextActionsEnabled ();
    updateActionDrawOpaqueEnabled ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::readLastTool ()
{
    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupTools);

    d->lastToolNumber = cfg.readEntry (kpSettingLastTool, -1);
}

//---------------------------------------------------------------------

// private
int kpMainWindow::toolNumber () const
{
    int number = 0;
    for (QList <kpTool *>::const_iterator it = d->tools.constBegin ();
         it != d->tools.constEnd ();
         ++it)
    {
        if (*it == tool ())
            return number;

        number++;
    }

    return -1;
}

//---------------------------------------------------------------------

// private
void kpMainWindow::saveLastTool ()
{
    int number = toolNumber ();
    if (number < 0 || number >= (int) d->tools.count ())
        return;


    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupTools);

    cfg.writeEntry (kpSettingLastTool, number);
    cfg.sync ();
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::maybeDragScrollingMainView () const
{
    return (tool () && d->mainView &&
            tool ()->viewUnderStartPoint () == d->mainView);
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotDragScroll (const QPoint &docPoint,
                                   const QPoint &docLastPoint,
                                   int zoomLevel,
                                   bool *scrolled)
{
  Q_UNUSED(docPoint)
  Q_UNUSED(docLastPoint)

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotDragScroll() maybeDragScrolling="
               << maybeDragScrollingMainView ()
               << endl;
#endif

    if (maybeDragScrollingMainView ())
    {
        return d->scrollView->beginDragScroll(zoomLevel, scrolled);
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotEndDragScroll ()
{
    // (harmless if haven't started drag scroll)
    return d->scrollView->endDragScroll ();
}

//---------------------------------------------------------------------


// private slot
void kpMainWindow::slotBeganDocResize ()
{
    toolEndShape ();

    recalculateStatusBarShape ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotContinuedDocResize (const QSize &)
{
    recalculateStatusBarShape ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotCancelledDocResize ()
{
    recalculateStatusBar ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotEndedDocResize (const QSize &size)
{
#define DOC_RESIZE_COMPLETED()           \
{                                        \
    d->docResizeToBeCompleted = false;    \
    recalculateStatusBar ();             \
}

    // Prevent statusbar updates
    d->docResizeToBeCompleted = true;

    d->docResizeWidth = (size.width () > 0 ? size.width () : 1),
    d->docResizeHeight = (size.height () > 0 ? size.height () : 1);

    if (d->docResizeWidth == d->document->width () &&
        d->docResizeHeight == d->document->height ())
    {
        DOC_RESIZE_COMPLETED ();
        return;
    }


    // Blank status to avoid confusion if dialog comes up
    setStatusBarMessage ();
    setStatusBarShapePoints ();
    setStatusBarShapeSize ();


    if (kpTool::warnIfBigImageSize (d->document->width (),
            d->document->height (),
            d->docResizeWidth, d->docResizeHeight,
            i18n ("<qt><p>Resizing the image to"
                    " %1x%2 may take a substantial amount of memory."
                    " This can reduce system"
                    " responsiveness and cause other application resource"
                    " problems.</p>"

                    "<p>Are you sure want to resize the"
                    " image?</p></qt>",
                  d->docResizeWidth,
                  d->docResizeHeight),
            i18nc ("@title:window", "Resize Image?"),
            i18n ("R&esize Image"),
            this))
    {
        d->commandHistory->addCommand (
            new kpTransformResizeScaleCommand (
                false/*doc, not sel*/,
                d->docResizeWidth, d->docResizeHeight,
                kpTransformResizeScaleCommand::Resize,
                commandEnvironment ()));

        saveDefaultDocSize (QSize (d->docResizeWidth, d->docResizeHeight));
    }


    DOC_RESIZE_COMPLETED ();

#undef DOC_RESIZE_COMPLETED
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotDocResizeMessageChanged (const QString &string)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotDocResizeMessageChanged(" << string
               << ") docResizeToBeCompleted=" << d->docResizeToBeCompleted
               << endl;
#else
    (void) string;
#endif

    if (d->docResizeToBeCompleted)
        return;

    recalculateStatusBarMessage ();
}

//---------------------------------------------------------------------


// private slot
void kpMainWindow::slotActionPrevToolOptionGroup1 ()
{
    if (!d->toolToolBar->shownToolWidget (0))
        return;

    // We don't call toolEndShape() here because we want #23 in the file BUGS
    // to later work.

    d->toolToolBar->shownToolWidget (0)->selectPreviousOption ();
    updateToolOptionPrevNextActionsEnabled ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotActionNextToolOptionGroup1 ()
{
    if (!d->toolToolBar->shownToolWidget (0))
        return;

    // We don't call toolEndShape() here because we want #23 in the file BUGS
    // to later work.

    d->toolToolBar->shownToolWidget (0)->selectNextOption ();
    updateToolOptionPrevNextActionsEnabled ();
}

//---------------------------------------------------------------------


// private slot
void kpMainWindow::slotActionPrevToolOptionGroup2 ()
{
    if (!d->toolToolBar->shownToolWidget (1))
        return;

    // We don't call toolEndShape() here because we want #23 in the file BUGS
    // to later work.

    d->toolToolBar->shownToolWidget (1)->selectPreviousOption ();
    updateToolOptionPrevNextActionsEnabled ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotActionNextToolOptionGroup2 ()
{
    if (!d->toolToolBar->shownToolWidget (1))
        return;

    // We don't call toolEndShape() here because we want #23 in the file BUGS
    // to later work.

    d->toolToolBar->shownToolWidget (1)->selectNextOption ();
    updateToolOptionPrevNextActionsEnabled ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotActionDrawOpaqueToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotActionDrawOpaqueToggled()";
#endif
    toolEndShape ();

    // TODO: How does this differ to setImageSelectionTransparency()?

    // ("kpToolWidgetBase::" is to access one overload shadowed by the override
    //  of the other overload)
    d->toolToolBar->toolWidgetOpaqueOrTransparent ()->kpToolWidgetBase::setSelected (
        (d->actionDrawOpaque->isChecked () ?
            0/*row 0 = opaque*/ :
            1/*row 1 = transparent*/),
        0/*column*/);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotActionDrawColorSimilarity ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotActionDrawColorSimilarity()";
#endif
    toolEndShape ();

    d->colorToolBar->openColorSimilarityDialog ();
}

//---------------------------------------------------------------------


// public slots

#define SLOT_TOOL(toolName)                       \
void kpMainWindow::slotTool##toolName ()          \
{                                                 \
    if (!d->toolToolBar)                           \
        return;                                   \
                                                  \
    if (tool () == d->tool##toolName)              \
        return;                                   \
                                                  \
    d->toolToolBar->selectTool (d->tool##toolName); \
}


SLOT_TOOL (RectSelection)
SLOT_TOOL (EllipticalSelection)
SLOT_TOOL (FreeFormSelection)
SLOT_TOOL (Text)
