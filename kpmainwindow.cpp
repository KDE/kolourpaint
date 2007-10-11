
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
#include <kpmainwindow_p.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qtimer.h>

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
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
#include <kpsinglekeytriggersaction.h>
#include <kpthumbnail.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kpviewmanager.h>
#include <kpviewscrollablecontainer.h>
#include <kpwidgetmapper.h>
#include <kpzoomedthumbnailview.h>
#include <kpzoomedview.h>

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
    d->m_moreEffectsDialogLastEffect = cfg->readNumEntry (kpSettingMoreEffectsLastEffect);
    d->m_resizeScaleDialogLastKeepAspect = cfg->readBoolEntry (kpSettingResizeScaleLastKeepAspect, false);


#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\t\tGeneral Settings: firstTime=" << m_configFirstTime
               << " showGrid=" << m_configShowGrid
               << " showPath=" << m_configShowPath
               << " colorSimilarity=" << m_configColorSimilarity
               << " moreEffectsDialogLastEffect=" << d->m_moreEffectsDialogLastEffect
               << " resizeScaleDialogLastKeepAspect=" << d->m_resizeScaleDialogLastKeepAspect
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
    m_configZoomedThumbnail = cfg->readBoolEntry (kpSettingThumbnailZoomed, true);
    d->m_configThumbnailShowRectangle = cfg->readBoolEntry (kpSettingThumbnailShowRectangle, true);

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\t\tThumbnail Settings: shown=" << m_configThumbnailShown
               << " geometry=" << m_configThumbnailGeometry
               << " zoomed=" << m_configZoomedThumbnail
               << " showRectangle=" << d->m_configThumbnailShowRectangle
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

    m_docResizeToBeCompleted = false;


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

    m_colorToolBar = new kpColorToolBar (i18n ("Color Box"), this, "Color Box");
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: new kpColorToolBar = " << time.restart () << "msec" << endl;
#endif

    createToolBox ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tTIME: createToolBox = " << time.restart () << "msec" << endl;
#endif

    m_scrollView = new kpViewScrollableContainer (this, "scrollView");
    connect (m_scrollView, SIGNAL (beganDocResize ()),
             this, SLOT (slotBeganDocResize ()));
    connect (m_scrollView, SIGNAL (continuedDocResize (const QSize &)),
             this, SLOT (slotContinuedDocResize (const QSize &)));
    connect (m_scrollView, SIGNAL (cancelledDocResize ()),
             this, SLOT (slotCancelledDocResize ()));
    connect (m_scrollView, SIGNAL (endedDocResize (const QSize &)),
             this, SLOT (slotEndedDocResize (const QSize &)));

    connect (m_scrollView, SIGNAL (statusMessageChanged (const QString &)),
             this, SLOT (slotDocResizeMessageChanged (const QString &)));

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


// private virtual [base KMainWindow]
void kpMainWindow::readProperties (KConfig *cfg)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow<" << this << ">::readProperties()" << endl;
#endif

    // No document at all?
    if (!cfg->hasKey (kpSessionSettingDocumentUrl))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tno url - no document" << endl;
    #endif
        setDocument (0);
    }
    // Have a document.
    else
    {
        const KURL url (cfg->readEntry (kpSessionSettingDocumentUrl));
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\turl=" << url << endl;
    #endif

        const QSize notFromURLDocSize =
            cfg->readSizeEntry (kpSessionSettingNotFromUrlDocumentSize);

        // Is from URL?
        if (notFromURLDocSize.isEmpty ())
        {
            // If this fails, the empty document that kpMainWindow::kpMainWindow()
            // created is left untouched.
            openInternal (url, defaultDocSize (),
                false/*show error message if url !exist*/);
        }
        // Not from URL?
        else
        {
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tnot from url; doc size=" << notFromURLDocSize << endl;
        #endif
            // Either we have an empty URL or we have a "kolourpaint doesnotexist.png"
            // URL.  Regarding the latter case, if a file now actually exists at that
            // URL, we do open it - ignoring notFromURLDocSize - to avoid putting
            // the user in a situation where he might accidentally overwrite an
            // existing file.
            openInternal (url, notFromURLDocSize,
                true/*create an empty doc with the same url if url !exist*/);
        }
    }

}

// private virtual [base KMainWindow]
// WARNING: KMainWindow API Doc says "No user interaction is allowed
//          in this function!"
void kpMainWindow::saveProperties (KConfig *cfg)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow<" << this << ">::saveProperties()" << endl;
#endif

    // No document at all?
    if (!m_document)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tno url - no document" << endl;
    #endif
    }
    // Have a document.
    else
    {
        // Save URL in all cases:
        //
        //    a) m_document->isFromURL()
        //    b) !m_document->isFromURL() [save size in this case]
        //       i) No URL
        //       ii) URL (from "kolourpaint doesnotexist.png")

        const KURL url = m_document->url ();
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\turl=" << url << endl;
    #endif
        cfg->writeEntry (kpSessionSettingDocumentUrl, url.url ());

        // Not from URL e.g. "kolourpaint doesnotexist.png"?
        //
        // Note that "kolourpaint doesexist.png" is considered to be from
        // a URL even if it was deleted in the background (hence the
        // "false" arg to isFromURL()).  This is because the user expects
        // it to be from a URL, so when we session restore, we pop up a
        // "cannot find file" dialog, instead of silently creating a new,
        // blank document.
        if (!m_document->isFromURL (false/*don't bother checking exists*/))
        {
            // If we don't have a URL either:
            //
            // a) it was not modified - so we can use either width() or
            //    constructorWidth() (they'll be equal).
            // b) the changes were discarded so we use the initial width,
            //    constructorWidth().
            //
            // Similarly for height() and constructorHeight().
            const QSize docSize (m_document->constructorWidth (),
                                 m_document->constructorHeight ());
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tnot from url; doc size=" << docSize << endl;
        #endif
            cfg->writeEntry (kpSessionSettingNotFromUrlDocumentSize, docSize);
        }


        // Local session save i.e. queryClose() was not called beforehand
        // (see QApplication::saveState())?
    #if 0
        if (m_document->isModified ())
        {
            // TODO: Implement by saving the current image to a persistent file.
            //       We do this instead of saving/mutating the backing image file
            //       as no one expects a file save on a session save without a
            //       "do you want to save" dialog first.
            //
            //       I don't think any KDE application implements local session saving.
            //
            //       --- The below code does not compile but shows you want to do ---

            // Create unique name for the document in this main window.
            const KURL tempURL = homeDir +
                "kolourpaint session " + sessionID +
                mainWindowPtrToString + ".png";
            // TODO: Use lossless PNG saving options.
            kpDocumentSaveOptions pngSaveOptions;

            if (kpDocument::savePixmapToFile (m_document->pixmapWithSelection (),
                    tempURL,
                    pngSaveOptions, *m_document->metaInfo (),
                    false/*no overwrite prompt*/,
                    false/*no lossy prompt*/,
                    this))
            {
                // readProperties() will still open kpSessionSettingDocumentUrl
                // (as that's the expected URL) and will then add commands to:
                //
                // 1. Resize the document to the size of image at
                //    kpSessionSettingDocumentUnsavedContentsUrl, if the sizes
                //    differ.
                // 2. Paste the kpSessionSettingDocumentUnsavedContentsUrl image
                //    (setting the main window's selection mode to opaque beforehand).
                //
                // It will then delete the file at
                // kpSessionSettingDocumentUnsavedContentsUrl.
                cfg->writeEntry (kpSessionSettingDocumentUnsavedContentsUrl,
                    tempURL.url ());
            }
            else
            {
                // Not much we can do - we aren't allowed to throw up a dialog.
            }
        }
    #endif
    }
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
    setupHelpMenuActions ();

    setupTextToolBarActions ();
    setupToolActions ();
}

// private
void kpMainWindow::enableDocumentActions (bool enable)
{
    enableFileMenuDocumentActions (enable);
    enableEditMenuDocumentActions (enable);
    enableViewMenuDocumentActions (enable);
    enableImageMenuDocumentActions (enable);
    enableSettingsMenuDocumentActions (enable);
    enableHelpMenuDocumentActions (enable);
}


// public
bool kpMainWindow::actionsSingleKeyTriggersEnabled () const
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::actionsSingleKeyTriggersEnabled()" << endl;
    QTime timer; timer.start ();
#endif

    if (m_toolToolBar)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\ttime=" << timer.restart () << endl;
    #endif
        return m_toolToolBar->toolsSingleKeyTriggersEnabled ();
    }

    return (m_actionPrevToolOptionGroup1->singleKeyTriggersEnabled () ||
            m_actionNextToolOptionGroup1->singleKeyTriggersEnabled () ||
            m_actionPrevToolOptionGroup2->singleKeyTriggersEnabled () ||
            m_actionNextToolOptionGroup2->singleKeyTriggersEnabled ());
}

// public
void kpMainWindow::enableActionsSingleKeyTriggers (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::enableActionsSingleKeyTriggers("
               << enable << ")" << endl;
    QTime timer; timer.start ();
#endif

    if (m_toolToolBar)
        m_toolToolBar->enableToolsSingleKeyTriggers (enable);

    m_actionPrevToolOptionGroup1->enableSingleKeyTriggers (enable);
    m_actionNextToolOptionGroup1->enableSingleKeyTriggers (enable);
    m_actionPrevToolOptionGroup2->enableSingleKeyTriggers (enable);
    m_actionNextToolOptionGroup2->enableSingleKeyTriggers (enable);

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\ttime=" << timer.restart () << endl;
#endif
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
    m_actionCopyToFile->setEnabled (false);

    delete m_viewManager; m_viewManager = 0;

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tdestroying document" << endl;
    kdDebug () << "\t\tm_document=" << m_document << endl;
#endif
    // destroy current document
    delete m_document;
    m_document = newDoc;


    if (!m_lastCopyToURL.isEmpty ())
        m_lastCopyToURL.setFileName (QString::null);
    m_copyToFirstTime = true;

    if (!m_lastExportURL.isEmpty ())
        m_lastExportURL.setFileName (QString::null);
    m_exportFirstTime = true;


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
        m_mainView = new kpZoomedView (m_document, m_toolToolBar, m_viewManager,
                                       0/*buddyView*/,
                                       m_scrollView,
                                       m_scrollView->viewport (), "mainView");
        if (m_scrollView)
        {
            m_scrollView->addChild (m_mainView);
        }
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
                 m_actionCopyToFile, SLOT (setEnabled (bool)));

        // this code won't actually enable any actions at this stage
        // (fresh document) but better safe than sorry
        m_actionCopy->setEnabled (m_document->selection ());
        m_actionCut->setEnabled (m_document->selection ());
        m_actionDeselect->setEnabled (m_document->selection ());
        m_actionDelete->setEnabled (m_document->selection ());
        m_actionCopyToFile->setEnabled (m_document->selection ());

        connect (m_document, SIGNAL (selectionEnabled (bool)),
                 this, SLOT (slotImageMenuUpdateDueToSelection ()));
        connect (m_document, SIGNAL (selectionIsTextChanged (bool)),
                 this, SLOT (slotImageMenuUpdateDueToSelection ()));

        // Status bar
        connect (m_document, SIGNAL (documentOpened ()),
                 this, SLOT (recalculateStatusBar ()));

        connect (m_document, SIGNAL (sizeChanged (const QSize &)),
                 this, SLOT (setStatusBarDocSize (const QSize &)));

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
                 m_viewManager, SLOT (adjustViewsToEnvironment ()));

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
    recalculateStatusBar ();
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
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::dropEvent" << e->pos () << endl;
#endif

    kpSelection sel;
    KURL::List urls;
    QString text;

    if (kpSelectionDrag::decode (e, sel/*ref*/, pasteWarnAboutLossInfo ()))
    {
        sel.setTransparency (selectionTransparency ());
        // TODO: drop at point like with QTextDrag below?
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
        QPoint selTopLeft = KP_INVALID_POINT;
        const QPoint globalPos = QWidget::mapToGlobal (e->pos ());
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tpos toGlobal=" << globalPos << endl;
    #endif

        kpView *view = 0;

        if (m_viewManager)
        {
            view = m_viewManager->viewUnderCursor ();
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\t\tviewUnderCursor=" << view << endl;
        #endif
            if (!view)
            {
                // HACK: see kpViewManager::setViewUnderCursor() to see why
                //       it's not reliable
            #if DEBUG_KP_MAIN_WINDOW
                kdDebug () << "\t\tattempting to discover view" << endl;

                if (m_mainView && m_scrollView)
                {
                    kdDebug () << "\t\t\tmainView->globalRect="
                            << kpWidgetMapper::toGlobal (m_mainView, m_mainView->rect ())
                            << " scrollView->globalRect="
                            << kpWidgetMapper::toGlobal (m_scrollView,
                                    QRect (0, 0,
                                            m_scrollView->visibleWidth (),
                                            m_scrollView->visibleHeight ()))
                            << endl;
                }
            #endif
                if (m_thumbnailView &&
                    kpWidgetMapper::toGlobal (m_thumbnailView, m_thumbnailView->rect ())
                        .contains (globalPos))
                {
                    // TODO: Code will never get executed.
                    //       Thumbnail doesn't accept drops.
                    view = m_thumbnailView;
                }
                else if (m_mainView &&
                         kpWidgetMapper::toGlobal (m_mainView, m_mainView->rect ())
                             .contains (globalPos) &&
                         m_scrollView &&
                         kpWidgetMapper::toGlobal (m_scrollView,
                             QRect (0, 0,
                                    m_scrollView->visibleWidth (),
                                    m_scrollView->visibleHeight ()))
                             .contains (globalPos))
                {
                    view = m_mainView;
                }
            }
        }

        if (view)
        {
            const QPoint viewPos = view->mapFromGlobal (globalPos);
            const QPoint docPoint = view->transformViewToDoc (viewPos);

            // viewUnderCursor() is hacky and can return a view when we aren't
            // over one thanks to drags.
            if (m_document && m_document->rect ().contains (docPoint))
            {
                selTopLeft = docPoint;

                // TODO: In terms of doc pixels, would be inconsistent behaviour
                //       based on zoomLevel of view.
                // selTopLeft -= QPoint (-view->selectionResizeHandleAtomicSize (),
                //                       -view->selectionResizeHandleAtomicSize ());
            }
        }

        pasteText (text, true/*force new text selection*/, selTopLeft);
    }
}


// private slot
void kpMainWindow::slotScrollViewAboutToScroll ()
{
#if DEBUG_KP_MAIN_WINDOW && 0
    kdDebug () << "kpMainWindow::slotScrollViewAboutToScroll() tool="
               << tool () << " viewManager=" << viewManager () << endl;
    if (viewManager ())
    {
        kdDebug () << "\tfastUpdates=" << viewManager ()->fastUpdates ()
                   << " queueUpdates=" << viewManager ()->queueUpdates ()
                   << endl;
    }
    else
    {
        // We're getting a late signal from the scrollview (thanks to
        // a timer inside the QScrollView).  By now, setDocument() has
        // already killed the document(), tool() and viewManager().
    }
#endif

    QTimer::singleShot (0, this, SLOT (slotScrollViewAfterScroll ()));
}

// private slot
void kpMainWindow::slotScrollViewAfterScroll ()
{
#if DEBUG_KP_MAIN_WINDOW && 0
    kdDebug () << "kpMainWindow::slotScrollViewAfterScroll() tool="
               << tool () << endl;
#endif

    if (tool ())
    {
        tool ()->somethingBelowTheCursorChanged ();
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
