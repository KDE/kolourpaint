
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <qclipboard.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qstringlist.h>

#include <kaction.h>
#include <kactionclasses.h>
#include <kapplication.h>
#include <kcommand.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstatusbar.h>
#include <kstdguiitem.h>
#include <kurldrag.h>

#include <kpdefs.h>
#include <kpcolortoolbar.h>
#include <kpconfigdialog.h>
#include <kpdocument.h>
#include <kptool.h>
#include <kptoolairspray.h>
#include <kptoolbrush.h>
#include <kptoolclear.h>
#include <kptoolcolorpicker.h>
#include <kptoolcolorwasher.h>
#include <kptoolconverttograyscale.h>
#include <kptoolinvertcolors.h>
#include <kptoolpen.h>
#include <kptoolline.h>
#include <kptoolellipse.h>
#include <kptoolellipticalselection.h>
#include <kptooleraser.h>
#include <kptoolflip.h>
#include <kptoolfloodfill.h>
#include <kptoolfreeformselection.h>
#include <kptoolrectangle.h>
#include <kptoolrectselection.h>
#include <kptoolresizescale.h>
#include <kptoolrotate.h>
#include <kptoolroundedrectangle.h>
#include <kptoolskew.h>
#include <kptooltoolbar.h>
#include <kpmainwindow.h>
#include <kpview.h>
#include <kpviewmanager.h>

#define DEBUG_KPMAINWINDOW 1

// TODO: split up, clean up kpMainWindow class

#define KP_STATUSBAR_ITEM_DOC 1111
#define KP_STATUSBAR_ITEM_POS 1234
#define KP_STATUSBAR_ITEM_SIZE 4321

// Disabling all the actions (e.g. Undo/Redo, Open, Save) when a tool is active
// would look a little weird so we just ignore all calls to user-triggerable
// slots instead.
//
// Syntax examples:
//  KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;        // void return
//  KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;  // to return false
//
#define KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE if (m_toolNum >= 0 && m_toolNum < m_numTools && \
                                               m_tools [m_toolNum] && \
                                               m_tools [m_toolNum]->hasBegunDraw ()) return

kpMainWindow::kpMainWindow (const KURL &url)
    : KMainWindow (0, "kpMainWindow"),
      m_mainView (0), m_document (0), m_viewManager (0)
{
    setMinimumSize (80, 60);
    setAcceptDrops (true);

    // read config
    KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    m_configFirstTime = configGroupSaver.config ()->readBoolEntry (kpSettingFirstTime, true);
    m_configShowGrid = configGroupSaver.config ()->readBoolEntry (kpSettingShowGrid, false);
    m_configShowPath = configGroupSaver.config ()->readBoolEntry (kpSettingShowPath, false);
    m_configDefaultOutputMimetype = configGroupSaver.config ()->readEntry (kpSettingDefaultOutputMimetype, "image/png");
    if (m_configFirstTime)
    {
        configGroupSaver.config ()->writeEntry (kpSettingFirstTime, false);
        configGroupSaver.config ()->sync ();
    }
    kdDebug () << "read config: firstTime=" << m_configFirstTime
               << " showGrid=" << m_configShowGrid
               << " showPath=" << m_configShowPath
               << " outputMimeType=" << m_configDefaultOutputMimetype
               << endl;

    setupActions ();

    createGUI ();
#if 0
    {
    // TODO: XML files that don't work are a complete bitch
            KToolBar *bar = toolBar ("toolbar_tools");
        m_actionToolPen->plug (bar);
        m_actionToolLine->plug (bar);
        m_actionToolEllipse->plug (bar);
        m_actionToolColorPicker->plug (bar);
        m_actionToolAirSpray->plug (bar);
        m_actionToolRectangle->plug (bar);
        m_actionToolRoundedRectangle->plug (bar);
        //m_actionToolPolygon->plug (bar);
        m_actionToolFloodFill->plug (bar);
        m_actionToolRotate->plug (bar);
        m_actionToolBrush->plug (bar);
        m_actionToolEraser->plug (bar);
        m_actionToolRectSelection->plug (bar);
        m_actionToolEllipticalSelection->plug (bar);
        m_actionToolFreeFormSelection->plug (bar);
        m_actionToolColorWasher->plug (bar);
        bar->insertLineSeparator ();
    }
#endif

    m_colorToolBar = new kpColorToolBar (this, "Color Palette");

    m_scrollView = new QScrollView (this);
    setCentralWidget (m_scrollView);


    m_viewManager = new kpViewManager (this);
    connect (m_viewManager, SIGNAL (selectionEnabled (bool)), m_actionCopy, SLOT (setEnabled (bool)));
    connect (m_viewManager, SIGNAL (selectionEnabled (bool)), m_actionCut, SLOT (setEnabled (bool)));
    connect (m_viewManager, SIGNAL (selectionEnabled (bool)), m_actionDeselect, SLOT (setEnabled (bool)));

    setupTools ();
    // create initial document
    slotNew (url);

    // TODO: change to a child window
    toolBar ("toolbar_thumbnaila")->setFullSize ();
    kpView *m_thumbnailView = new kpView (toolBar ("toolbar_thumbnaila"), "thumbnailView", this, 200, 200, true /* autoVariableZoom */);
    m_viewManager->registerView (m_thumbnailView);    // TODO: unregister
    toolBar ("toolbar_thumbnaila")->insertWidget (2000, toolBar ("toolbar_thumbnaila")->width (), m_thumbnailView);
    toolBar ("toolbar_thumbnaila")->setFullSize ();

    statusBar ()->insertItem (QString::null, KP_STATUSBAR_ITEM_DOC, 0/*stretch*/);
    statusBar ()->insertItem (QString::null, KP_STATUSBAR_ITEM_POS, 1/*stretch*/, true/*permanent*/);
    statusBar ()->insertItem (QString::null, KP_STATUSBAR_ITEM_SIZE, 1/*stretch*/, true/*permanent*/);

    setAutoSaveSettings ("kpmainwindow", true);
 
    // put our non-XMLGUI toolbars in a sane place, the first time around
    if (m_configFirstTime)
    {
        m_toolToolBar->setBarPos (KToolBar::Left);
        toolBar ("toolbar_thumbnaila")->setBarPos (KToolBar::Right);
        toolBar ("toolbar_thumbnaila")->hide ();  // it doesn't work well yet
        m_colorToolBar->setBarPos (KToolBar::Bottom);
    }
}

void kpMainWindow::setupActions ()
{
    setupFileMenuActions ();
    setupEditMenuActions ();
    setupViewMenuActions ();
    setupImageMenuActions ();
    setupSettingsMenuActions ();
    KActionCollection *ac = actionCollection ();

#if 0
    m_actionToolPen = new KAction (i18n ("Pen"), "tool_pen", CTRL + ALT + Key_P, this, SLOT (slotToolPen ()),
                    ac, "tool_pen");
    m_actionToolLine = new KAction (i18n ("Line"), "tool_line", CTRL + ALT + Key_I, this, SLOT (slotToolLine ()),
                    ac, "tool_line");
    m_actionToolEllipse = new KAction (i18n ("Ellipse"), "tool_ellipse", CTRL + ALT + Key_E, this, SLOT (slotToolEllipse ()),
                    ac, "tool_ellipse");
    m_actionToolColorPicker = new KAction (i18n ("Color Picker"), "tool_color_picker", 0, this, SLOT (slotToolColorPicker ()), ac, "tool_color_picker");
    m_actionToolAirSpray = new KAction (i18n ("Spraycan"), "tool_spraycan", 0, this, SLOT (slotToolAirSpray ()), ac, "tool_air_spray");
    m_actionToolRectangle = new KAction (i18n ("Rectangle"), "tool_rectangle", 0, this, SLOT (slotToolRectangle ()), ac, "tool_rectangle");
    m_actionToolRoundedRectangle = new KAction (i18n ("Rounded Rectangle"), "tool_rounded_rectangle", 0, this, SLOT (slotToolRoundedRectangle ()), ac, "tool_rounded_rectangle");
    //m_actionToolPolygon
    m_actionToolFloodFill = new KAction (i18n ("Flood Fill"), "tool_flood_fill", 0, this, SLOT (slotToolFloodFill ()), ac, "tool_flood_fill");
    m_actionToolRotate = new KAction (i18n ("Rotate"), "tool_rotate", 0, this, SLOT (slotToolRotate ()), ac, "tool_rotate");
    m_actionToolBrush = new KAction (i18n ("Brush"), "tool_brush", 0, this, SLOT (slotToolBrush ()), ac, "tool_brush");
    m_actionToolEraser = new KAction (i18n ("Eraser"), "tool_eraser", 0, this, SLOT (slotToolEraser ()), ac, "tool_eraser");
    m_actionToolRectSelection = new KAction (i18n ("Selection (Rectangle)"), "tool_rect_selection", 0, this, SLOT (slotToolRectSelection ()), ac, "tool_rect_selection");
    m_actionToolEllipticalSelection = new KAction (i18n ("Selection (Elliptical)"), "tool_elliptical_selection", 0, this, SLOT (slotToolEllipticalSelection ()), ac, "tool_elliptical_selection");
    m_actionToolFreeFormSelection = new KAction (i18n ("Selection (Free Form)"), "tool_free_form_selection", 0, this, SLOT (slotToolFreeFormSelection ()), ac, "tool_free_form_sleection");
    m_actionToolColorWasher = new KAction (i18n ("Color Washer"), "tool_color_washer", 0, this, SLOT (slotToolColorWasher ()), ac, "tool_color_washer");
#endif
}

void kpMainWindow::setupFileMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionNew = KStdAction::openNew (this, SLOT (slotNew ()), ac);
    m_actionOpen = KStdAction::open (this, SLOT (slotOpen ()), ac);

    m_actionOpenRecent = KStdAction::openRecent (this, SLOT (slotOpenRecent (const KURL &)), ac);
    m_actionOpenRecent->loadEntries (kapp->config ());

    m_actionSave = KStdAction::save (this, SLOT (slotSave ()), ac);
    m_actionSaveAs = KStdAction::saveAs (this, SLOT (slotSaveAs ()), ac);
    m_actionRevert = KStdAction::revert (this, SLOT (slotRevert ()), ac);

    m_actionPrint = KStdAction::print (this, SLOT (slotPrint ()), ac);
    m_actionPrintPreview = KStdAction::printPreview (this, SLOT (slotPrintPreview ()), ac);

    m_actionSetAsWallpaperCentered = new KAction (i18n ("Set As Wa&llpaper (Centered)"), 0,
        this, SLOT (slotSetAsWallpaperCentered ()), ac, "file_set_as_wallpaper_centered");
    m_actionSetAsWallpaperTiled = new KAction (i18n ("Set As &Wallpaper (Tiled)"), 0,
        this, SLOT (slotSetAsWallpaperTiled ()), ac, "file_set_as_wallpaper_tiled");

    m_actionClose = KStdAction::close (this, SLOT (slotClose ()), ac);
    m_actionQuit = KStdAction::quit (this, SLOT (slotQuit ()), ac);

    m_actionSave->setEnabled (false);
    m_actionSaveAs->setEnabled (false);
    m_actionSaveAs->setEnabled (false);
    m_actionRevert->setEnabled (false);
    m_actionPrint->setEnabled (false);
    m_actionPrintPreview->setEnabled (false);
    m_actionClose->setEnabled (false);
}

void kpMainWindow::setupEditMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    // Undo/Redo
    m_commandHistory = new KCommandHistory (ac, true);
    m_commandHistory->setUndoLimit (5);  // CONFIG

    m_actionCut = KStdAction::cut (this, SLOT (slotCut ()), ac);
    m_actionCopy = KStdAction::copy (this, SLOT (slotCopy ()), ac);
    m_actionPaste = KStdAction::paste (this, SLOT (slotPaste ()), ac);

    m_actionSelectAll = KStdAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    m_actionDeselect = KStdAction::deselect (this, SLOT (slotDeselect ()), ac);

    m_actionCut->setEnabled (false);
    m_actionCopy->setEnabled (false);
}

void kpMainWindow::setupViewMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    /*m_actionFullScreen = KStdAction::fullScreen (0, 0, ac);
    m_actionFullScreen->setEnabled (false);*/

    m_actionZoomIn = KStdAction::zoomIn (this, SLOT (slotZoomIn ()), ac);
    m_actionZoomOut = KStdAction::zoomOut (this, SLOT (slotZoomOut ()), ac);

    m_actionZoom = new KSelectAction (i18n ("&Zoom"), 0,
        this, SLOT (slotZoom ()), actionCollection (), "view_zoom");
    m_zoomList << "100%" << "200%" << "300%" << "400%" << "500%" << "600%" << "700%"
               << "800%" << "900%" << "1000%";
    m_actionZoom->setItems (m_zoomList);
    m_actionZoom->setCurrentItem (0);

    m_actionShowGrid = new KToggleAction (i18n ("Show &Grid"), CTRL + Key_G,
        this, SLOT (slotShowGrid ()), actionCollection (), "view_show_grid");
    m_actionShowGrid->setChecked (m_configShowGrid);
    connect (m_actionShowGrid, SIGNAL (toggled (bool)), this, SLOT (slotActionShowGridToggled (bool)));

    m_actionZoomIn->setEnabled (false);
    m_actionZoomOut->setEnabled (false);
    m_actionZoom->setEnabled (false);
    m_actionShowGrid->setEnabled (false);
}

void kpMainWindow::setupImageMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionProperties = new KAction (i18n ("&Properties..."), 0,
        this, SLOT (slotProperties ()), ac, "image_properties");

    m_actionResizeScale = new KAction (i18n ("Resize / &Scale..."), 0,
        this, SLOT (slotResizeScale ()), ac, "image_resize_scale");

    m_actionFlip = new KAction (i18n ("&Flip..."), 0,
        this, SLOT (slotFlip ()), ac, "image_flip");

    m_actionRotate = new KAction (i18n ("&Rotate..."), 0,
        this, SLOT (slotRotate ()), ac, "image_rotate");

    m_actionSkew = new KAction (i18n ("S&kew..."), 0,
        this, SLOT (slotSkew ()), ac, "image_skew");

    m_actionConvertToGrayscale = new KAction (i18n ("Convert to &Grayscale"), 0,
        this, SLOT (slotConvertToGrayscale ()), ac, "image_convert_to_grayscale");

    m_actionInvertColors = new KAction (i18n ("&Invert Colors"), CTRL + Key_I,
        this, SLOT (slotInvertColors ()), ac, "image_invert_colors");

    m_actionClear = new KAction (i18n ("&Clear"), CTRL + SHIFT + Key_N,
        this, SLOT (slotClear ()), ac, "image_clear");
}

void kpMainWindow::setupSettingsMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    // Settings/Toolbars |> %s
    setStandardToolBarMenuEnabled (true);

    // Settings/Show Statusbar
    createStandardStatusBarAction ();

    m_actionShowPath = new KToggleAction (i18n ("Sho&w Path"), 0,
        this, SLOT (slotShowPath ()), ac, "settings_show_path");
    m_actionShowPath->setChecked (m_configShowPath);
    kdDebug () << "BLAH: m_configShowPath=" << m_configShowPath << " isCheck=" << m_actionShowPath->isChecked () << endl;
    connect (m_actionShowPath, SIGNAL (toggled (bool)),
             this, SLOT (slotActionShowPathToggled (bool)));

    m_actionKeyBindings = KStdAction::keyBindings (this, SLOT (slotKeyBindings ()), ac);
    m_actionConfigureToolbars = KStdAction::configureToolbars (this, SLOT (slotConfigureToolBars ()), ac);
    m_actionConfigure = KStdAction::preferences (this, SLOT (slotConfigure ()), ac);
}

void kpMainWindow::setupTools ()
{
    kpTool *brush;

    // initialise tools
    m_numTools = 0;
    m_tools [m_numTools++] = new kpToolRectSelection (this);
    m_tools [m_numTools++] = new kpToolEllipticalSelection (this);
    
    m_tools [m_numTools++] = new kpToolFreeFormSelection (this);
    m_tools [m_numTools++] = new kpToolColorPicker (this);
    
    m_tools [m_numTools++] = new kpToolPen (this);
    m_tools [m_numTools++] = new kpToolLine (this);

    m_tools [m_numTools++] = brush = new kpToolBrush (this);
    m_tools [m_numTools++] = new kpToolEraser (this);
    
    m_tools [m_numTools++] = new kpToolAirSpray (this);
    m_tools [m_numTools++] = new kpToolColorWasher (this);
    
    m_tools [m_numTools++] = new kpToolRectangle (this);
    m_tools [m_numTools++] = new kpToolRoundedRectangle (this);
    
    m_tools [m_numTools++] = new kpToolEllipse (this);
    m_tools [m_numTools++] = new kpToolFloodFill (this);

    m_toolToolBar = new kpToolToolBar (this, 2/*columns/rows*/, "Tool Box");
    connect (m_toolToolBar, SIGNAL (toolSelected (kpTool *)), SLOT (switchToTool (kpTool *)));
    for (int i = 0; i < m_numTools; i++)
    {
        if (m_tools [i])
            m_toolToolBar->registerTool (m_tools [i]);
    }

    m_toolNum = -1;
    m_previousToolNum = -1;

    m_toolToolBar->selectTool (brush);  // CONFIG: last used
}

kpMainWindow::~kpMainWindow ()
{
    m_actionOpenRecent->saveEntries (kapp->config ());

    delete m_commandHistory;

    switchToTool (-1);

    for (int i = 0; i < m_numTools; i++)
        delete m_tools [i];

    delete m_mainView;
    delete m_document;
    delete m_scrollView;
    delete m_viewManager;
}

kpDocument *kpMainWindow::document ()
{
    return m_document;
}

kpViewManager *kpMainWindow::viewManager ()
{
    return m_viewManager;
}

kpColorToolBar *kpMainWindow::colorToolBar ()
{
    return m_colorToolBar;
}

KCommandHistory *kpMainWindow::commandHistory ()
{
    return m_commandHistory;
}

QColor kpMainWindow::color (int which) const
{
#if DEBUG_KPMAINWINDOW && 0
    kdDebug () << "kpMainWindow::color (" << which << ")" << endl;
#endif
    return m_colorToolBar->color (which);
}

QColor kpMainWindow::color (const ButtonState &buttonState) const
{
    if (buttonState & Qt::LeftButton)
        return color (0);
    else
        return color (1);
}

kpTool *kpMainWindow::tool ()
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::tool() m_toolNum=" << m_toolNum
               << " ptr=" << (m_toolNum >= 0 ? m_tools [m_toolNum] : 0) << endl;
#endif
    return m_toolNum >= 0 ? m_tools [m_toolNum] : 0;
}

void kpMainWindow::slotUpdateCaption ()
{
    setCaption (m_configShowPath ? m_document->prettyURL () : m_document->filename (),
                m_document->isModified ());
}

void kpMainWindow::slotDocumentRestored ()
{
    m_document->setModified (false);
    slotUpdateCaption ();
}

bool kpMainWindow::queryClose ()
{
    if (!m_document || !m_document->isModified ())
        return true;  // ok to close current doc

    int result = KMessageBox::warningYesNoCancel (this,
                     i18n ("The document \"%1\" has been modified.\n"
                           "Do you want to save it?")
                         .arg (m_document->filename ()),
                    QString::null,
                    KStdGuiItem::save (), KStdGuiItem::discard ());

    switch (result)
    {
    case KMessageBox::Yes:
        return slotSave ();  // close only if save succeeds
    case KMessageBox::No:
        return true;  // close without saving
    default:
        return false;  // don't close current doc
    }
}

void kpMainWindow::slotNew (const KURL &url)
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

/*    if (!queryClose ())
        return;*/

    if (m_document && !m_document->isEmpty ())
    {
        // TODO: who is going to destruct it?
        kpMainWindow *win = new kpMainWindow (url);
        win->show ();
        return;
    }

    delete m_document;

    m_document = new kpDocument (400, 300, 32);
    if (!url.isEmpty ())
    {
        m_document->open (url, true /*create an empty doc with the same url if url !exist*/);
    }

    // status bar
    connect (m_document, SIGNAL (documentOpened ()), this, SLOT (slotUpdateStatusBar ()));
    connect (m_document, SIGNAL (sizeChanged (int, int)), this, SLOT (slotUpdateStatusBar (int, int)));
    connect (m_document, SIGNAL (colorDepthChanged (int)), this, SLOT (slotUpdateStatusBar (int)));

    // caption (url, modified)
    connect (m_document, SIGNAL (documentModified ()), this, SLOT (slotUpdateCaption ()));
    connect (m_document, SIGNAL (documentOpened ()), this, SLOT (slotUpdateCaption ()));
    connect (m_document, SIGNAL (documentSaved ()), this, SLOT (slotUpdateCaption ()));

    // command history
    connect (m_commandHistory, SIGNAL (documentRestored ()), this, SLOT (slotDocumentRestored ()));  // caption "!modified"
    connect (m_document, SIGNAL (documentSaved ()), m_commandHistory, SLOT (documentSaved ()));

    // sync document -> views
    connect (m_document, SIGNAL (contentsChanged (const QRect &)),
             m_viewManager, SLOT (updateViews (const QRect &)));
    connect (m_document, SIGNAL (sizeChanged (int, int)),
             m_viewManager, SLOT (resizeViews (int, int)));
    connect (m_document, SIGNAL (colorDepthChanged (int)),
             m_viewManager, SLOT (updateViews ()));

    setMainView (new kpView (m_scrollView->viewport (), "kpView", this,
                             m_document->width (), m_document->height ()));
    m_viewManager->resizeViews (m_document->width (), m_document->height ());

    slotUpdateStatusBar ();  // update doc size
    slotUpdateCaption ();  // Untitled to start with
    m_commandHistory->clear ();
}

void kpMainWindow::setMainView (kpView *view)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::setMainView (" << (int) view << ")" << endl;
#endif

    m_viewManager->unregisterView (m_mainView);
    delete m_mainView;

    m_mainView = view;
    if (m_mainView)
    {
        m_scrollView->addChild (m_mainView);
        m_viewManager->registerView (m_mainView);
        m_mainView->show ();

        m_actionSave->setEnabled (true);
        m_actionSaveAs->setEnabled (true);
        m_actionSaveAs->setEnabled (true);
        m_actionRevert->setEnabled (true);
        m_actionPrint->setEnabled (true);
        m_actionPrintPreview->setEnabled (true);
        m_actionClose->setEnabled (true);

        m_actionZoomIn->setEnabled (true);
        m_actionZoomOut->setEnabled (false);
        m_actionZoom->setEnabled (true);
        m_actionShowGrid->setEnabled (false);
    }
    else
    {
        m_actionSave->setEnabled (false);
        m_actionSaveAs->setEnabled (false);
        m_actionSaveAs->setEnabled (false);
        m_actionRevert->setEnabled (false);
        m_actionPrint->setEnabled (false);
        m_actionPrintPreview->setEnabled (false);
        m_actionClose->setEnabled (false);

        m_actionZoomIn->setEnabled (false);
        m_actionZoomOut->setEnabled (false);
        m_actionZoom->setEnabled (false);
        m_actionShowGrid->setEnabled (false);
    }
}

bool kpMainWindow::open (const QString &str, bool newDocSameNameIfNotExist)
{
    KURL url;
    url.setPath (str);

    return open (url, newDocSameNameIfNotExist);
}

bool kpMainWindow::open (const KURL &url, bool newDocSameNameIfNotExist)
{
    if (m_document && !m_document->isEmpty ())
    {
        // TODO: who is going to destruct it?
        // TODO: what if it can't open
        kpMainWindow *win = new kpMainWindow (url);
        win->show ();
        return true;
    }

    if (!m_document)
    {
        slotNew (url);
        return true;
    }

    if (!m_document->open (url, newDocSameNameIfNotExist))
        return false;

    if (KIO::NetAccess::exists (url))
        m_actionOpenRecent->addURL (url);

    setMainView (new kpView (m_scrollView->viewport (), "kpView", this,
                             m_document->width (), m_document->height ()));
    m_viewManager->resizeViews (m_document->width (), m_document->height ());

    m_commandHistory->clear();
    return true;
}

bool kpMainWindow::slotOpen ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    /*if (!queryClose ())
        return false;*/

    KURL url = KFileDialog::getImageOpenURL (":kolourpaint", this, i18n ("Open Image"));
    if (url.isEmpty ())
        return false;

    return open (url);
}

bool kpMainWindow::slotOpenRecent (const KURL &url)
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    /*if (!queryClose ())
        return false;*/

    return open (url);
}

bool kpMainWindow::slotSave ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    return save ();
}

bool kpMainWindow::save (bool localOnly)
{
    if (m_document->url ().isEmpty () ||
        KImageIO::mimeTypes (KImageIO::Writing).findIndex (m_document->mimetype ()) == -1 ||
        (localOnly && !m_document->url ().isLocalFile ()))
    {
        return saveAs (localOnly);
    }
    else
    {
        if (m_document->save ())
        {
            m_actionOpenRecent->addURL (m_document->url ());
            return true;
        }
        else
            return false;
    }
}

bool kpMainWindow::slotSaveAs ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    return saveAs ();
}

bool kpMainWindow::saveAs (bool localOnly)
{
    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Writing);
    if (mimeTypes.isEmpty ())
    {
        kdError () << "No KImageIO output mimetypes!" << endl;
        return false;
    }

    QString path = m_document->url ().path ();
    kdDebug () << "kpMainWindow::slotSaveAs currentPath=" << path << endl;
    KFileDialog *fd = new KFileDialog (path.isEmpty () ? ":kolourpaint" : path,
                                       QString::null, this, "fd", true);
    fd->setOperationMode (KFileDialog::Saving);
    if (localOnly)
        fd->setMode (KFile::File | KFile::LocalOnly);

    QString defaultMimeType;

    // use the current mimetype of the document (if available)
    //
    // this is so as to not stuff up users who are just changing the filename
    // but want to save in the same type
    QString docMimeType = m_document->mimetype ();
    if (!docMimeType.isEmpty () && mimeTypes.findIndex (docMimeType) > -1)
        defaultMimeType = docMimeType;

    if (defaultMimeType.isEmpty ())
    {
        if (mimeTypes.findIndex (m_configDefaultOutputMimetype) > -1)
            defaultMimeType = m_configDefaultOutputMimetype;
        else if (mimeTypes.findIndex ("image/x-bmp") > -1)
            defaultMimeType = "image/x-bmp";
        else
            defaultMimeType = mimeTypes.first ();
    }

#if DEBUG_KPMAINWINDOW
    kdDebug () << "mimeTypes=" << mimeTypes << endl;
#endif
    fd->setMimeFilter (mimeTypes, defaultMimeType);
    if (fd->exec ())
    {
        QString mimetype = fd->currentMimeFilter ();
        if (!m_document->saveAs (fd->selectedURL (), mimetype))
        {
            delete fd;
            return false;
        }
        else
        {
            m_actionOpenRecent->addURL (fd->selectedURL ());

            // user forced a mimetype (as opposed to selecting the same type as the current doc)
            // - probably wants to use it in the future
            if (mimetype != docMimeType)
            {
                KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
                configGroupSaver.config ()->writeEntry (kpSettingDefaultOutputMimetype, mimetype);
                configGroupSaver.config ()->sync ();
            }
        }
    }
    else
    {
    #if DEBUG_KPMAINWINDOW
        kdDebug () << "fd aborted" << endl;
    #endif
        delete fd;
        return false;
    }

    delete fd;
    return true;
}

bool kpMainWindow::slotRevert ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    if (!queryClose ())
        return false;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotRevert() reverting!" << endl;
#endif

    if (!m_document->open (m_document->url ()))
        return false;

    m_viewManager->resizeViews (m_document->width (), m_document->height ());
    m_commandHistory->clear();

    return true;
}

void kpMainWindow::slotPrint ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    KPrinter printer;

    if (!printer.setup (this))
        return;

    QPainter painter (&printer);
    painter.drawPixmap (0, 0, *m_document->pixmap ());
}

void kpMainWindow::slotPrintPreview ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

}

void kpMainWindow::slotClose ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotClose()" << endl;
#endif

    if (!queryClose ())
        return;

    setMainView (0);
    delete m_document; m_document = 0;
}

void kpMainWindow::slotQuit ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotQuit()" << endl;
#endif

    close ();  // will call queryClose()
}

void kpMainWindow::slotCut ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    if (m_viewManager->selectionActive ())
    {
        QApplication::setOverrideCursor (QCursor (Qt::WaitCursor));
        QApplication::clipboard ()->setPixmap (m_viewManager->tempPixmap (), QClipboard::Clipboard);
        m_viewManager->invalidateTempPixmap ();
        QApplication::restoreOverrideCursor ();
    }
}

void kpMainWindow::slotCopy ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    if (m_viewManager->selectionActive ())
    {
        QApplication::setOverrideCursor (QCursor (Qt::WaitCursor));
        QApplication::clipboard ()->setPixmap (m_viewManager->tempPixmap (), QClipboard::Clipboard);
        QApplication::restoreOverrideCursor ();
    }
}

void kpMainWindow::slotPaste ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    QApplication::setOverrideCursor (QCursor (Qt::WaitCursor));

    QPixmap pixmap = QApplication::clipboard ()->pixmap (QClipboard::Clipboard);

    if (!pixmap.isNull ())
    {
        m_viewManager->setTempPixmapAt (pixmap, QPoint (0, 0), kpViewManager::SelectionPixmap);
        slotToolRectSelection ();
    }

    QApplication::restoreOverrideCursor ();
}

void kpMainWindow::slotSelectAll ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    m_viewManager->setTempPixmapAt (*m_document->pixmap (), QPoint (0, 0), kpViewManager::SelectionPixmap);
    slotToolRectSelection ();
}

void kpMainWindow::slotDeselect ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    if (m_viewManager->selectionActive ())
        m_viewManager->invalidateTempPixmap ();
}

// virtual
void kpMainWindow::dragEnterEvent (QDragEnterEvent *e)
{
    e->accept (QImageDrag::canDecode (e) || KURLDrag::canDecode (e));
}

// virtual
void kpMainWindow::dropEvent (QDropEvent *e)
{
    QPixmap pixmap;
    KURL::List urls;

    if (QImageDrag::decode (e, pixmap))
    {
        if (!pixmap.isNull ())
        {
            m_viewManager->setTempPixmapAt (pixmap, QPoint (0, 0), kpViewManager::SelectionPixmap);
            slotToolRectSelection ();
        }
    }
    else if (KURLDrag::decode (e, urls))
    {
        for (KURL::List::ConstIterator it = urls.begin (); it != urls.end (); it++)
        {
            open (*it);
        }
    }
}

void kpMainWindow::slotZoomIn ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotZoomIn ()" << endl;
#endif

    m_actionZoom->setCurrentItem (m_actionZoom->currentItem () + 1);
    slotZoom ();
}

void kpMainWindow::slotZoomOut ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotZoomOut ()" << endl;
#endif

    m_actionZoom->setCurrentItem (m_actionZoom->currentItem () - 1);
    slotZoom ();
}

void kpMainWindow::slotZoom ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotZoom ()" << endl;
#endif

    QString text = m_actionZoom->currentText ();
    text.truncate (text.length () - 1);
    int zoomLevel = text.toInt ();

#if DEBUG_KPMAINWINDOW
    kdDebug () << "\tzoomText=" << text << " zoomLevel=" << zoomLevel << "%" << endl;
#endif

    m_actionZoomIn->setEnabled (m_actionZoom->currentItem () < (int) m_zoomList.count () - 1);
    m_actionZoomOut->setEnabled (m_actionZoom->currentItem () > 0);

    m_mainView->setZoomLevel (zoomLevel, zoomLevel);
    m_actionShowGrid->setEnabled (m_mainView->canShowGrid ());
    if (m_actionShowGrid->isEnabled ())
    {
        m_actionShowGrid->setChecked (m_configShowGrid);
        slotShowGrid ();
    }
    else
    {
        m_actionShowGrid->setChecked (false);
    }
}

void kpMainWindow::slotShowGrid ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotShowGrid ()" << endl;
#endif

    m_configShowGrid = m_actionShowGrid->isChecked ();
    m_mainView->showGrid (m_configShowGrid);
}

void kpMainWindow::slotShowPath ()
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotShowPath ()" << endl;
#endif

    m_configShowPath = m_actionShowPath->isChecked ();
    slotUpdateCaption ();
}

void kpMainWindow::slotConfigure ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kpConfigDialog *dialog = new kpConfigDialog ();
    dialog->exec ();
    delete dialog;
}

void kpMainWindow::slotKeyBindings ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    KKeyDialog::configure (actionCollection ());
    actionCollection ()->readShortcutSettings ();
}

void kpMainWindow::slotConfigureToolBars ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    // TODO: wrong
    KEditToolbar dialog (actionCollection ());
    if (dialog.exec ())
        createGUI ();
}

void kpMainWindow::switchToTool (kpTool *tool)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::switchToTool (" << tool << ")" << endl;
#endif

    for (int i = 0; i < m_numTools; i++)
    {
    #if DEBUG_KPMAINWINDOW && 0
        kdDebug () << "\tComparing with tool #" << i << ": ptr=" << m_tools [i] << endl;
    #endif
        if (m_tools [i] == tool)
        {
            switchToTool (i);
            break;
        }
    }
}

void kpMainWindow::switchToTool (int which)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::switchToTool (" << which << ")" << endl;
#endif

    if (which != -1 && m_toolNum == which)
        return;

    if (m_toolNum != -1)
    {
        kpTool *tool = m_tools [m_toolNum];
        tool->endInternal ();

        disconnect (tool, SIGNAL (beganDraw (const QPoint &)),
                    this, SLOT (slotUpdateStatusBar (const QPoint &)));
        disconnect (tool, SIGNAL (endedDraw (const QPoint &)),
                    this, SLOT (slotUpdateStatusBar (const QPoint &)));
        disconnect (tool, SIGNAL (mouseMoved (const QPoint &)),
                    this, SLOT (slotUpdateStatusBar (const QPoint &)));
        disconnect (tool, SIGNAL (mouseDragged (const QRect &)),
                    this, SLOT (slotUpdateStatusBar (const QRect &)));

        disconnect (m_colorToolBar, SIGNAL (foregroundColorChanged (const QColor &)),
                    tool, SLOT (slotForegroundColorChanged (const QColor &)));
        disconnect (m_colorToolBar, SIGNAL (backgroundColorChanged (const QColor &)),
                    tool, SLOT (slotBackgroundColorChanged (const QColor &)));
    }

    m_previousToolNum = m_toolNum;

    if (which != -1)
    {
        m_toolNum = which;

        if (m_toolNum != -1)
        {
            kpTool *tool = m_tools [m_toolNum];
            tool->beginInternal ();

            connect (tool, SIGNAL (beganDraw (const QPoint &)),
                     SLOT (slotUpdateStatusBar (const QPoint &)));
            connect (tool, SIGNAL (endedDraw (const QPoint &)),
                     SLOT (slotUpdateStatusBar (const QPoint &)));
            connect (tool, SIGNAL (mouseMoved (const QPoint &)),
                     SLOT (slotUpdateStatusBar (const QPoint &)));
            connect (tool, SIGNAL (mouseDragged (const QRect &)),
                     SLOT (slotUpdateStatusBar (const QRect &)));

            connect (m_colorToolBar, SIGNAL (foregroundColorChanged (const QColor &)),
                     tool, SLOT (slotForegroundColorChanged (const QColor &)));
            connect (m_colorToolBar, SIGNAL (backgroundColorChanged (const QColor &)),
                     tool, SLOT (slotBackgroundColorChanged (const QColor &)));
        }
    }
    else
        m_toolNum = -1;
}

void kpMainWindow::switchToPreviousTool ()
{
    if (m_previousToolNum != -1)
        switchToTool (m_previousToolNum);
}

void kpMainWindow::slotToolPen ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolPen()" << endl;
    switchToTool (0);
}

void kpMainWindow::slotToolLine ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolLine()" << endl;
    switchToTool (1);
}

void kpMainWindow::slotToolEllipse ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolEllipse()" << endl;
    switchToTool (2);
}

void kpMainWindow::slotToolColorPicker ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolColorPicker()" << endl;
    switchToTool (3);
}

void kpMainWindow::slotToolAirSpray ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolAirSpray()" << endl;
    switchToTool (4);
}

void kpMainWindow::slotToolRectangle ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolRectangle()" << endl;
    switchToTool (5);
}

void kpMainWindow::slotToolRoundedRectangle ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolRoundedRectangle()" << endl;
    switchToTool (6);
}

void kpMainWindow::slotToolPolygon ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolPolygon ()" << endl;
    switchToTool (7);
}

void kpMainWindow::slotToolFloodFill ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolFloodFill()" << endl;
    switchToTool (8);
}

void kpMainWindow::slotToolRotate ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolRotate()" << endl;
    switchToTool (9);
}

void kpMainWindow::slotToolBrush ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolBrush()" << endl;
    switchToTool (10);
}

void kpMainWindow::slotToolEraser ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolEraser()" << endl;
    switchToTool (11);
}

void kpMainWindow::slotToolRectSelection ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolRectSelection()" << endl;
    switchToTool (12);
}

void kpMainWindow::slotToolEllipticalSelection ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolEllipticalSelection()" << endl;
    switchToTool (14);
}

void kpMainWindow::slotToolFreeFormSelection ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolFreeFormSelection()" << endl;
    switchToTool (15);
}

void kpMainWindow::slotToolColorWasher ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kdDebug () << "slotToolColorWasher()" << endl;
    switchToTool (13);
}


/*
 * Image Menu
 */

void kpMainWindow::slotProperties ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;
}

void kpMainWindow::slotResizeScale ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kpToolResizeScaleDialog *dialog = new kpToolResizeScaleDialog (this);

    if (dialog->exec () && !dialog->isNoop ())
    {
        m_commandHistory->addCommand (
            new kpToolResizeScaleCommand (m_document, m_viewManager,
                                          dialog->imageWidth (), dialog->imageHeight (),
                                          dialog->scaleToFit ()));
    }
}

void kpMainWindow::slotFlip ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kpToolFlipDialog *dialog = new kpToolFlipDialog (this);

    if (dialog->exec () && !dialog->isNoopFlip ())
    {
        m_commandHistory->addCommand (
            new kpToolFlipCommand (m_document, m_viewManager,
                                   dialog->getHorizontalFlip (), dialog->getVerticalFlip ()));
    }
}

void kpMainWindow::slotRotate ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kpToolRotateDialog *dialog = new kpToolRotateDialog (this);

    if (dialog->exec () && !dialog->isNoopRotate ())
    {
        m_commandHistory->addCommand (
            new kpToolRotateCommand (m_document, m_viewManager, dialog->angle ()));
    }
}

void kpMainWindow::slotSkew ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    kpToolSkewDialog *dialog = new kpToolSkewDialog (this);

    if (dialog->exec () && !dialog->isNoop ())
    {
        m_commandHistory->addCommand (
            new kpToolSkewCommand (m_document, m_viewManager,
                                   dialog->horizontalAngle (), dialog->verticalAngle ()));
    }
}

void kpMainWindow::slotConvertToGrayscale ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

   m_commandHistory->addCommand (
        new kpToolConvertToGrayscaleCommand (m_document, m_viewManager));
}

void kpMainWindow::slotInvertColors ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

   m_commandHistory->addCommand (
        new kpToolInvertColorsCommand (m_document, m_viewManager));
}

void kpMainWindow::slotClear ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotClear ()" << endl;
#endif

    m_commandHistory->addCommand (
        new kpToolClearCommand (m_document, m_viewManager,
                                colorToolBar ()->backgroundColor ()));
}

void kpMainWindow::slotUpdateStatusBar ()
{
    if (m_document)
        slotUpdateStatusBar (m_document->width (), m_document->height (), m_document->colorDepth ());
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

void kpMainWindow::slotUpdateStatusBar (int docWidth, int docHeight)
{
    if (m_document && docWidth > 0 && docHeight > 0)
        slotUpdateStatusBar (docWidth, docHeight, m_document->colorDepth ());
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

void kpMainWindow::slotUpdateStatusBar (int docColorDepth)
{
    if (m_document && docColorDepth > 0)
        slotUpdateStatusBar (m_document->width (), m_document->height (), docColorDepth);
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

void kpMainWindow::slotUpdateStatusBar (int docWidth, int docHeight, int docColorDepth)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotUpdateStatusBar ("
               << docWidth << "x" << docHeight << "@" << docColorDepth << endl;
#endif

    if (m_document && docWidth > 0 && docHeight > 0 && docColorDepth > 0)
        statusBar ()->changeItem (i18n ("%1 x %2  (%3-bit)")
                                    .arg (docWidth).arg (docHeight)
                                    .arg (docColorDepth),
                                  KP_STATUSBAR_ITEM_DOC);
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}


// private
// this prevents coords like "-2,400" from being shown
bool kpMainWindow::legalDocPoint (const QPoint &point) const
{
   return (point.x () >= 0 && point.x () < m_document->width () &&
           point.y () >= 0 && point.y () < m_document->height ());
}


// public slot
void kpMainWindow::slotUpdateStatusBar (const QPoint &point)
{
    QString string;

    // we just don't display illegal points full stop
    if (legalDocPoint (point))
        string = i18n ("%1,%2").arg (point.x ()).arg (point.y ());

    statusBar ()->changeItem (string, KP_STATUSBAR_ITEM_POS);
    statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_SIZE);
}

// public slot
void kpMainWindow::slotUpdateStatusBar (const QRect &srect)
{
    int x1 = srect.left (), y1 = srect.top (), x2 = srect.right (), y2 = srect.bottom ();

    // we clip illegal points when dragging
    x1 = QMIN (QMAX (x1, 0), m_document->width () - 1);
    x2 = QMIN (QMAX (x2, 0), m_document->width () - 1);
    y1 = QMIN (QMAX (y1, 0), m_document->height () - 1);
    y2 = QMIN (QMAX (y2, 0), m_document->height () - 1);

    QRect rect = QRect (QPoint (x1, y1), QPoint (x2, y2));

    statusBar ()->changeItem (i18n ("%1,%2 - %3,%4")
                                    .arg (rect.left ())
                                    .arg (rect.top ())
                                    .arg (rect.right ())
                                    .arg (rect.bottom ()),
                                KP_STATUSBAR_ITEM_POS);
    statusBar ()->changeItem (i18n ("%1x%2")
                                    .arg (rect.width ())
                                    .arg (rect.height ()),
                                KP_STATUSBAR_ITEM_SIZE);
}

void kpMainWindow::slotActionShowGridToggled (bool on)
{
// TODO: KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE
    m_configShowGrid = on;

    KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    configGroupSaver.config ()->writeEntry (kpSettingShowGrid, m_configShowGrid);
    configGroupSaver.config ()->sync ();
}

void kpMainWindow::slotActionShowPathToggled (bool on)
{
    m_configShowPath = on;

    KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    configGroupSaver.config ()->writeEntry (kpSettingShowPath, m_configShowPath);
    configGroupSaver.config ()->sync ();
}

#include <dcopclient.h>
#include <qcstring.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qfile.h>
#include <kapplication.h>
#include <kmessagebox.h>

// private
void kpMainWindow::setAsWallpaper (bool centered)
{
    if (!m_document->url ().isLocalFile () || m_document->url ().isEmpty () || m_document->isModified ())
    {
        QString question;

        if (!m_document->url ().isLocalFile ())
            question = i18n ("Before this image can be set as the wallpaper, you must save it as a local file.\n"
                             "Do you want to save it?");
        else
            question = i18n ("Before this image can be set as the wallpaper, you must save it.\n"
                             "Do you want to save it?");

        int result = KMessageBox::questionYesNo (this,
                         question, QString::null,
                         KStdGuiItem::save (), KStdGuiItem::cancel ());

        if (result == KMessageBox::Yes)
        {
            // save() is smart enough to pop up a filedialog if it's a
            // remote file that should be saved locally
            if (!save (true/*localOnly*/))
            {
                // save failed or aborted - don't set the wallpaper
                return;
            }
        }
        else
        {
            // don't want to save - don't set wallpaper
            return;
        }
    }


    QByteArray data;
    QDataStream dataStream (data, IO_WriteOnly);

    // write path
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::setAsWallpaper() path=" << m_document->url ().path () << endl;
#endif
    dataStream << QString (m_document->url ().path ());

    // write position:
    //
    // SYNC: kdebase/kcontrol/background/bgsettings.h:
    // 1 = Centered
    // 2 = Tiled
    // 6 = Scaled
    // 8 = lastWallpaperMode
    //
    // Why restrict the user to Centered & Tiled?
    // Why don't we let the user choose if it should be common to all desktops?
    // Why don't we rewrite the Background control page?
    //
    // Answer: This is supposed to be a quick & convenient feature.
    //
    // If you want more options, go to kcontrol for that kind of
    // flexiblity.  We don't want to slow down average users, who see way too
    // many dialogs already and probably haven't even heard of "Centered Maxpect"...
    //
    dataStream << int (centered ? 1 : 2);


    // I'm going to all this trouble because the user might not have kdebase
    // installed so kdebase/kdesktop/KBackgroundIface.h might not be around
    // to be compiled in (where user == developer :))
    if (!KApplication::dcopClient ()->send ("kdesktop", "KBackgroundIface",
                                            "setWallpaper(QString,int)", data))
    {
        KMessageBox::sorry (this, i18n ("Could not change wallpaper."));
    }
}

void kpMainWindow::slotSetAsWallpaperCentered ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    setAsWallpaper (true/*centered*/);
}

void kpMainWindow::slotSetAsWallpaperTiled ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    setAsWallpaper (false/*tiled*/);
}

#include <kpmainwindow.moc>
