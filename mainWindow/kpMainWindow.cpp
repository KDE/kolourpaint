
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

#include <q3dragobject.h>
#include <qevent.h>
#include <qpainter.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krecentfilesaction.h>
#include <kconfiggroup.h>

#include <kpAbstractImageSelection.h>
#include <kpCommandEnvironment.h>
#include <kpColorToolBar.h>
#include <kpCommandHistory.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpDocumentEnvironment.h>
#include <kpPixmapFX.h>
#include <kpSelectionDrag.h>
#include <kpThumbnail.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>
#include <kpWidgetMapper.h>
#include <kpZoomedThumbnailView.h>
#include <kpZoomedView.h>

#if DEBUG_KP_MAIN_WINDOW
    #include <qdatetime.h>
#endif


kpMainWindow::kpMainWindow ()
    : KXmlGuiWindow (0/*parent*/),
      m_isFullyConstructed (false)
{
    init ();
    open (KUrl (), true/*create an empty doc*/);

    m_isFullyConstructed = true;
}

kpMainWindow::kpMainWindow (const KUrl &url)
    : KXmlGuiWindow (0/*parent*/),
      m_isFullyConstructed (false)
{
    init ();
    open (url, true/*create an empty doc with the same url if url !exist*/);

    m_isFullyConstructed = true;
}

kpMainWindow::kpMainWindow (kpDocument *newDoc)
    : KXmlGuiWindow (0/*parent*/),
      m_isFullyConstructed (false)
{
    init ();
    setDocument (newDoc);

    m_isFullyConstructed = true;
}


// TODO: Move into appropriate kpMainWindow_*.cpp or another class

// private
void kpMainWindow::readGeneralSettings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tkpMainWindow(" << objectName () << ")::readGeneralSettings()" << endl;
#endif

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

    m_configFirstTime = cfg.readEntry (kpSettingFirstTime, true);
    m_configShowGrid = cfg.readEntry (kpSettingShowGrid, false);
    m_configShowPath = cfg.readEntry (kpSettingShowPath, false);
    d->m_moreEffectsDialogLastEffect = cfg.readEntry (kpSettingMoreEffectsLastEffect, 0);
    d->m_resizeScaleDialogLastKeepAspect = cfg.readEntry (kpSettingResizeScaleLastKeepAspect, false);

    if (cfg.hasKey (kpSettingOpenImagesInSameWindow))
    {
        d->configOpenImagesInSameWindow = cfg.readEntry (kpSettingOpenImagesInSameWindow, false);
    }
    else
    {
        d->configOpenImagesInSameWindow = false;
#if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tconfigOpenImagesInSameWindow: first time"
                  << " - writing default: " << d->configOpenImagesInSameWindow
                  << endl;
#endif
        // TODO: More hidden options have to write themselves out on startup,
        //       not on use, to be discoverable (e.g. printing centered on page).
        cfg.writeEntry (kpSettingOpenImagesInSameWindow,
                        d->configOpenImagesInSameWindow);
        cfg.sync ();
    }


#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\t\tGeneral Settings: firstTime=" << m_configFirstTime
               << " showGrid=" << m_configShowGrid
               << " showPath=" << m_configShowPath
               << " moreEffectsDialogLastEffect=" << d->m_moreEffectsDialogLastEffect
               << " resizeScaleDialogLastKeepAspect=" << d->m_resizeScaleDialogLastKeepAspect
               << " openImagesInSameWindow=" << d->configOpenImagesInSameWindow
               << endl;
#endif
}

// private
void kpMainWindow::readThumbnailSettings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tkpMainWindow(" << objectName () << ")::readThumbnailSettings()" << endl;
#endif

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupThumbnail);

    m_configThumbnailShown = cfg.readEntry (kpSettingThumbnailShown, false);
    m_configThumbnailGeometry = cfg.readEntry (kpSettingThumbnailGeometry, QRect ());
    m_configZoomedThumbnail = cfg.readEntry (kpSettingThumbnailZoomed, true);
    d->m_configThumbnailShowRectangle = cfg.readEntry (kpSettingThumbnailShowRectangle, true);

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\t\tThumbnail Settings: shown=" << m_configThumbnailShown
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
    kDebug () << "kpMainWindow(" << objectName () << ")::init()" << endl;
    QTime totalTime; totalTime.start ();
    QTime time; time.start ();
#endif

    d = new kpMainWindowPrivate;

    //
    // Set fields that must be set early on.
    //
    // For all other fields that don't need to be set as urgently, please
    // set them in their respective kpMainWindow_*.cpp.
    //

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
    m_settingImageSelectionTransparency = 0;
    m_settingTextStyle = 0;

    m_docResizeToBeCompleted = false;


    //
    // set mainwindow properties
    //

    setMinimumSize (320, 260);
    setAcceptDrops (true);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: little init = " << time.restart () << "msec" << endl;
#endif


    //
    // read config
    //

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    KGlobal::config ()->reparseConfiguration ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: reparseConfig = " << time.restart () << "msec" << endl;
#endif

    readGeneralSettings ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: readGeneralSettings = " << time.restart () << "msec" << endl;
#endif

    readThumbnailSettings ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: readThumbnailSettings = " << time.restart () << "msec" << endl;
#endif


    //
    // create GUI
    //

    setupActions ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: setupActions = " << time.restart () << "msec" << endl;
#endif

    createStatusBar ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: createStatusBar = " << time.restart () << "msec" << endl;
#endif

    createGUI ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: createGUI = " << time.restart () << "msec" << endl;
#endif


    //
    // Create more GUI.
    //

    m_colorToolBar = new kpColorToolBar (i18n ("Color Box"), this);
    m_colorToolBar->setObjectName ("Color Box");  // (needed for QMainWindow::saveState())
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: new kpColorToolBar = " << time.restart () << "msec" << endl;
#endif

    createToolBox ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tTIME: createToolBox = " << time.restart () << "msec" << endl;
#endif

    m_scrollView = new kpViewScrollableContainer (this);
    m_scrollView->setObjectName ("scrollView");
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
    kDebug () << "\tTIME: m_scrollView = " << time.restart () << "msec" << endl;
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
        kDebug () << "\tfirstTime: positioning toolbars" << endl;
    #endif

        addToolBar (Qt::LeftToolBarArea, m_toolToolBar);
        addToolBar (Qt::BottomToolBarArea, m_colorToolBar);

        KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

        cfg.writeEntry (kpSettingFirstTime, m_configFirstTime = false);
        cfg.sync ();
    }

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tall done in " << totalTime.elapsed () << "msec" << endl;
#endif
}


// private virtual [base KMainWindow]
void kpMainWindow::readProperties (const KConfigGroup &configGroup)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow<" << this << ">::readProperties()" << endl;
#endif

    // No document at all?
    if (!configGroup.hasKey (kpSessionSettingDocumentUrl))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tno url - no document" << endl;
    #endif
        setDocument (0);
    }
    // Have a document.
    else
    {
        const KUrl url = configGroup.readEntry (kpSessionSettingDocumentUrl,
                                                QString ());
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\turl=" << url << endl;
    #endif

        const QSize notFromURLDocSize =
            configGroup.readEntry (kpSessionSettingNotFromUrlDocumentSize,
                                   QSize ());

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
            kDebug () << "\tnot from url; doc size=" << notFromURLDocSize << endl;
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
void kpMainWindow::saveProperties (KConfigGroup &configGroup)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow<" << this << ">::saveProperties()" << endl;
#endif

    // No document at all?
    if (!m_document)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tno url - no document" << endl;
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

        const KUrl url = m_document->url ();
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\turl=" << url << endl;
    #endif
        configGroup.writeEntry (kpSessionSettingDocumentUrl, url.url ());

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
            kDebug () << "\tnot from url; doc size=" << docSize << endl;
        #endif
            configGroup.writeEntry (kpSessionSettingNotFromUrlDocumentSize, docSize);
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
            const KUrl tempURL = homeDir +
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
                configGroup.writeEntry (kpSessionSettingDocumentUnsavedContentsUrl,
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

    // Get the kpTool to finish up.  This makes sure that the kpTool destructor
    // will not need to access any other class (that might be deleted before
    // the destructor is called by the QObject child-deletion mechanism).
    if (tool ())
        tool ()->endInternal ();

    // Delete document & views.
    // Note: This will disconnects signals from the current kpTool, so kpTool
    //       must not be destructed yet.
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
kpDocumentEnvironment *kpMainWindow::documentEnvironment ()
{
    if (!d->documentEnvironment)
        d->documentEnvironment = new kpDocumentEnvironment (this);

    return d->documentEnvironment;
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
kpColorCells *kpMainWindow::colorCells () const
{
    return m_colorToolBar ? m_colorToolBar->colorCells () : 0;
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

kpCommandEnvironment *kpMainWindow::commandEnvironment ()
{
    if (!d->commandEnvironment)
        d->commandEnvironment = new kpCommandEnvironment (this);

    return d->commandEnvironment;
}


// private
void kpMainWindow::setupActions ()
{
    setupFileMenuActions ();
    setupEditMenuActions ();
    setupViewMenuActions ();
    setupImageMenuActions ();
    setupColorsMenuActions ();
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
    enableColorsMenuDocumentActions (enable);
    enableSettingsMenuDocumentActions (enable);
    enableHelpMenuDocumentActions (enable);
}


// private
void kpMainWindow::setDocument (kpDocument *newDoc)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::setDocument (" << newDoc << ")" << endl;
#endif

    // is it a close operation?
    if (!newDoc)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tdisabling actions" << endl;
    #endif

        // sync with the bit marked "sync" below

        Q_ASSERT (m_colorToolBar);
        m_colorToolBar->setEnabled (false);

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
    kDebug () << "\tdestroying views" << endl;
#endif

    delete m_mainView; m_mainView = 0;
    slotDestroyThumbnail ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdestroying viewManager" << endl;
#endif

    // viewManager will die and so will the selection
    m_actionCopy->setEnabled (false);
    m_actionCut->setEnabled (false);
    m_actionDelete->setEnabled (false);
    m_actionDeselect->setEnabled (false);
    m_actionCopyToFile->setEnabled (false);

    delete m_viewManager; m_viewManager = 0;

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdestroying document" << endl;
    kDebug () << "\t\tm_document=" << m_document << endl;
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
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\treparenting doc that may have been created into a"
                  << " different mainWindiow" << endl;
    #endif
        m_document->setEnviron (documentEnvironment ());

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () <<"\tcreating viewManager" << endl;
    #endif
        m_viewManager = new kpViewManager (this);

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tcreating views" << endl;
    #endif
        m_mainView = new kpZoomedView (m_document, m_toolToolBar, m_viewManager,
                                       0/*buddyView*/,
                                       m_scrollView,
                                       m_scrollView->viewport ());
        m_mainView->setObjectName ("mainView");

        m_scrollView->addChild (m_mainView);
        m_viewManager->registerView (m_mainView);
        m_mainView->show ();

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\thooking up document signals" << endl;
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
        Q_ASSERT (m_commandHistory);
        connect (m_commandHistory, SIGNAL (documentRestored ()),
                 this, SLOT (slotDocumentRestored ()));  // caption "!modified"
        connect (m_document, SIGNAL (documentSaved ()),
                 m_commandHistory, SLOT (documentSaved ()));

        // Sync document -> views
        connect (m_document, SIGNAL (contentsChanged (const QRect &)),
                 m_viewManager, SLOT (updateViews (const QRect &)));
        connect (m_document, SIGNAL (sizeChanged (int, int)),
                 m_viewManager, SLOT (adjustViewsToEnvironment ()));

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tenabling actions" << endl;
    #endif

        // sync with the bit marked "sync" above

        Q_ASSERT (m_colorToolBar);
        m_colorToolBar->setEnabled (true);


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
                kDebug () << "\tcreating thumbnail immediately" << endl;
            #endif
                slotCreateThumbnail ();
            }
            // this' geometry is weird ATM
            else
            {
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\tcreating thumbnail LATER" << endl;
            #endif
                QTimer::singleShot (0, this, SLOT (slotCreateThumbnail ()));
            }
        }
    #endif
    }

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tupdating mainWindow elements" << endl;
#endif

    slotImageMenuUpdateDueToSelection ();
    recalculateStatusBar ();
    slotUpdateCaption ();  // Untitled to start with
    slotEnableReload ();
    slotEnableSettingsShowPath ();

    if (m_commandHistory)
        m_commandHistory->clear ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdocument and views ready to go!" << endl;
#endif
}


// private virtual [base KMainWindow]
bool kpMainWindow::queryClose ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::queryClose()" << endl;
#endif
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (!m_document || !m_document->isModified ())
        return true;  // ok to close current doc

    int result = KMessageBox::warningYesNoCancel (this,
                     i18n ("The document \"%1\" has been modified.\n"
                           "Do you want to save it?",
                           m_document->prettyFilename ()),
                    QString::null/*caption*/,
                    KStandardGuiItem::save (), KStandardGuiItem::discard ());

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
    e->setAccepted (kpSelectionDrag::canDecode (e) ||
                    KUrl::List::canDecode (e->mimeData ()) ||
                    Q3TextDrag::canDecode (e));
}

// private virtual [base QWidget]
void kpMainWindow::dropEvent (QDropEvent *e)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::dropEvent" << e->pos () << endl;
#endif

    KUrl::List urls;
    QString text;

    kpAbstractImageSelection *sel = kpSelectionDrag::decode (e,
        pasteWarnAboutLossInfo ());
    if (sel)
    {
        sel->setTransparency (imageSelectionTransparency ());
        // TODO: drop at point like with QTextDrag below?
        paste (*sel);
        delete sel;
    }
    else if (!(urls = KUrl::List::fromMimeData (e->mimeData ())).isEmpty ())
    {
        foreach (KUrl u, urls)
            open (u);
    }
    else if (Q3TextDrag::decode (e, text/*ref*/))
    {
        QPoint selTopLeft = KP_INVALID_POINT;
        const QPoint globalPos = QWidget::mapToGlobal (e->pos ());
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tpos toGlobal=" << globalPos << endl;
    #endif

        kpView *view = 0;

        if (m_viewManager)
        {
            view = m_viewManager->viewUnderCursor ();
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\t\tviewUnderCursor=" << view << endl;
        #endif
            if (!view)
            {
                // HACK: see kpViewManager::setViewUnderCursor() to see why
                //       it's not reliable
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\t\tattempting to discover view" << endl;

                if (m_mainView && m_scrollView)
                {
                    kDebug () << "\t\t\tmainView->globalRect="
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
    kDebug () << "kpMainWindow::slotScrollViewAboutToScroll()" << endl;
    kDebug () << "\tfastUpdates=" << viewManager ()->fastUpdates ()
               << " queueUpdates=" << viewManager ()->queueUpdates ()
               << endl;
#endif

    QTimer::singleShot (0, this, SLOT (slotScrollViewAfterScroll ()));
}

// private slot
void kpMainWindow::slotScrollViewAfterScroll ()
{
#if DEBUG_KP_MAIN_WINDOW && 0
    kDebug () << "kpMainWindow::slotScrollViewAfterScroll() tool="
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
        setCaption (m_configShowPath ? m_document->prettyUrl ()
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


#include <kpMainWindow.moc>
