
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


#include "kpMainWindow.h"
#include "kpMainWindowPrivate.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "environments/tools/kpToolEnvironment.h"
#include "widgets/kpColorCells.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "environments/document/kpDocumentEnvironment.h"
#include "layers/selections/kpSelectionDrag.h"
#include "kpThumbnail.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "views/manager/kpViewManager.h"
#include "kpViewScrollableContainer.h"
#include "generic/kpWidgetMapper.h"
#include "views/kpZoomedThumbnailView.h"
#include "views/kpZoomedView.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QEvent>
#include <QMenu>
#include <QTimer>
#include <QDropEvent>

#include "kpLogCategories.h"


//---------------------------------------------------------------------

kpMainWindow::kpMainWindow ()
    : KXmlGuiWindow (nullptr/*parent*/)
{
    init ();
    open (QUrl (), true/*create an empty doc*/);

    d->isFullyConstructed = true;
}

//---------------------------------------------------------------------

kpMainWindow::kpMainWindow (const QUrl &url)
    : KXmlGuiWindow (nullptr/*parent*/)
{
    init ();
    open (url, true/*create an empty doc with the same url if url !exist*/);

    d->isFullyConstructed = true;
}

//---------------------------------------------------------------------

kpMainWindow::kpMainWindow (kpDocument *newDoc)
    : KXmlGuiWindow (nullptr/*parent*/)
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
    qCDebug(kpLogMainWindow) << "\tkpMainWindow(" << objectName () << ")::readGeneralSettings()";
#endif

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);

    d->configFirstTime = cfg.readEntry (kpSettingFirstTime, true);
    d->configShowGrid = cfg.readEntry (kpSettingShowGrid, false);
    d->configShowPath = cfg.readEntry (kpSettingShowPath, false);
    d->moreEffectsDialogLastEffect = cfg.readEntry (kpSettingMoreEffectsLastEffect, 0);
    kpToolEnvironment::drawAntiAliased = cfg.readEntry(kpSettingDrawAntiAliased, true);

    if (cfg.hasKey (kpSettingOpenImagesInSameWindow))
    {
        d->configOpenImagesInSameWindow = cfg.readEntry (kpSettingOpenImagesInSameWindow, false);
    }
    else
    {
        d->configOpenImagesInSameWindow = false;
#if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tconfigOpenImagesInSameWindow: first time"
                  << " - writing default: " << d->configOpenImagesInSameWindow;
#endif
        // TODO: More hidden options have to write themselves out on startup,
        //       not on use, to be discoverable (e.g. printing centered on page).
        cfg.writeEntry (kpSettingOpenImagesInSameWindow,
                        d->configOpenImagesInSameWindow);
        cfg.sync ();
    }

    d->configPrintImageCenteredOnPage = cfg.readEntry (kpSettingPrintImageCenteredOnPage, true);


#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\t\tGeneral Settings: firstTime=" << d->configFirstTime
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
    qCDebug(kpLogMainWindow) << "\tkpMainWindow(" << objectName () << ")::readThumbnailSettings()";
#endif

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupThumbnail);

    d->configThumbnailShown = cfg.readEntry (kpSettingThumbnailShown, false);
    d->configThumbnailGeometry = cfg.readEntry (kpSettingThumbnailGeometry, QRect ());
    d->configZoomedThumbnail = cfg.readEntry (kpSettingThumbnailZoomed, true);
    d->configThumbnailShowRectangle = cfg.readEntry (kpSettingThumbnailShowRectangle, true);

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\t\tThumbnail Settings: shown=" << d->configThumbnailShown
               << " geometry=" << d->configThumbnailGeometry
               << " zoomed=" << d->configZoomedThumbnail
               << " showRectangle=" << d->configThumbnailShowRectangle;
#endif
}

//---------------------------------------------------------------------

void kpMainWindow::finalizeGUI(KXMLGUIClient *client)
{
  if ( client == this )
  {
    const QList<QMenu *> menuToHide = findChildren<QMenu *>(QStringLiteral("toolToolBarHiddenMenu"));
    // should only contain one but...
    for (auto *menu : menuToHide)
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
    qCDebug(kpLogMainWindow) << "kpMainWindow(" << objectName () << ")::init()";
    QTime totalTime; totalTime.start ();
#endif

    d = new kpMainWindowPrivate;

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
    KSharedConfig::openConfig ()->reparseConfiguration ();

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
    d->scrollView->setObjectName ( QStringLiteral("scrollView" ));

    connect (d->scrollView, &kpViewScrollableContainer::beganDocResize,
             this, &kpMainWindow::slotBeganDocResize);

    connect (d->scrollView, &kpViewScrollableContainer::continuedDocResize,
             this, &kpMainWindow::slotContinuedDocResize);

    connect (d->scrollView, &kpViewScrollableContainer::cancelledDocResize,
             this, &kpMainWindow::slotCancelledDocResize);

    connect (d->scrollView, &kpViewScrollableContainer::endedDocResize,
             this, &kpMainWindow::slotEndedDocResize);

    connect (d->scrollView, &kpViewScrollableContainer::statusMessageChanged,
             this, &kpMainWindow::slotDocResizeMessageChanged);

    connect (d->scrollView, &kpViewScrollableContainer::contentsMoved,
             this, &kpMainWindow::slotScrollViewAfterScroll);

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

      KConfigGroup cfg(KSharedConfig::openConfig(), kpSettingsGroupGeneral);

      cfg.writeEntry(kpSettingFirstTime, d->configFirstTime = false);
      cfg.sync();
    }


#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tall done in " << totalTime.elapsed () << "msec";
#endif
}

//---------------------------------------------------------------------

// private virtual [base KMainWindow]
void kpMainWindow::readProperties (const KConfigGroup &configGroup)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow<" << this << ">::readProperties()";
#endif

    // No document at all?
    if (!configGroup.hasKey (kpSessionSettingDocumentUrl))
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tno url - no document";
    #endif
        setDocument (nullptr);
    }
    // Have a document.
    else
    {
        const QUrl url = QUrl (configGroup.readEntry (kpSessionSettingDocumentUrl,
                                                QString ()));
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\turl=" << url;
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
            qCDebug(kpLogMainWindow) << "\tnot from url; doc size=" << notFromURLDocSize;
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
    qCDebug(kpLogMainWindow) << "kpMainWindow<" << this << ">::saveProperties()";
#endif

    // No document at all?
    if (!d->document)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tno url - no document";
    #endif
    }
    // Have a document.
    else
    {
        // Save URL in all cases:
        //
        //    a) d->document->isFromExistingURL()
        //    b) !d->document->isFromExistingURL() [save size in this case]
        //       i) No URL
        //       ii) URL (from "kolourpaint doesnotexist.png")

        const QUrl url = d->document->url ();
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\turl=" << url;
    #endif
        configGroup.writeEntry (kpSessionSettingDocumentUrl, url.url ());

        // Not from URL e.g. "kolourpaint doesnotexist.png"?
        //
        // Note that "kolourpaint doesexist.png" is considered to be from
        // a URL even if it was deleted in the background. This is because the user expects
        // it to be from a URL, so when we session restore, we pop up a
        // "cannot find file" dialog, instead of silently creating a new,
        // blank document.
        if (!d->document->isFromExistingURL ())
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
            qCDebug(kpLogMainWindow) << "\tnot from url; doc size=" << docSize;
        #endif
            configGroup.writeEntry (kpSessionSettingNotFromUrlDocumentSize, docSize);
        }
    }
}

//---------------------------------------------------------------------


kpMainWindow::~kpMainWindow ()
{
    d->isFullyConstructed = false;

    // Get the kpTool to finish up.  This makes sure that the kpTool destructor
    // will not need to access any other class (that might be deleted before
    // the destructor is called by the QObject child-deletion mechanism).
    if (tool ()) {
        tool ()->endInternal ();
    }

    // Delete document & views.
    // Note: This will disconnects signals from the current kpTool, so kpTool
    //       must not be destructed yet.
    setDocument (nullptr);

    delete d->commandHistory; d->commandHistory = nullptr;
    delete d->scrollView; d->scrollView = nullptr;

    delete d; d = nullptr;
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
    if (!d->documentEnvironment) {
        d->documentEnvironment = new kpDocumentEnvironment (this);
    }

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
    return d->colorToolBar ? d->colorToolBar->colorCells () : nullptr;
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
    if (!d->commandEnvironment) {
        d->commandEnvironment = new kpCommandEnvironment (this);
    }

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
    //qCDebug(kpLogMainWindow) << newDoc;

    // is it a close operation?
    if (!newDoc)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tdisabling actions";
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

    delete d->mainView; d->mainView = nullptr;
    slotDestroyThumbnail ();

    // viewManager will die and so will the selection
    d->actionCopy->setEnabled (false);
    d->actionCut->setEnabled (false);
    d->actionDelete->setEnabled (false);
    d->actionDeselect->setEnabled (false);
    d->actionCopyToFile->setEnabled (false);

    delete d->viewManager; d->viewManager = nullptr;

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tdestroying document";
    qCDebug(kpLogMainWindow) << "\t\td->document=" << d->document;
#endif
    // destroy current document
    delete d->document;
    d->document = newDoc;


    if (!d->lastCopyToURL.isEmpty ())
    {
        // remove file name from path
        QString path = d->lastCopyToURL.path ();
        path = path.left (path.lastIndexOf (QLatin1Char ('/')) + 1);
        d->lastCopyToURL.setPath (path);
    }
    d->copyToFirstTime = true;

    if (!d->lastExportURL.isEmpty ())
    {
        QString path = d->lastExportURL.path ();
        path = path.left (path.lastIndexOf (QLatin1Char ('/')) + 1);
        d->lastExportURL.setPath (path);
    }
    d->exportFirstTime = true;


    // not a close operation?
    if (d->document)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\treparenting doc that may have been created into a"
                  << " different mainWindiow";
    #endif
        d->document->setEnviron (documentEnvironment ());

        d->viewManager = new kpViewManager (this);

        d->mainView = new kpZoomedView (d->document, d->toolToolBar, d->viewManager,
                                       nullptr/*buddyView*/,
                                       d->scrollView,
                                       d->scrollView->viewport ());
        d->mainView->setObjectName ( QStringLiteral("mainView" ));

        d->viewManager->registerView (d->mainView);
        d->scrollView->setView (d->mainView);
        d->mainView->show ();

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\thooking up document signals";
    #endif

        // Copy/Cut/Deselect/Delete
        connect (d->document, &kpDocument::selectionEnabled,
                 d->actionCut, &QAction::setEnabled);

        connect (d->document, &kpDocument::selectionEnabled,
                 d->actionCopy, &QAction::setEnabled);

        connect (d->document, &kpDocument::selectionEnabled,
                 d->actionDelete, &QAction::setEnabled);

        connect (d->document, &kpDocument::selectionEnabled,
                 d->actionDeselect, &QAction::setEnabled);

        connect (d->document, &kpDocument::selectionEnabled,
                 d->actionCopyToFile, &QAction::setEnabled);

        // this code won't actually enable any actions at this stage
        // (fresh document) but better safe than sorry
        d->actionCopy->setEnabled (d->document->selection ());
        d->actionCut->setEnabled (d->document->selection ());
        d->actionDeselect->setEnabled (d->document->selection ());
        d->actionDelete->setEnabled (d->document->selection ());
        d->actionCopyToFile->setEnabled (d->document->selection ());

        connect (d->document, &kpDocument::selectionEnabled,
                 this, &kpMainWindow::slotImageMenuUpdateDueToSelection);

        connect (d->document, &kpDocument::selectionIsTextChanged,
                 this, &kpMainWindow::slotImageMenuUpdateDueToSelection);

        // Status bar
        connect (d->document, &kpDocument::documentOpened,
                 this, &kpMainWindow::recalculateStatusBar);

        connect (d->document, SIGNAL (sizeChanged(QSize)),
                 this, SLOT (setStatusBarDocSize(QSize)));

        // Caption (url, modified)
        connect (d->document, &kpDocument::documentModified,
                 this, &kpMainWindow::slotUpdateCaption);

        connect (d->document, &kpDocument::documentOpened,
                 this, &kpMainWindow::slotUpdateCaption);

        connect (d->document, &kpDocument::documentSaved,
                 this, &kpMainWindow::slotUpdateCaption);

        // File/Reload action only available with non-empty URL
        connect (d->document, &kpDocument::documentSaved,
                 this, &kpMainWindow::slotEnableReload);

        connect (d->document, &kpDocument::documentSaved,
                 this, &kpMainWindow::slotEnableSettingsShowPath);

        // Command history
        Q_ASSERT (d->commandHistory);
        connect (d->commandHistory, &kpCommandHistory::documentRestored,
                 this, &kpMainWindow::slotDocumentRestored); // caption "!modified"

        connect (d->document, &kpDocument::documentSaved,
                 d->commandHistory, &kpCommandHistory::documentSaved);

        // Sync document -> views
        connect (d->document, &kpDocument::contentsChanged,
                 d->viewManager, &kpViewManager::updateViews);

        connect (d->document, static_cast<void (kpDocument::*)(int, int)>(&kpDocument::sizeChanged),
                 d->viewManager, &kpViewManager::adjustViewsToEnvironment);

        connect (d->document,
                 static_cast<void (kpDocument::*)(int, int)>(&kpDocument::sizeChanged),
                 d->viewManager, &kpViewManager::adjustViewsToEnvironment);

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tenabling actions";
    #endif

        // sync with the bit marked "sync" above

        Q_ASSERT (d->colorToolBar);
        d->colorToolBar->setEnabled (true);


        // Hide the text toolbar - it will be shown by kpToolText::begin()
        enableTextToolBarActions (false);

        enableToolsDocumentActions (true);

        enableDocumentActions (true);
    }

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tupdating mainWindow elements";
#endif

    slotImageMenuUpdateDueToSelection ();
    recalculateStatusBar ();
    slotUpdateCaption ();  // Untitled to start with
    slotEnableReload ();
    slotEnableSettingsShowPath ();

    if (d->commandHistory) {
        d->commandHistory->clear ();
    }

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tdocument and views ready to go!";
#endif
}

//---------------------------------------------------------------------

// private virtual [base QWidget]
void kpMainWindow::dragEnterEvent (QDragEnterEvent *e)
{
    // It's faster to test for QMimeData::hasText() first due to the
    // lazy evaluation of the '||' operator.
    e->setAccepted (e->mimeData ()->hasText () ||
                    e->mimeData ()->hasUrls () ||
                    kpSelectionDrag::canDecode (e->mimeData ()));
}

//---------------------------------------------------------------------

// private virtual [base QWidget]
void kpMainWindow::dropEvent (QDropEvent *e)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::dropEvent" << e->pos ();
#endif

    QList<QUrl> urls;

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
    else if (!(urls = e->mimeData ()->urls ()).isEmpty ())
    {
        // LOTODO: kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);
        //
        //         However, you would need to prefix all possible error/warning
        //         dialogs that might be called, with Qt::arrowCursor e.g. in
        //         kpDocument  and probably a lot more places.
        for (const auto &u : urls)
            open (u);
    }
    else if (e->mimeData ()->hasText ())
    {
        const QString text = e->mimeData ()->text ();

        QPoint selTopLeft = KP_INVALID_POINT;
        const QPoint globalPos = QWidget::mapToGlobal (e->pos ());
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tpos toGlobal=" << globalPos;
    #endif

        kpView *view = nullptr;

        if (d->viewManager)
        {
            view = d->viewManager->viewUnderCursor ();
        #if DEBUG_KP_MAIN_WINDOW
            qCDebug(kpLogMainWindow) << "\t\tviewUnderCursor=" << view;
        #endif
            if (!view)
            {
                // HACK: see kpViewManager::setViewUnderCursor() to see why
                //       it's not reliable
            #if DEBUG_KP_MAIN_WINDOW
                qCDebug(kpLogMainWindow) << "\t\tattempting to discover view";

                if (d->mainView && d->scrollView)
                {
                    qCDebug(kpLogMainWindow) << "\t\t\tmainView->globalRect="
                            << kpWidgetMapper::toGlobal (d->mainView, d->mainView->rect ())
                            << " scrollView->globalRect="
                            << kpWidgetMapper::toGlobal (d->scrollView,
                                    QRect (0, 0,
                                            d->scrollView->viewport()->width (),
                                            d->scrollView->viewport()->height ()));
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
    if (d->document) {
        d->document->setModified (false);
    }
    slotUpdateCaption ();
}

//---------------------------------------------------------------------

