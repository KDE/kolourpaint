
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
#include <KMenuBar>

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


// private
kpToolSelectionEnvironment *kpMainWindow::toolSelectionEnvironment ()
{
    if (!d->toolSelectionEnvironment)
        d->toolSelectionEnvironment = new kpToolSelectionEnvironment (this);

    return d->toolSelectionEnvironment;
}

// private
kpToolEnvironment *kpMainWindow::toolEnvironment ()
{
    // It's fine to return a more complex environment than required.
    return toolSelectionEnvironment ();
}


// private
void kpMainWindow::setupToolActions ()
{
    kpToolSelectionEnvironment *toolSelEnv = toolSelectionEnvironment ();
    kpToolEnvironment *toolEnv = toolEnvironment ();

    m_tools.append (m_toolFreeFormSelection = new kpToolFreeFormSelection (toolSelEnv, this));
    m_tools.append (m_toolRectSelection = new kpToolRectSelection (toolSelEnv, this));

    m_tools.append (m_toolEllipticalSelection = new kpToolEllipticalSelection (toolSelEnv, this));
    m_tools.append (m_toolText = new kpToolText (toolSelEnv, this));

    m_tools.append (m_toolLine = new kpToolLine (toolEnv, this));
    m_tools.append (m_toolPen = new kpToolPen (toolEnv, this));

    m_tools.append (m_toolEraser = new kpToolEraser (toolEnv, this));
    m_tools.append (m_toolBrush = new kpToolBrush (toolEnv, this));

    m_tools.append (m_toolFloodFill = new kpToolFloodFill (toolEnv, this));
    m_tools.append (m_toolColorPicker = new kpToolColorPicker (toolEnv, this));

    m_tools.append (m_toolColorEraser = new kpToolColorEraser (toolEnv, this));
    m_tools.append (m_toolSpraycan = new kpToolSpraycan (toolEnv, this));

    m_tools.append (m_toolRoundedRectangle = new kpToolRoundedRectangle (toolEnv, this));
    m_tools.append (m_toolRectangle = new kpToolRectangle (toolEnv, this));

    m_tools.append (m_toolPolygon = new kpToolPolygon (toolEnv, this));
    m_tools.append (m_toolEllipse = new kpToolEllipse (toolEnv, this));

    m_tools.append (m_toolPolyline = new kpToolPolyline (toolEnv, this));
    m_tools.append (m_toolCurve = new kpToolCurve (toolEnv, this));

    m_tools.append (m_toolZoom = new kpToolZoom (toolEnv, this));


    KActionCollection *ac = actionCollection ();

    m_actionPrevToolOptionGroup1 = ac->addAction ("prev_tool_option_group_1");
    m_actionPrevToolOptionGroup1->setText (i18n ("Previous Tool Option (Group #1)"));
    m_actionPrevToolOptionGroup1->setShortcuts (kpTool::shortcutForKey (Qt::Key_1));
    connect (m_actionPrevToolOptionGroup1, SIGNAL (triggered (bool)),
        SLOT (slotActionPrevToolOptionGroup1 ()));

    m_actionNextToolOptionGroup1 = ac->addAction ("next_tool_option_group_1");
    m_actionNextToolOptionGroup1->setText (i18n ("Next Tool Option (Group #1)"));
    m_actionNextToolOptionGroup1->setShortcuts (kpTool::shortcutForKey (Qt::Key_2));
    connect (m_actionNextToolOptionGroup1, SIGNAL (triggered (bool)),
        SLOT (slotActionNextToolOptionGroup1 ()));

    m_actionPrevToolOptionGroup2 = ac->addAction ("prev_tool_option_group_2");
    m_actionPrevToolOptionGroup2->setText (i18n ("Previous Tool Option (Group #2)"));
    m_actionPrevToolOptionGroup2->setShortcuts (kpTool::shortcutForKey (Qt::Key_3));
    connect (m_actionPrevToolOptionGroup2, SIGNAL (triggered (bool)),
        SLOT (slotActionPrevToolOptionGroup2 ()));

    m_actionNextToolOptionGroup2 = ac->addAction ("next_tool_option_group_2");
    m_actionNextToolOptionGroup2->setText (i18n ("Next Tool Option (Group #2)"));
    m_actionNextToolOptionGroup2->setShortcuts (kpTool::shortcutForKey (Qt::Key_4));
    connect (m_actionNextToolOptionGroup2, SIGNAL (triggered (bool)),
        SLOT (slotActionNextToolOptionGroup2 ()));


    //
    // Implemented in this file (kpMainWindow_Tools.cpp), not
    // kpImageWindow_Image.cpp since they're really setting tool options.
    //

    d->actionDrawOpaque = ac->add<KToggleAction> ("image_draw_opaque");
    d->actionDrawOpaque->setText (i18n ("&Draw Opaque"));
    connect (d->actionDrawOpaque, SIGNAL (triggered (bool)),
        SLOT (slotActionDrawOpaqueToggled ()));

    d->actionDrawColorSimilarity = ac->addAction ("image_draw_color_similarity");
    d->actionDrawColorSimilarity->setText (i18n ("Draw With Color Similarity..."));
    connect (d->actionDrawColorSimilarity, SIGNAL (triggered (bool)),
        SLOT (slotActionDrawColorSimilarity ()));
}

// private
void kpMainWindow::createToolBox ()
{
    // HACK: Until we have a proper kpToolToolBar where tool actions are
    //       plugged into it properly, Qt4 will not recognise our actions'
    //       shortcuts.
    //
    //       In kolourpaintui.rc, we get around this by plugging them into
    //       a fake menu.  Here, we hide that fake menu.
    Q_ASSERT (menuBar ());
    foreach (QAction *action, menuBar ()->actions ())
    {
        if (action->text () == "DO NOT TRANSLATE, JUST LEAVE AS IS: toolToolBarHiddenMenu")
        {
            action->setVisible (false);
            break;
        }
    }


    m_toolToolBar = new kpToolToolBar (i18n ("Tool Box"), 2/*columns/rows*/, this);
    m_toolToolBar->setObjectName ("Tool Box");  // (needed for QMainWindow::saveState())
    // HACK: QToolBar::setOrientation() is no longer virtual in Qt4.
    //       Therefore, our override of setOrientation() will never be
    //       called in response to user movement.  We might be stuck in
    //       a horizontal position at the start.  So force it vertical
    //       - this is probably what the user wants.
    m_toolToolBar->setOrientation (Qt::Vertical);
    connect (m_toolToolBar, SIGNAL (sigToolSelected (kpTool *)),
             this, SLOT (slotToolSelected (kpTool *)));
    connect (m_toolToolBar, SIGNAL (toolWidgetOptionSelected ()),
             this, SLOT (updateToolOptionPrevNextActionsEnabled ()));

    connect (m_toolToolBar->toolWidgetOpaqueOrTransparent (),
             SIGNAL (isOpaqueChanged (bool)),
             SLOT (updateActionDrawOpaqueChecked ()));
    updateActionDrawOpaqueChecked ();

    for (QList <kpTool *>::const_iterator it = m_tools.begin ();
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
    kDebug () << "kpMainWindow::enableToolsDocumentsAction(" << enable << ")" << endl;
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


    for (QList <kpTool *>::const_iterator it = m_tools.begin ();
         it != m_tools.end ();
         it++)
    {
        kpToolAction *action = (*it)->action ();
        if (action)
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tchanging enabled state of " << (*it)->objectName () << endl;
        #endif

            if (!enable && action->isChecked ())
                action->setChecked (false);

            action->setEnabled (enable);
        }
        else
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tno action for " << (*it)->objectName () << endl;
        #endif
        }
    }


    updateToolOptionPrevNextActionsEnabled ();
    updateActionDrawOpaqueEnabled ();
}

// private slot
void kpMainWindow::updateToolOptionPrevNextActionsEnabled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::updateToolOptionPrevNextActionsEnabled()"
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


// private slot
void kpMainWindow::updateActionDrawOpaqueChecked ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::updateActionDrawOpaqueChecked()" << endl;
#endif

    const bool drawOpaque =
        (m_toolToolBar->toolWidgetOpaqueOrTransparent ()->selectedRow () == 0);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdrawOpaque=" << drawOpaque << endl;
#endif

    d->actionDrawOpaque->setChecked (drawOpaque);
}

// private
void kpMainWindow::updateActionDrawOpaqueEnabled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::updateActionDrawOpaqueEnabled()" << endl;
#endif

    const bool enable = m_toolActionsEnabled;

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tenable=" << enable
              << " tool=" << (tool () ? tool ()->objectName () : 0)
              << " (is selection=" << toolIsASelectionTool () << ")"
              << endl;
#endif

    d->actionDrawOpaque->setEnabled (enable && toolIsASelectionTool ());
}


// public
QActionGroup *kpMainWindow::toolsActionGroup ()
{
    if (!d->toolsActionGroup)
        d->toolsActionGroup = new QActionGroup (this);

    return d->toolsActionGroup;
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
kpImageSelectionTransparency kpMainWindow::imageSelectionTransparency () const
{
    kpToolWidgetOpaqueOrTransparent *oot = m_toolToolBar->toolWidgetOpaqueOrTransparent ();
    Q_ASSERT (oot);

    return kpImageSelectionTransparency (oot->isOpaque (), backgroundColor (), m_colorToolBar->colorSimilarity ());
}

// public
void kpMainWindow::setImageSelectionTransparency (const kpImageSelectionTransparency &transparency, bool forceColorChange)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::setImageSelectionTransparency() isOpaque=" << transparency.isOpaque ()
               << " color=" << (transparency.transparentColor ().isValid () ? (int *) transparency.transparentColor ().toQRgb () : 0)
               << " forceColorChange=" << forceColorChange
               << endl;
#endif

    kpToolWidgetOpaqueOrTransparent *oot = m_toolToolBar->toolWidgetOpaqueOrTransparent ();
    Q_ASSERT (oot);

    m_settingImageSelectionTransparency++;

    oot->setOpaque (transparency.isOpaque ());
    if (transparency.isTransparent () || forceColorChange)
    {
        m_colorToolBar->setColor (1, transparency.transparentColor ());
        m_colorToolBar->setColorSimilarity (transparency.colorSimilarity ());
    }

    m_settingImageSelectionTransparency--;
}

// public
int kpMainWindow::settingImageSelectionTransparency () const
{
    return m_settingImageSelectionTransparency;
}


// private slot
void kpMainWindow::slotToolSelected (kpTool *tool)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotToolSelected (" << tool << ")" << endl;
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
    updateActionDrawOpaqueEnabled ();
}


// private
void kpMainWindow::readLastTool ()
{
    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupTools);

    m_lastToolNumber = cfg.readEntry (kpSettingLastTool, -1);
}


// private
int kpMainWindow::toolNumber () const
{
    int number = 0;
    for (QList <kpTool *>::const_iterator it = m_tools.begin ();
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


    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupTools);

    cfg.writeEntry (kpSettingLastTool, number);
    cfg.sync ();
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
    kDebug () << "kpMainWindow::slotDragScroll() maybeDragScrolling="
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
                    " image?</p></qt>",
                  m_docResizeWidth,
                  m_docResizeHeight),
            i18n ("Resize Image?"),
            i18n ("R&esize Image"),
            this))
    {
        m_commandHistory->addCommand (
            new kpTransformResizeScaleCommand (
                false/*doc, not sel*/,
                m_docResizeWidth, m_docResizeHeight,
                kpTransformResizeScaleCommand::Resize,
                commandEnvironment ()));

        saveDefaultDocSize (QSize (m_docResizeWidth, m_docResizeHeight));
    }


    DOC_RESIZE_COMPLETED ();

#undef DOC_RESIZE_COMPLETED
}

// private slot
void kpMainWindow::slotDocResizeMessageChanged (const QString &string)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotDocResizeMessageChanged(" << string
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


// private slot
void kpMainWindow::slotActionDrawOpaqueToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotActionDrawOpaqueToggled()" << endl;
#endif
    // ("kpToolWidgetBase::" is to access one overload shadowed by the override
    //  of the other overload)
    m_toolToolBar->toolWidgetOpaqueOrTransparent ()->kpToolWidgetBase::setSelected (
        (d->actionDrawOpaque->isChecked () ?
            0/*row 0 = opaque*/ :
            1/*row 1 = transparent*/),
        0/*column*/);
}

// private slot
void kpMainWindow::slotActionDrawColorSimilarity ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotActionDrawColorSimilarity()" << endl;
#endif
    m_colorToolBar->openColorSimilarityDialog ();
}


// public slots

// LOTODO: Who actually calls these?
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

SLOT_TOOL (Spraycan)
SLOT_TOOL (Brush)
SLOT_TOOL (ColorEraser)
SLOT_TOOL (ColorPicker)
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
SLOT_TOOL (Zoom)
