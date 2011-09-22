
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

#include <qevent.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <KMenu>
#include <KMenuBar>
#include <krecentfilesaction.h>

#include <kpAbstractImageSelection.h>
#include <kpCommandEnvironment.h>
#include <kpColorCells.h>
#include <kpColorToolBar.h>
#include <kpCommandHistory.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpDocumentEnvironment.h>
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


//---------------------------------------------------------------------

kpMainWindow::kpMainWindow ()
    : KXmlGuiWindow (0/*parent*/)
{
    init ();
    open (KUrl (), true/*create an empty doc*/);

    d->isFullyConstructed = true;
}

//---------------------------------------------------------------------

kpMainWindow::kpMainWindow (const KUrl &url)
    : KXmlGuiWindow (0/*parent*/)
{
    init ();
    open (url, true/*create an empty doc with the same url if url !exist*/);

    d->isFullyConstructed = true;
}

//---------------------------------------------------------------------

kpMainWindow::kpMainWindow (kpDocument *newDoc)
    : KXmlGuiWindow (0/*parent*/)
{
    init ();
    setDocument (newDoc);

    d->isFullyConstructed = true;
}

//---------------------------------------------------------------------


// TODO: Move into appropriate kpMainWindow_*.cpp or another class

// private
void kpMainWindow::readGeneralSettings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tkpMainWindow(" << objectName () << ")::readGeneralSettings()";
#endif

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

    d->configFirstTime = cfg.readEntry (kpSettingFirstTime, true);
    d->configShowGrid = cfg.readEntry (kpSettingShowGrid, false);
    d->configShowPath = cfg.readEntry (kpSettingShowPath, false);
    d->moreEffectsDialogLastEffect = cfg.readEntry (kpSettingMoreEffectsLastEffect, 0);

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

    d->configPrintImageCenteredOnPage = cfg.readEntry (kpSettingPrintImageCenteredOnPage, true);


#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\t\tGeneral Settings: firstTime=" << d->configFirstTime
               << " showGrid=" << d->configShowGrid
               << " showPath=" << d->configShowPath
               << " moreEffectsDialogLastEffect=" << d->moreEffectsDialogLastEffect
               << " openImagesInSameWindow=" << d->configOpenImagesInSameWindow
               << " printImageCenteredOnPage=" << d->configPrintImageCenteredOnPage;
#endif
}

//---------------------------------------------------------------------

// private
void kpMainWindow::readThumbnailSettings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tkpMainWindow(" << objectName () << ")::readThumbnailSettings()";
#endif

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupThumbnail);

    d->configThumbnailShown = cfg.readEntry (kpSettingThumbnailShown, false);
    d->configThumbnailGeometry = cfg.readEntry (kpSettingThumbnailGeometry, QRect ());
    d->configZoomedThumbnail = cfg.readEntry (kpSettingThumbnailZoomed, true);
    d->configThumbnailShowRectangle = cfg.readEntry (kpSettingThumbnailShowRectangle, true);

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\t\tThumbnail Settings: shown=" << d->configThumbnailShown
               << " geometry=" << d->configThumbnailGeometry
               << " zoomed=" << d->configZoomedThumbnail
               << " showRectangle=" << d->configThumbnailShowRectangle
               << endl;
#endif
}

//---------------------------------------------------------------------

void kpMainWindow::finalizeGUI(KXMLGUIClient *client)
{
  if ( client == this )
  {
    const QList<KMenu *> menuToHide = findChildren<KMenu *>("toolToolBarHiddenMenu");
    // should only contain one but...
    foreach (KMenu *menu, menuToHide)
    {
        menu->menuAction()->setVisible(false);
    }
  }
}

//---------------------------------------------------------------------

// private
void kpMainWindow::init ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow(" << objectName () << ")::init()";
    QTime totalTime; totalTime.start ();
#endif

    d = new kpMainWindowPrivate;

    //
    // Set fields that must be set early on.
    //
    // For all other fields that don't need to be set as urgently, please
    // set them in their respective kpMainWindow_*.cpp.
    //

    d->isFullyConstructed = false;

    d->scrollView = 0;
    d->mainView = 0;
    d->thumbnail = 0;
    d->thumbnailView = 0;
    d->document = 0;
    d->viewManager = 0;
    d->colorToolBar = 0;
    d->toolToolBar = 0;
    d->commandHistory = 0;
    d->statusBarCreated = false;
    d->settingImageSelectionTransparency = 0;
    d->settingTextStyle = 0;

    d->docResizeToBeCompleted = false;

    d->documentEnvironment = 0;
    d->commandEnvironment = 0;
    d->toolSelectionEnvironment = 0;
    d->toolsActionGroup = 0;
    d->transformDialogEnvironment = 0;


    //
    // set mainwindow properties
    //

    setMinimumSize (320, 260);
    setAcceptDrops (true);

    //
    // read config
    //

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    KGlobal::config ()->reparseConfiguration ();

    readGeneralSettings ();
    readThumbnailSettings ();

    //
    // create GUI
    //
    setupActions ();
    createStatusBar ();
    createGUI ();

    createColorBox ();
    createToolBox ();


    // Let the Tool Box take all the vertical space, since it can be quite
    // tall with all its tool option widgets.  This also avoids occasional
    // bugs like the Tool Box overlapping the Color Tool Bar.
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // no tabbed docks; does not make sense with only 2 dock widgets
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);

    addDockWidget(Qt::BottomDockWidgetArea, d->colorToolBar, Qt::Horizontal);

    d->scrollView = new kpViewScrollableContainer (this);
    d->scrollView->setObjectName ( QLatin1String("scrollView" ));
    connect (d->scrollView, SIGNAL (beganDocResize ()),
             this, SLOT (slotBeganDocResize ()));
    connect (d->scrollView, SIGNAL (continuedDocResize (const QSize &)),
             this, SLOT (slotContinuedDocResize (const QSize &)));
    connect (d->scrollView, SIGNAL (cancelledDocResize ()),
             this, SLOT (slotCancelledDocResize ()));
    connect (d->scrollView, SIGNAL (endedDocResize (const QSize &)),
             this, SLOT (slotEndedDocResize (const QSize &)));

    connect (d->scrollView, SIGNAL (statusMessageChanged (const QString &)),
             this, SLOT (slotDocResizeMessageChanged (const QString &)));

    connect (d->scrollView, SIGNAL(contentsMoved()),
             this, SLOT(slotScrollViewAfterScroll()));

    setCentralWidget (d->scrollView);

    //
    // set initial pos/size of GUI
    //

    setAutoSaveSettings ();

    // our non-XMLGUI tools-toolbar will get initially the toolButtonStyle as
    // all other toolbars, but we want to show only icons for the tools by default
    // (have to do this _after_ setAutoSaveSettings as that applies the default settings)
    if (d->configFirstTime)
    {
      d->toolToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

      KConfigGroup cfg(KGlobal::config(), kpSettingsGroupGeneral);

      cfg.writeEntry(kpSettingFirstTime, d->configFirstTime = false);
      cfg.sync();
    }


#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tall done in " << totalTime.elapsed () << "msec";
#endif
}

//---------------------------------------------------------------------

// private virtual [base KMainWindow]
void kpMainWindow::readProperties (const KConfigGroup &configGroup)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow<" << this << ">::readProperties()";
#endif

    // No document at all?
    if (!configGroup.hasKey (kpSessionSettingDocumentUrl))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tno url - no document";
    #endif
        setDocument (0);
    }
    // Have a document.
    else
    {
        const KUrl url = configGroup.readEntry (kpSessionSettingDocumentUrl,
                                                QString ());
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\turl=" << url;
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
            kDebug () << "\tnot from url; doc size=" << notFromURLDocSize;
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

//---------------------------------------------------------------------

// private virtual [base KMainWindow]
// WARNING: KMainWindow API Doc says "No user interaction is allowed
//          in this function!"
void kpMainWindow::saveProperties (KConfigGroup &configGroup)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow<" << this << ">::saveProperties()";
#endif

    // No document at all?
    if (!d->document)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tno url - no document";
    #endif
    }
    // Have a document.
    else
    {
        // Save URL in all cases:
        //
        //    a) d->document->isFromURL()
        //    b) !d->document->isFromURL() [save size in this case]
        //       i) No URL
        //       ii) URL (from "kolourpaint doesnotexist.png")

        const KUrl url = d->document->url ();
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\turl=" << url;
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
        if (!d->document->isFromURL (false/*don't bother checking exists*/))
        {
            // If we don't have a URL either:
            //
            // a) it was not modified - so we can use either width() or
            //    constructorWidth() (they'll be equal).
            // b) the changes were discarded so we use the initial width,
            //    constructorWidth().
            //
            // Similarly for height() and constructorHeight().
            const QSize docSize (d->document->constructorWidth (),
                                 d->document->constructorHeight ());
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tnot from url; doc size=" << docSize;
        #endif
            configGroup.writeEntry (kpSessionSettingNotFromUrlDocumentSize, docSize);
        }


        // Local session save i.e. queryClose() was not called beforehand
        // (see QApplication::saveState())?
    #if 0
        if (d->document->isModified ())
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

            if (kpDocument::savePixmapToFile (d->document->pixmapWithSelection (),
                    tempURL,
                    pngSaveOptions, *d->document->metaInfo (),
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

//---------------------------------------------------------------------


kpMainWindow::~kpMainWindow ()
{
    d->isFullyConstructed = false;

    // Get the kpTool to finish up.  This makes sure that the kpTool destructor
    // will not need to access any other class (that might be deleted before
    // the destructor is called by the QObject child-deletion mechanism).
    if (tool ())
        tool ()->endInternal ();

    // Delete document & views.
    // Note: This will disconnects signals from the current kpTool, so kpTool
    //       must not be destructed yet.
    setDocument (0);

    delete d->commandHistory; d->commandHistory = 0;
    delete d->scrollView; d->scrollView = 0;

    delete d; d = 0;
}

//---------------------------------------------------------------------


// public
kpDocument *kpMainWindow::document () const
{
    return d->document;
}

//---------------------------------------------------------------------

// public
kpDocumentEnvironment *kpMainWindow::documentEnvironment ()
{
    if (!d->documentEnvironment)
        d->documentEnvironment = new kpDocumentEnvironment (this);

    return d->documentEnvironment;
}

//---------------------------------------------------------------------

// public
kpViewManager *kpMainWindow::viewManager () const
{
    return d->viewManager;
}

//---------------------------------------------------------------------

// public
kpColorToolBar *kpMainWindow::colorToolBar () const
{
    return d->colorToolBar;
}

//---------------------------------------------------------------------

// public
kpColorCells *kpMainWindow::colorCells () const
{
    return d->colorToolBar ? d->colorToolBar->colorCells () : 0;
}

//---------------------------------------------------------------------

// public
kpToolToolBar *kpMainWindow::toolToolBar () const
{
    return d->toolToolBar;
}

//---------------------------------------------------------------------

// public
kpCommandHistory *kpMainWindow::commandHistory () const
{
    return d->commandHistory;
}

//---------------------------------------------------------------------

kpCommandEnvironment *kpMainWindow::commandEnvironment ()
{
    if (!d->commandEnvironment)
        d->commandEnvironment = new kpCommandEnvironment (this);

    return d->commandEnvironment;
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setupActions ()
{
    setupFileMenuActions ();
    setupEditMenuActions ();
    setupViewMenuActions ();
    setupImageMenuActions ();
    setupColorsMenuActions ();
    setupSettingsMenuActions ();

    setupTextToolBarActions ();
    setupToolActions ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableDocumentActions (bool enable)
{
    enableFileMenuDocumentActions (enable);
    enableEditMenuDocumentActions (enable);
    enableViewMenuDocumentActions (enable);
    enableImageMenuDocumentActions (enable);
    enableColorsMenuDocumentActions (enable);
    enableSettingsMenuDocumentActions (enable);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setDocument (kpDocument *newDoc)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::setDocument (" << newDoc << ")";
#endif

    // is it a close operation?
    if (!newDoc)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tdisabling actions";
    #endif

        // sync with the bit marked "sync" below

        // TODO: Never disable the Color Box because the user should be
        //       able to manipulate the colors, even without a currently
        //       open document.
        //
        //       We just have to make sure that signals from the Color
        //       Box aren't fired and received unexpectedly when there's
        //       no document.
        Q_ASSERT (d->colorToolBar);
        d->colorToolBar->setEnabled (false);

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
    kDebug () << "\tdestroying views";
#endif

    delete d->mainView; d->mainView = 0;
    slotDestroyThumbnail ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdestroying viewManager";
#endif

    // viewManager will die and so will the selection
    d->actionCopy->setEnabled (false);
    d->actionCut->setEnabled (false);
    d->actionDelete->setEnabled (false);
    d->actionDeselect->setEnabled (false);
    d->actionCopyToFile->setEnabled (false);

    delete d->viewManager; d->viewManager = 0;

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdestroying document";
    kDebug () << "\t\td->document=" << d->document;
#endif
    // destroy current document
    delete d->document;
    d->document = newDoc;


    if (!d->lastCopyToURL.isEmpty ())
        d->lastCopyToURL.setFileName (QString());
    d->copyToFirstTime = true;

    if (!d->lastExportURL.isEmpty ())
        d->lastExportURL.setFileName (QString());
    d->exportFirstTime = true;


    // not a close operation?
    if (d->document)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\treparenting doc that may have been created into a"
                  << " different mainWindiow" << endl;
    #endif
        d->document->setEnviron (documentEnvironment ());

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () <<"\tcreating viewManager";
    #endif
        d->viewManager = new kpViewManager (this);

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tcreating views";
    #endif
        d->mainView = new kpZoomedView (d->document, d->toolToolBar, d->viewManager,
                                       0/*buddyView*/,
                                       d->scrollView,
                                       d->scrollView->viewport ());
        d->mainView->setObjectName ( QLatin1String("mainView" ));

        d->scrollView->setView (d->mainView);
        d->viewManager->registerView (d->mainView);
        d->mainView->show ();

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\thooking up document signals";
    #endif

        // Copy/Cut/Deselect/Delete
        connect (d->document, SIGNAL (selectionEnabled (bool)),
                 d->actionCut, SLOT (setEnabled (bool)));
        connect (d->document, SIGNAL (selectionEnabled (bool)),
                 d->actionCopy, SLOT (setEnabled (bool)));
        connect (d->document, SIGNAL (selectionEnabled (bool)),
                 d->actionDelete, SLOT (setEnabled (bool)));
        connect (d->document, SIGNAL (selectionEnabled (bool)),
                 d->actionDeselect, SLOT (setEnabled (bool)));
        connect (d->document, SIGNAL (selectionEnabled (bool)),
                 d->actionCopyToFile, SLOT (setEnabled (bool)));

        // this code won't actually enable any actions at this stage
        // (fresh document) but better safe than sorry
        d->actionCopy->setEnabled (d->document->selection ());
        d->actionCut->setEnabled (d->document->selection ());
        d->actionDeselect->setEnabled (d->document->selection ());
        d->actionDelete->setEnabled (d->document->selection ());
        d->actionCopyToFile->setEnabled (d->document->selection ());

        connect (d->document, SIGNAL (selectionEnabled (bool)),
                 this, SLOT (slotImageMenuUpdateDueToSelection ()));
        connect (d->document, SIGNAL (selectionIsTextChanged (bool)),
                 this, SLOT (slotImageMenuUpdateDueToSelection ()));

        // Status bar
        connect (d->document, SIGNAL (documentOpened ()),
                 this, SLOT (recalculateStatusBar ()));

        connect (d->document, SIGNAL (sizeChanged (const QSize &)),
                 this, SLOT (setStatusBarDocSize (const QSize &)));

        // Caption (url, modified)
        connect (d->document, SIGNAL (documentModified ()),
                 this, SLOT (slotUpdateCaption ()));
        connect (d->document, SIGNAL (documentOpened ()),
                 this, SLOT (slotUpdateCaption ()));
        connect (d->document, SIGNAL (documentSaved ()),
                 this, SLOT (slotUpdateCaption ()));

        // File/Reload action only available with non-empty URL
        connect (d->document, SIGNAL (documentSaved ()),
                 this, SLOT (slotEnableReload ()));

        connect (d->document, SIGNAL (documentSaved ()),
                 this, SLOT (slotEnableSettingsShowPath ()));

        // Command history
        Q_ASSERT (d->commandHistory);
        connect (d->commandHistory, SIGNAL (documentRestored ()),
                 this, SLOT (slotDocumentRestored ()));  // caption "!modified"
        connect (d->document, SIGNAL (documentSaved ()),
                 d->commandHistory, SLOT (documentSaved ()));

        // Sync document -> views
        connect (d->document, SIGNAL (contentsChanged (const QRect &)),
                 d->viewManager, SLOT (updateViews (const QRect &)));
        connect (d->document, SIGNAL (sizeChanged (int, int)),
                 d->viewManager, SLOT (adjustViewsToEnvironment ()));

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tenabling actions";
    #endif

        // sync with the bit marked "sync" above

        Q_ASSERT (d->colorToolBar);
        d->colorToolBar->setEnabled (true);


        // Hide the text toolbar - it will be shown by kpToolText::begin()
        enableTextToolBarActions (false);

        enableToolsDocumentActions (true);

        enableDocumentActions (true);

    // TODO: The thumbnail auto zoom doesn't work because it thinks its
    //       width == 1 when !this->isShown().  So for consistency,
    //       never create the thumbnail.
    #if 0
        if (d->configThumbnailShown)
        {
            if (isShown ())
            {
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\tcreating thumbnail immediately";
            #endif
                slotCreateThumbnail ();
            }
            // this' geometry is weird ATM
            else
            {
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\tcreating thumbnail LATER";
            #endif
                QTimer::singleShot (0, this, SLOT (slotCreateThumbnail ()));
            }
        }
    #endif
    }

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tupdating mainWindow elements";
#endif

    slotImageMenuUpdateDueToSelection ();
    recalculateStatusBar ();
    slotUpdateCaption ();  // Untitled to start with
    slotEnableReload ();
    slotEnableSettingsShowPath ();

    if (d->commandHistory)
        d->commandHistory->clear ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tdocument and views ready to go!";
#endif
}

//---------------------------------------------------------------------

// private virtual [base QWidget]
void kpMainWindow::dragEnterEvent (QDragEnterEvent *e)
{
    // It's faster to test for QMimeData::hasText() first due to the
    // lazy evaluation of the '||' operator.
    e->setAccepted (e->mimeData ()->hasText () ||
                    KUrl::List::canDecode (e->mimeData ()) ||
                    kpSelectionDrag::canDecode (e->mimeData ()));
}

//---------------------------------------------------------------------

// private virtual [base QWidget]
void kpMainWindow::dropEvent (QDropEvent *e)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::dropEvent" << e->pos ();
#endif

    KUrl::List urls;

    kpAbstractImageSelection *sel = kpSelectionDrag::decode (e->mimeData ());
    if (sel)
    {
        // TODO: How do you actually drop a selection or ordinary images on
        //       the clipboard)?  Will this code path _ever_ execute?
        sel->setTransparency (imageSelectionTransparency ());
        // TODO: drop at point like with QTextDrag below?
        paste (*sel);
        delete sel;
    }
    else if (!(urls = KUrl::List::fromMimeData (e->mimeData ())).isEmpty ())
    {
        // LOTODO: kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);
        //
        //         However, you would need to prefix all possible error/warning
        //         dialogs that might be called, with Qt::arrowCursor e.g. in
        //         kpDocument  and probably a lot more places.
        foreach (const KUrl &u, urls)
            open (u);
    }
    else if (e->mimeData ()->hasText ())
    {
        const QString text = e->mimeData ()->text ();

        QPoint selTopLeft = KP_INVALID_POINT;
        const QPoint globalPos = QWidget::mapToGlobal (e->pos ());
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tpos toGlobal=" << globalPos;
    #endif

        kpView *view = 0;

        if (d->viewManager)
        {
            view = d->viewManager->viewUnderCursor ();
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\t\tviewUnderCursor=" << view;
        #endif
            if (!view)
            {
                // HACK: see kpViewManager::setViewUnderCursor() to see why
                //       it's not reliable
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\t\tattempting to discover view";

                if (d->mainView && d->scrollView)
                {
                    kDebug () << "\t\t\tmainView->globalRect="
                            << kpWidgetMapper::toGlobal (d->mainView, d->mainView->rect ())
                            << " scrollView->globalRect="
                            << kpWidgetMapper::toGlobal (d->scrollView,
                                    QRect (0, 0,
                                            d->scrollView->viewport()->width (),
                                            d->scrollView->viewport()->height ()))
                            << endl;
                }
            #endif
                if (d->thumbnailView &&
                    kpWidgetMapper::toGlobal (d->thumbnailView, d->thumbnailView->rect ())
                        .contains (globalPos))
                {
                    // TODO: Code will never get executed.
                    //       Thumbnail doesn't accept drops.
                    view = d->thumbnailView;
                }
                else if (d->mainView &&
                         kpWidgetMapper::toGlobal (d->mainView, d->mainView->rect ())
                             .contains (globalPos) &&
                         d->scrollView &&
                         kpWidgetMapper::toGlobal (d->scrollView,
                             QRect (0, 0,
                                    d->scrollView->viewport()->width (),
                                    d->scrollView->viewport()->height ()))
                             .contains (globalPos))
                {
                    view = d->mainView;
                }
            }
        }

        if (view)
        {
            const QPoint viewPos = view->mapFromGlobal (globalPos);
            const QPoint docPoint = view->transformViewToDoc (viewPos);

            // viewUnderCursor() is hacky and can return a view when we aren't
            // over one thanks to drags.
            if (d->document && d->document->rect ().contains (docPoint))
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

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotScrollViewAfterScroll ()
{
    // OPT: Why can't this be moved into kpViewScrollableContainer::slotDragScroll(),
    //      grouping all drag-scroll-related repaints, which would potentially avoid
    //      double repainting?
    if (tool ())
    {
        tool ()->somethingBelowTheCursorChanged ();
    }
}

//---------------------------------------------------------------------

// private virtual [base QWidget]
void kpMainWindow::moveEvent (QMoveEvent * /*e*/)
{
    if (d->thumbnail)
    {
        // Disabled because it lags too far behind the mainWindow
        // d->thumbnail->move (d->thumbnail->pos () + (e->pos () - e->oldPos ()));

        notifyThumbnailGeometryChanged ();
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotUpdateCaption ()
{
    if (d->document)
    {
        setCaption (d->configShowPath ? d->document->prettyUrl ()
                                     : d->document->prettyFilename (),
                    d->document->isModified ());
    }
    else
    {
        setCaption (QString(), false);
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotDocumentRestored ()
{
    if (d->document)
        d->document->setModified (false);
    slotUpdateCaption ();
}

//---------------------------------------------------------------------

#include <kpMainWindow.moc>
