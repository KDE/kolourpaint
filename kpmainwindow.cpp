
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


#include <qdragobject.h>
#include <qpainter.h>
#include <qscrollview.h>
#include <qtimer.h>

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurldrag.h>

#include <kpcolortoolbar.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kpselectiondrag.h>
#include <kpthumbnail.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kpmainwindow.h>
#include <kpview.h>
#include <kpviewmanager.h>

#if DEBUG_KP_MAIN_WINDOW
    #include <qdatetime.h>
#endif


kpMainWindow::kpMainWindow ()
    : KMainWindow (0/*parent*/, "mainWindow"),
      m_isFullyConstructed (false)
{
    init ();
    open (KURL (), true/*create an empty doc*/);

    m_isFullyConstructed = true;
}

kpMainWindow::kpMainWindow (const KURL &url)
    : KMainWindow (0/*parent*/, "mainWindow"),
      m_isFullyConstructed (false)
{
    init ();
    open (url, true/*create an empty doc with the same url if url !exist*/);

    m_isFullyConstructed = true;
}

kpMainWindow::kpMainWindow (kpDocument *newDoc)
    : KMainWindow (0/*parent*/, "mainWindow"),
      m_isFullyConstructed (false)
{
    init ();
    setDocument (newDoc);

    m_isFullyConstructed = true;
}


// public
double kpMainWindow::configColorSimilarity () const
{
    return m_configColorSimilarity;
}

// public
void kpMainWindow::configSetColorSimilarity (double val)
{
    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingColorSimilarity, m_configColorSimilarity = val);
    cfg->sync ();
}


// private
void kpMainWindow::readGeneralSettings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tkpMainWindow(" << name () << ")::readGeneralSettings()" << endl;
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    m_configFirstTime = cfg->readBoolEntry (kpSettingFirstTime, true);
    m_configShowGrid = cfg->readBoolEntry (kpSettingShowGrid, false);
    m_configShowPath = cfg->readBoolEntry (kpSettingShowPath, false);
    m_configColorSimilarity = cfg->readDoubleNumEntry (kpSettingColorSimilarity, 0);

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\t\tGeneral Settings: firstTime=" << m_configFirstTime
               << " showGrid=" << m_configShowGrid
               << " showPath=" << m_configShowPath
               << " colorSimilarity=" << m_configColorSimilarity
               << endl;
#endif
}

// private
void kpMainWindow::readThumbnailSettings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tkpMainWindow(" << name () << ")::readThumbnailSettings()" << endl;
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupThumbnail);
    KConfigBase *cfg = cfgGroupSaver.config ();

    m_configThumbnailShown = cfg->readBoolEntry (kpSettingThumbnailShown, false);
    m_configThumbnailGeometry = cfg->readRectEntry (kpSettingThumbnailGeometry);

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\t\tThumbnail Settings: shown=" << m_configThumbnailShown
               << " geometry=" << m_configThumbnailGeometry
               << endl;
#endif
}

// private
void kpMainWindow::init ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow(" << name () << ")::init()" << endl;
    QTime totalTime; totalTime.start ();
    QTime time; time.start ();
#endif

    d = new kpMainWindowPrivate;

    m_scrollView = 0;
    m_mainView = 0;
    m_thumbnail = 0;
    m_thumbnailView = 0;
    m_document = 0;
    m_viewManager = 0;
    m_colorToolBar = 0;
    m_toolToolBar = 0;
    m_commandHistory = 0;
    m_statusBarCreated = false;
    m_settingSelectionTransparency = 0;
    m_settingTextStyle = 0;


    //
    // set mainwindow properties
    //

    setMinimumSize (320, 260);
    setAcceptDrops (true);
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: little init = " << time.restart () << "msec" << endl;
#endif


    //
    // read config
    //

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realise what other processes have done e.g. Settings / Show Path
    kapp->config ()->reparseConfiguration ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: reparseConfig = " << time.restart () << "msec" << endl;
#endif

    readGeneralSettings ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: readGeneralSettings = " << time.restart () << "msec" << endl;
#endif

    readThumbnailSettings ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: readThumbnailSettings = " << time.restart () << "msec" << endl;
#endif


    //
    // create GUI
    //

    setupActions ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: setupActions = " << time.restart () << "msec" << endl;
#endif

    createStatusBar ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: createStatusBar = " << time.restart () << "msec" << endl;
#endif

    createGUI ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: createGUI = " << time.restart () << "msec" << endl;
#endif


    //
    // create more GUI
    //

    m_colorToolBar = new kpColorToolBar (this, "Color Box");
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: new kpColorToolBar = " << time.restart () << "msec" << endl;
#endif

    setupTools ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: setupTools = " << time.restart () << "msec" << endl;
#endif

    m_scrollView = new QScrollView (this, "scrollView", Qt::WStaticContents | Qt::WNoAutoErase);
    connect (m_scrollView, SIGNAL (contentsMoving (int, int)),
             this, SLOT (slotScrollViewAboutToScroll ()));
    setCentralWidget (m_scrollView);
    m_scrollView->show ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: m_scrollView = " << time.restart () << "msec" << endl;
#endif


    //
    // set initial pos/size of GUI
    //

    setAutoSaveSettings ();

    // Put our non-XMLGUI toolbars in a sane place, the first time around
    // (have to do this _after_ setAutoSaveSettings as that applies default
    //  (i.e. random) settings to the toolbars)
    if (m_configFirstTime)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tfirstTime: positioning toolbars" << endl;
    #endif

        m_toolToolBar->setBarPos (KToolBar::Left);
        m_colorToolBar->setBarPos (KToolBar::Bottom);

        KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
        KConfigBase *cfg = cfgGroupSaver.config ();

        cfg->writeEntry (kpSettingFirstTime, m_configFirstTime = false);
        cfg->sync ();
    }

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tall done in " << totalTime.elapsed () << "msec" << endl;
#endif
}

kpMainWindow::~kpMainWindow ()
{
    m_isFullyConstructed = false;

    // delete document & views
    setDocument (0);

    delete m_commandHistory; m_commandHistory = 0;
    delete m_scrollView; m_scrollView = 0;

    delete d; d = 0;
}


// public
kpDocument *kpMainWindow::document () const
{
    return m_document;
}

// public
kpViewManager *kpMainWindow::viewManager () const
{
    return m_viewManager;
}

// public
kpColorToolBar *kpMainWindow::colorToolBar () const
{
    return m_colorToolBar;
}

// public
kpToolToolBar *kpMainWindow::toolToolBar () const
{
    return m_toolToolBar;
}

// public
kpCommandHistory *kpMainWindow::commandHistory () const
{
    return m_commandHistory;
}


// private
void kpMainWindow::setupActions ()
{
    setupFileMenuActions ();
    setupEditMenuActions ();
    setupViewMenuActions ();
    setupImageMenuActions ();
    setupSettingsMenuActions ();
    setupTextToolBarActions ();
}

// private
void kpMainWindow::enableDocumentActions (bool enable)
{
    enableFileMenuDocumentActions (enable);
    enableEditMenuDocumentActions (enable);
    enableViewMenuDocumentActions (enable);
    enableImageMenuDocumentActions (enable);
    enableSettingsMenuDocumentActions (enable);
}


// private
void kpMainWindow::setDocument (kpDocument *newDoc)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::setDocument (" << newDoc << ")" << endl;
#endif

    // is it a close operation?
    if (!newDoc)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tdisabling actions" << endl;
    #endif

        // sync with the bit marked "sync" below

        if (m_colorToolBar)
            m_colorToolBar->setEnabled (false);
        else
        {
            kdError () << "kpMainWindow::setDocument() without colorToolBar"
                       << endl;
        }

        enableTextToolBarActions (false);
    }

    // Always disable the tools.
    // If we decide to open a new document/mainView we want
    // kpTool::begin() to be called again e.g. in case it sets the cursor.
    // kpViewManager won't do this because we nuke it to avoid stale state.
    enableToolsDocumentActions (false);

    if (!newDoc)
    {
        enableDocumentActions (false);
    }

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tdestroying views" << endl;
#endif

    delete m_mainView; m_mainView = 0;
    slotDestroyThumbnail ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tdestroying viewManager" << endl;
#endif

    // viewManager will die and so will the selection
    m_actionCopy->setEnabled (false);
    m_actionCut->setEnabled (false);
    m_actionDelete->setEnabled (false);
    m_actionDeselect->setEnabled (false);
    d->m_actionCopyToFile->setEnabled (false);

    delete m_viewManager; m_viewManager = 0;

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tdestroying document" << endl;
    kdDebug () << "\t\tm_document=" << m_document << endl;
#endif
    // destroy current document
    delete m_document;
    m_document = newDoc;


    // not a close operation?
    if (m_document)
    {
        if (m_document->mainWindow () != this)
        {
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tchanging doc's mainWindow from "
                       << m_document->mainWindow ()
                       << " to this="
                       << this
                       << endl;
        #endif
            m_document->setMainWindow (this);
        }

    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () <<"\tcreating viewManager" << endl;
    #endif
        m_viewManager = new kpViewManager (this);

    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tcreating views" << endl;
    #endif
        m_mainView = new kpView (m_scrollView->viewport (), "mainView", this,
                                 m_document->width (), m_document->height ());
        if (m_scrollView)
            m_scrollView->addChild (m_mainView);
        else
            kdError () << "kpMainWindow::setDocument() without scrollView" << endl;
        m_viewManager->registerView (m_mainView);
        m_mainView->show ();

    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\thooking up document signals" << endl;
    #endif

        // Copy/Cut/Deselect/Delete
        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 m_actionCut, SLOT (setEnabled (bool)));
        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 m_actionCopy, SLOT (setEnabled (bool)));
        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 m_actionDelete, SLOT (setEnabled (bool)));
        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 m_actionDeselect, SLOT (setEnabled (bool)));
        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 d->m_actionCopyToFile, SLOT (setEnabled (bool)));

        // this code won't actually enable any actions at this stage
        // (fresh document) but better safe than sorry
        m_actionCopy->setEnabled (m_document->selection ());
        m_actionCut->setEnabled (m_document->selection ());
        m_actionDeselect->setEnabled (m_document->selection ());
        m_actionDelete->setEnabled (m_document->selection ());
        d->m_actionCopyToFile->setEnabled (m_document->selection ());

        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 this, SLOT (slotImageMenuUpdateDueToSelection ()));
        connect (m_document, SIGNAL (selectionIsTextChanged (bool)),
                 this, SLOT (slotImageMenuUpdateDueToSelection ()));

        // Status bar
        connect (m_document, SIGNAL (documentOpened ()),
                 this, SLOT (slotUpdateStatusBar ()));

        connect (m_document, SIGNAL (sizeChanged (const QSize &)),
                 this, SLOT (slotUpdateStatusBarDocSize (const QSize &)));
        connect (m_document, SIGNAL (colorDepthChanged (int)),
                 this, SLOT (slotUpdateStatusBarDocDepth (int)));

        // Caption (url, modified)
        connect (m_document, SIGNAL (documentModified ()),
                 this, SLOT (slotUpdateCaption ()));
        connect (m_document, SIGNAL (documentOpened ()),
                 this, SLOT (slotUpdateCaption ()));
        connect (m_document, SIGNAL (documentSaved ()),
                 this, SLOT (slotUpdateCaption ()));

        // File/Reload action only available with non-empty URL
        connect (m_document, SIGNAL (documentSaved ()),
                 this, SLOT (slotEnableReload ()));

        connect (m_document, SIGNAL (documentSaved ()),
                 this, SLOT (slotEnableSettingsShowPath ()));

        // Command history
        if (m_commandHistory)
        {
            connect (m_commandHistory, SIGNAL (documentRestored ()),
                     this, SLOT (slotDocumentRestored ()));  // caption "!modified"
            connect (m_document, SIGNAL (documentSaved ()),
                     m_commandHistory, SLOT (documentSaved ()));
        }
        else
        {
            kdError () << "kpMainWindow::setDocument() without commandHistory"
                       << endl;
        }

        // Sync document -> views
        connect (m_document, SIGNAL (contentsChanged (const QRect &)),
                 m_viewManager, SLOT (updateViews (const QRect &)));
        connect (m_document, SIGNAL (sizeChanged (int, int)),
                 m_viewManager, SLOT (resizeViews (int, int)));
        connect (m_document, SIGNAL (colorDepthChanged (int)),
                 m_viewManager, SLOT (updateViews ()));

    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tenabling actions" << endl;
    #endif

        // sync with the bit marked "sync" above

        if (m_colorToolBar)
            m_colorToolBar->setEnabled (true);
        else
        {
            kdError () << "kpMainWindow::setDocument() without colorToolBar"
                       << endl;
        }


        // Hide the text toolbar - it will be shown by kpToolText::begin()
        enableTextToolBarActions (false);

        enableToolsDocumentActions (true);

        enableDocumentActions (true);

    // TODO: The thumbnail auto zoom doesn't work because it thinks its
    //       width == 1 when !this->isShown().  So for consistency,
    //       never create the thumbnail.
    #if 0
        if (m_configThumbnailShown)
        {
            if (isShown ())
            {
            #if DEBUG_KP_MAIN_WINDOW
                kdDebug () << "\tcreating thumbnail immediately" << endl;
            #endif
                slotCreateThumbnail ();
            }
            // this' geometry is weird ATM
            else
            {
            #if DEBUG_KP_MAIN_WINDOW
                kdDebug () << "\tcreating thumbnail LATER" << endl;
            #endif
                QTimer::singleShot (0, this, SLOT (slotCreateThumbnail ()));
            }
        }
    #endif
    }

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tupdating mainWindow elements" << endl;
#endif

    slotImageMenuUpdateDueToSelection ();
    slotUpdateStatusBar ();
    slotUpdateCaption ();  // Untitled to start with
    slotEnableReload ();
    slotEnableSettingsShowPath ();

    if (m_commandHistory)
        m_commandHistory->clear ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tdocument and views ready to go!" << endl;
#endif
}


// private virtual [base KMainWindow]
bool kpMainWindow::queryClose ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::queryClose()" << endl;
#endif
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (!m_document || !m_document->isModified ())
        return true;  // ok to close current doc

    int result = KMessageBox::warningYesNoCancel (this,
                     i18n ("The document \"%1\" has been modified.\n"
                           "Do you want to save it?")
                         .arg (m_document->prettyFilename ()),
                    QString::null/*caption*/,
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


// private virtual [base QWidget]
void kpMainWindow::dragEnterEvent (QDragEnterEvent *e)
{
    e->accept (kpSelectionDrag::canDecode (e) ||
               KURLDrag::canDecode (e) ||
               QTextDrag::canDecode (e));
}

// private virtual [base QWidget]
void kpMainWindow::dropEvent (QDropEvent *e)
{
    kpSelection sel;
    KURL::List urls;
    QString text;

    if (kpSelectionDrag::decode (e, sel/*ref*/, pasteWarnAboutLossInfo ()))
    {
        sel.setTransparency (selectionTransparency ());
        paste (sel);
    }
    else if (KURLDrag::decode (e, urls/*ref*/))
    {
        for (KURL::List::ConstIterator it = urls.begin (); it != urls.end (); it++)
        {
            open (*it);
        }
    }
    else if (QTextDrag::decode (e, text/*ref*/))
    {
        pasteText (text, true/*force new text selection*/);
    }
}


// private slot
void kpMainWindow::slotScrollViewAboutToScroll ()
{
    if (tool ())
    {
        // TODO: can tool() become 0 after the event loop?
        QTimer::singleShot (0, tool (), SLOT (somethingBelowTheCursorChanged ()));
    }
}


// private virtual [base QWidget]
void kpMainWindow::moveEvent (QMoveEvent * /*e*/)
{
    if (m_thumbnail)
    {
        // Disabled because it lags too far behind the mainWindow
        // m_thumbnail->move (m_thumbnail->pos () + (e->pos () - e->oldPos ()));

        notifyThumbnailGeometryChanged ();
    }
}


// public
void kpMainWindow::drawTransparentBackground (QPainter *painter,
                                              int /*viewWidth*/, int /*viewHeight*/,
                                              const QRect &rect,
                                              bool isPreview)
{
    const int cellSize = !isPreview ? 16 : 10;

    int starty = rect.y ();
    if (starty % cellSize)
        starty -= (starty % cellSize);

    int startx = rect.x ();
    if (startx % cellSize)
        startx -= (startx % cellSize);

    painter->save ();
    for (int y = starty; y <= rect.bottom (); y += cellSize)
    {
        for (int x = startx; x <= rect.right (); x += cellSize)
        {
            bool parity = (x / cellSize + y / cellSize) % 2;
            QColor col;

            if (parity)
            {
                if (!isPreview)
                    col = QColor (213, 213, 213);
                else
                    col = QColor (224, 224, 224);
            }
            else
                col = Qt::white;

            painter->fillRect (x - rect.x (), y - rect.y (), cellSize, cellSize,
                               col);
        }
    }
    painter->restore ();
}


// private slot
void kpMainWindow::slotUpdateCaption ()
{
    if (m_document)
    {
        setCaption (m_configShowPath ? m_document->prettyURL ()
                                     : m_document->prettyFilename (),
                    m_document->isModified ());
    }
    else
    {
        setCaption (QString::null, false);
    }
}

// private slot
void kpMainWindow::slotDocumentRestored ()
{
    if (m_document)
        m_document->setModified (false);
    slotUpdateCaption ();
}


#include <kpmainwindow.moc>
