
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2007 Martin Koller <m.koller@surfeu.at>
   Copyright (c) 2007 John Layt <john@layt.net>
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

#include <qdatastream.h>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <QtDBus>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

#include <kapplication.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krecentfilesaction.h>
#include <kscan.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kdeprintdialog.h>
#include <kprintpreview.h>
#include <kurlcombobox.h>

#include <kpCommandHistory.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpDocumentMetaInfoCommand.h>
#include <kpDocumentMetaInfoDialog.h>
#include <kpDocumentSaveOptionsWidget.h>
#include <kpPixmapFX.h>
#include <kpPrintDialogPage.h>
#include <kpView.h>
#include <kpViewManager.h>


// private
void kpMainWindow::setupFileMenuActions ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::setupFileMenuActions()";
#endif
    KActionCollection *ac = actionCollection ();

    d->actionNew = KStandardAction::openNew (this, SLOT (slotNew ()), ac);
    d->actionOpen = KStandardAction::open (this, SLOT (slotOpen ()), ac);

    d->actionOpenRecent = KStandardAction::openRecent (this, SLOT (slotOpenRecent (const KUrl &)), ac);
    d->actionOpenRecent->loadEntries (KGlobal::config ()->group (kpSettingsGroupRecentFiles));
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\trecent URLs=" << d->actionOpenRecent->items ();
#endif

    d->actionSave = KStandardAction::save (this, SLOT (slotSave ()), ac);
    d->actionSaveAs = KStandardAction::saveAs (this, SLOT (slotSaveAs ()), ac);

    d->actionExport = ac->addAction("file_export");
    d->actionExport->setText (i18n ("E&xport..."));
    d->actionExport->setIcon (KIcon ("document-export"));
    connect(d->actionExport, SIGNAL(triggered(bool) ), SLOT (slotExport ()));

    d->actionScan = ac->addAction("file_scan");
    d->actionScan->setText (i18n ("Scan..."));
    d->actionScan->setIcon (SmallIcon ("scanner"));
    connect(d->actionScan, SIGNAL(triggered(bool) ), SLOT (slotScan ()));

    d->actionProperties = ac->addAction ("file_properties");
    d->actionProperties->setText (i18n ("Properties"));
    d->actionProperties->setIcon (KIcon ("document-properties"));
    connect (d->actionProperties, SIGNAL (triggered (bool)), SLOT (slotProperties ()));

    //d->actionRevert = KStandardAction::revert (this, SLOT (slotRevert ()), ac);
    d->actionReload = ac->addAction ("file_revert");
    d->actionReload->setText (i18n ("Reloa&d"));
    d->actionReload->setIcon (KIcon ("view-refresh"));
    connect(d->actionReload, SIGNAL(triggered(bool) ), SLOT (slotReload ()));
    d->actionReload->setShortcuts(KStandardShortcut::reload ());
    slotEnableReload ();

    d->actionPrint = KStandardAction::print (this, SLOT (slotPrint ()), ac);
    d->actionPrintPreview = KStandardAction::printPreview (this, SLOT (slotPrintPreview ()), ac);

    d->actionMail = KStandardAction::mail (this, SLOT (slotMail ()), ac);

    d->actionSetAsWallpaperCentered = ac->addAction ("file_set_as_wallpaper_centered");
    d->actionSetAsWallpaperCentered->setText (i18n ("Set as Wa&llpaper (Centered)"));
    connect(d->actionSetAsWallpaperCentered, SIGNAL(triggered(bool) ), SLOT (slotSetAsWallpaperCentered ()));
    d->actionSetAsWallpaperTiled = ac->addAction ("file_set_as_wallpaper_tiled");
    d->actionSetAsWallpaperTiled->setText (i18n ("Set as Wallpaper (&Tiled)"));
    connect(d->actionSetAsWallpaperTiled, SIGNAL(triggered(bool) ), SLOT (slotSetAsWallpaperTiled ()));

    d->actionClose = KStandardAction::close (this, SLOT (slotClose ()), ac);
    d->actionQuit = KStandardAction::quit (this, SLOT (slotQuit ()), ac);

    d->scanDialog = 0;

    enableFileMenuDocumentActions (false);
}

// private
void kpMainWindow::enableFileMenuDocumentActions (bool enable)
{
    // d->actionNew
    // d->actionOpen

    // d->actionOpenRecent

    d->actionSave->setEnabled (enable);
    d->actionSaveAs->setEnabled (enable);

    d->actionExport->setEnabled (enable);

    // d->actionScan

    d->actionProperties->setEnabled (enable);

    // d->actionReload

    d->actionPrint->setEnabled (enable);
    d->actionPrintPreview->setEnabled (enable);

    d->actionMail->setEnabled (enable);

    d->actionSetAsWallpaperCentered->setEnabled (enable);
    d->actionSetAsWallpaperTiled->setEnabled (enable);

    d->actionClose->setEnabled (enable);
    // d->actionQuit->setEnabled (enable);
}

// private
void kpMainWindow::addRecentURL (const KUrl &url_)
{
    // HACK: KRecentFilesAction::loadEntries() clears the KRecentFilesAction::d->urls
    //       map.
    //
    //       So afterwards, the URL ref, our method is given, points to an
    //       element in this now-cleared map (see KRecentFilesAction::urlSelected(QAction*)).
    //       Accessing it would result in a crash.
    //
    //       To avoid the crash, make a copy of it before calling
    //       loadEntries() and use this copy, instead of the to-be-dangling
    //       ref.
    const KUrl url = url_;

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::addRecentURL(" << url << ")";
#endif
    if (url.isEmpty ())
        return;


    KSharedConfig::Ptr cfg = KGlobal::config();

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    cfg->reparseConfiguration ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\trecent URLs=" << d->actionOpenRecent->items ();
#endif
    // HACK: Something might have changed interprocess.
    // If we could PROPAGATE: interprocess, then this wouldn't be required.
    d->actionOpenRecent->loadEntries (cfg->group (kpSettingsGroupRecentFiles));
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tafter loading config=" << d->actionOpenRecent->items ();
#endif

    d->actionOpenRecent->addUrl (url);

    d->actionOpenRecent->saveEntries (cfg->group (kpSettingsGroupRecentFiles));
    cfg->sync ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tnew recent URLs=" << d->actionOpenRecent->items ();
#endif


    // TODO: PROPAGATE: interprocess
    // TODO: Is this loop safe since a KMainWindow later along in the list,
    //       could be closed as the code in the body almost certainly re-enters
    //       the event loop?  Problem for KDE 3 as well, I think.
    foreach (KMainWindow *kmw, KMainWindow::memberList ())
    {
        Q_ASSERT (dynamic_cast <kpMainWindow *> (kmw));
        kpMainWindow *mw = static_cast <kpMainWindow *> (kmw);

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\t\tmw=" << mw;
    #endif

        if (mw != this)
        {
            // WARNING: Do not use KRecentFilesAction::setItems()
            //          - it does not work since only its superclass,
            //          KSelectAction, implements setItems() and can't
            //          update KRecentFilesAction's URL list.

            // Avoid URL memory leak in KRecentFilesAction::loadEntries().
            mw->d->actionOpenRecent->clear ();

            mw->d->actionOpenRecent->loadEntries (cfg->group (kpSettingsGroupRecentFiles));
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\t\t\tcheck recent URLs="
                        << mw->d->actionOpenRecent->items () << endl;
        #endif
        }
    }
}


// private slot
// TODO: Disable action if
//       (d->configOpenImagesInSameWindow && d->document && d->document->isEmpty())
//       as it does nothing if this is true.
void kpMainWindow::slotNew ()
{
    toolEndShape ();

    if (d->document && !d->configOpenImagesInSameWindow)
    {
        // A document -- empty or otherwise -- is open.
        // Force open a new window.  In contrast, open() might not open
        // a new window in this case.
        kpMainWindow *win = new kpMainWindow ();
        win->show ();
    }
    else
    {
        open (KUrl (), true/*create an empty doc*/);
    }
}


// private
QSize kpMainWindow::defaultDocSize () const
{
    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    KGlobal::config ()->reparseConfiguration ();

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

    QSize docSize = cfg.readEntry (kpSettingLastDocSize, QSize ());

    if (docSize.isEmpty ())
    {
        docSize = QSize (400, 300);
    }
    else
    {
        // Don't get too big or you'll thrash (or even lock up) the computer
        // just by opening a window
        docSize = QSize (qMin (2048, docSize.width ()),
                         qMin (2048, docSize.height ()));
    }

    return docSize;
}

// private
void kpMainWindow::saveDefaultDocSize (const QSize &size)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tCONFIG: saving Last Doc Size = " << size;
#endif

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

    cfg.writeEntry (kpSettingLastDocSize, size);
    cfg.sync ();
}


// private
bool kpMainWindow::shouldOpen ()
{
    if (d->configOpenImagesInSameWindow)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\topenImagesInSameWindow";
    #endif
        // (this brings up a dialog and might save the current doc)
        if (!queryCloseDocument ())
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\t\tqueryCloseDocument() aborts open";
        #endif
            return false;
        }
    }

    return true;
}

// private
void kpMainWindow::setDocumentChoosingWindow (kpDocument *doc)
{
    // Want new window?
    if (d->document && !d->document->isEmpty () &&
        !d->configOpenImagesInSameWindow)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\topen in new window";
    #endif
        // Send doc to new window.
        kpMainWindow *win = new kpMainWindow (doc);
        win->show ();
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\topen in same window";
    #endif
        // (sets up views, doc signals)
        setDocument (doc);
    }
}


// private
kpDocument *kpMainWindow::openInternal (const KUrl &url,
        const QSize &fallbackDocSize,
        bool newDocSameNameIfNotExist)
{
    // If using OpenImagesInSameWindow mode, ask whether to close the
    // current document.
    if (!shouldOpen ())
        return 0;

    // Create/open doc.
    kpDocument *newDoc = new kpDocument (fallbackDocSize.width (),
                                         fallbackDocSize.height (),
                                         documentEnvironment ());
    if (!newDoc->open (url, newDocSameNameIfNotExist))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\topen failed";
    #endif
        delete newDoc;
        return 0;
    }

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\topen OK";
#endif
    // Send document to current or new window.
    setDocumentChoosingWindow (newDoc);

    return newDoc;
}

// private
bool kpMainWindow::open (const KUrl &url, bool newDocSameNameIfNotExist)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::open(" << url
              << ",newDocSameNameIfNotExist=" << newDocSameNameIfNotExist
              << ")" << endl;
#endif

    kpDocument *newDoc = openInternal (url,
                                       defaultDocSize (),
                                       newDocSameNameIfNotExist);
    if (newDoc)
    {
        if (newDoc->isFromURL (false/*don't bother checking exists*/))
            addRecentURL (url);
        return true;
    }
    else
    {
        return false;
    }
}


// private
KUrl::List kpMainWindow::askForOpenURLs (const QString &caption, const QString &startURL,
                                         bool allowMultipleURLs)
{
    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Reading);
#if DEBUG_KP_MAIN_WINDOW
    QStringList sortedMimeTypes = mimeTypes;
    sortedMimeTypes.sort ();
    kDebug () << "kpMainWindow::askForURLs(allowMultiple="
               << allowMultipleURLs
               << ")" << endl
               << "\tmimeTypes=" << mimeTypes << endl
               << "\tsortedMimeTypes=" << sortedMimeTypes << endl;
#endif
    QString filter = mimeTypes.join (" ");

    KFileDialog fd (startURL, filter, this);
    fd.setCaption (caption);
    fd.setOperationMode (KFileDialog::Opening);

    if (allowMultipleURLs)
        fd.setMode (KFile::Files);

    if (fd.exec ())
        return fd.selectedUrls ();
    else
        return KUrl::List ();
}

// private slot
void kpMainWindow::slotOpen ()
{
    toolEndShape ();


    const KUrl::List urls = askForOpenURLs (i18n ("Open Image"),
        d->document ? d->document->url ().url () : QString());

    for (KUrl::List::const_iterator it = urls.begin ();
         it != urls.end ();
         it++)
    {
        open (*it);
    }
}

// private slot
void kpMainWindow::slotOpenRecent (const KUrl &url)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotOpenRecent(" << url << ")";
    kDebug () << "\titems=" << d->actionOpenRecent->items ();
#endif

    toolEndShape ();

    open (url);

    // If the open is successful, addRecentURL() would have bubbled up the
    // URL in the File / Open Recent action.  As a side effect, the URL is
    // deselected.
    //
    // If the open fails, we should deselect the URL:
    //
    // 1. for consistency
    //
    // 2. because it has not been opened.
    //
    d->actionOpenRecent->setCurrentItem (-1);
}


// private slot
void kpMainWindow::slotScan ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotScan() scanDialog=" << d->scanDialog;
#endif

    toolEndShape ();


    if (!d->scanDialog)
    {
        // Create scan dialog by looking for plugin.
        // [takes about 500ms on 350Mhz]
        d->scanDialog = KScanDialog::getScanDialog (this);

        // No scanning support (kdegraphics/libkscan) installed?
        // [Remove $KDEDIR/share/servicetypes/kscan.desktop and
        //         $KDEDIR/share/services/scanservice.desktop to simulate this]
        if (!d->scanDialog)
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tcould not create scan dialog";
        #endif
            // Instead, we could try to create the scan dialog in the ctor
            // and just disable the action in the first place, removing
            // the need for this dialog.
            //
            // But this increases startup time and is a bit risky e.g. if
            // the scan support hangs, KolourPaint would not be able to be
            // started at all.
            //
            // Also, disabling the action is bad because the scan support
            // can be installed while KolourPaint is still running.
            KMessageBox::sorry (this,
                         i18n ("Scanning support is not installed."),
                         i18n ("No Scanning Support"));
            return;
        }

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tcreated scanDialog=" << d->scanDialog;
    #endif
        connect (d->scanDialog, SIGNAL (finalImage (const QImage &, int)),
                 SLOT (slotScanned (const QImage &, int)));
    }


    // If using OpenImagesInSameWindow mode, ask whether to close the
    // current document.
    //
    // Do this after scan support is detected.  Because if it's not, what
    // would be the point of closing the document?
    //
    // Ideally, we would do this after the user presses "Final Scan" in
    // the scan dialog and before the scan begins (if the user wants to
    // cancel the scan operation, it would be annoying to offer this choice
    // only after the slow scan is completed) but the KScanDialog API does
    // not allow this.  So we settle for doing this before any
    // scan dialogs are shown.  We don't do this between KScanDialog::setup()
    // and KScanDialog::exec() as it could be confusing alternating between
    // scanning and KolourPaint dialogs.
    if (!shouldOpen ())
        return;


#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tcalling setup";
#endif
    // Bring up dialog to select scan device.
    // If there is no scanner, we find that this does not bring up a dialog
    // but still returns true.
    if (d->scanDialog->setup ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\t\tOK - showing dialog";
    #endif
        // Called only if scanner configured/available.
        //
        // In reality, this seems to be called even if you press "Cancel" in
        // the KScanDialog::setup() dialog!
        //
        // We use exec() to make sure it's modal.  show() seems to work too
        // but better safe than sorry.
        d->scanDialog->exec ();
    }
    else
    {
        // Have never seen this code path execute even if "Cancel" is pressed.
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\t\tFAIL";
    #endif
    }
}

// private slot
void kpMainWindow::slotScanned (const QImage &image, int)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotScanned() image.rect=" << image.rect ();
#endif

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\thiding dialog";
#endif
    // (KScanDialog does not close itself after a scan is made)
    //
    // Close the dialog, first thing:
    //
    // 1. This means that any dialogs we bring up won't be nested on top.
    //
    // 2. We don't want to return from this method but forget to close
    //    the dialog.  So do it before anything else.
    d->scanDialog->hide ();

    // (just in case there's some drawing between slotScan() exiting and
    //  us being called)
    toolEndShape ();


    // TODO: Maybe this code should be moved into kpdocument.cpp -
    //       since it resembles the responsibilities of kpDocument::open().

    // Convert QImage to kpDocument's image format, gathering meta info
    // from QImage.
    kpDocumentSaveOptions saveOptions;
    kpDocumentMetaInfo metaInfo;
    const QPixmap pixmap = kpDocument::convertToPixmapAsLosslessAsPossible (
        image,
        kpMainWindow::pasteWarnAboutLossInfo (),
        &saveOptions,
        &metaInfo);

    if (pixmap.isNull ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tcould not convert to pixmap";
    #endif
        KMessageBox::sorry (this,
                            i18n ("Cannot scan - out of graphics memory."),
                            i18n ("Cannot Scan"));
        return;
    }


    // Create document from image and meta info.
    kpDocument *doc = new kpDocument (pixmap.width (), pixmap.height (),
                                      documentEnvironment ());
    doc->setImage (pixmap);
    doc->setSaveOptions (saveOptions);
    doc->setMetaInfo (metaInfo);


    // Send document to current or new window.
    setDocumentChoosingWindow (doc);
}


// private slot
void kpMainWindow::slotProperties ()
{
    toolEndShape ();

    kpDocumentMetaInfoDialog dialog (document ()->metaInfo (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        commandHistory ()->addCommand (
            new kpDocumentMetaInfoCommand (
                i18n ("Document Properties"),
                dialog.metaInfo ()/*new*/, *document ()->metaInfo ()/*old*/,
                commandEnvironment ()));
    }
}


// private slot
bool kpMainWindow::save (bool localOnly)
{
    if (d->document->url ().isEmpty () ||
        !KImageIO::mimeTypes (KImageIO::Writing)
            .contains (d->document->saveOptions ()->mimeType ()) ||
        // SYNC: kpDocument::getPixmapFromFile() can't determine quality
        //       from file so it has been set initially to an invalid value.
        (d->document->saveOptions ()->mimeTypeHasConfigurableQuality () &&
            d->document->saveOptions ()->qualityIsInvalid ()) ||
        (localOnly && !d->document->url ().isLocalFile ()))
    {
        return saveAs (localOnly);
    }
    else
    {
        if (d->document->save (false/*no overwrite prompt*/,
                              !d->document->savedAtLeastOnceBefore ()/*lossy prompt*/))
        {
            addRecentURL (d->document->url ());
            return true;
        }
        else
            return false;
    }
}

// private slot
bool kpMainWindow::slotSave ()
{
    toolEndShape ();

    return save ();
}

// private
KUrl kpMainWindow::askForSaveURL (const QString &caption,
                                  const QString &startURL,
                                  const kpImage &imageToBeSaved,
                                  const kpDocumentSaveOptions &startSaveOptions,
                                  const kpDocumentMetaInfo &docMetaInfo,
                                  const QString &forcedSaveOptionsGroup,
                                  bool localOnly,
                                  kpDocumentSaveOptions *chosenSaveOptions,
                                  bool isSavingForFirstTime,
                                  bool *allowOverwritePrompt,
                                  bool *allowLossyPrompt)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::askForURL() startURL=" << startURL;
    startSaveOptions.printDebug ("\tstartSaveOptions");
#endif

    bool reparsedConfiguration = false;

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    // so reparseConfiguration() must be called
#define SETUP_READ_CFG()                                                             \
    if (!reparsedConfiguration)                                                      \
    {                                                                                \
        KGlobal::config ()->reparseConfiguration ();                                 \
        reparsedConfiguration = true;                                                \
    }                                                                                \
                                                                                     \
    KConfigGroup cfg (KGlobal::config (), forcedSaveOptionsGroup);    \


    if (chosenSaveOptions)
        *chosenSaveOptions = kpDocumentSaveOptions ();

    if (allowOverwritePrompt)
        *allowOverwritePrompt = true;  // play it safe for now

    if (allowLossyPrompt)
        *allowLossyPrompt = true;  // play it safe for now


    kpDocumentSaveOptions fdSaveOptions = startSaveOptions;

    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Writing);
#if DEBUG_KP_MAIN_WINDOW
    QStringList sortedMimeTypes = mimeTypes;
    sortedMimeTypes.sort ();
    kDebug () << "\tmimeTypes=" << mimeTypes
               << "\tsortedMimeTypes=" << sortedMimeTypes << endl;
#endif
    if (mimeTypes.isEmpty ())
    {
        kError () << "No KImageIO output mimetypes!" << endl;
        return KUrl ();
    }

#define MIME_TYPE_IS_VALID() (!fdSaveOptions.mimeTypeIsInvalid () &&                 \
                              mimeTypes.contains (fdSaveOptions.mimeType ()))
    if (!MIME_TYPE_IS_VALID ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tmimeType=" << fdSaveOptions.mimeType ()
                   << " not valid, get default" << endl;
    #endif

        SETUP_READ_CFG ();

        fdSaveOptions.setMimeType (kpDocumentSaveOptions::defaultMimeType (cfg));


        if (!MIME_TYPE_IS_VALID ())
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tmimeType=" << fdSaveOptions.mimeType ()
                       << " not valid, get hardcoded" << endl;
        #endif
            if (mimeTypes.contains ("image/png"))
                fdSaveOptions.setMimeType ("image/png");
            else if (mimeTypes.contains ("image/bmp"))
                fdSaveOptions.setMimeType ("image/bmp");
            else
                fdSaveOptions.setMimeType (mimeTypes.first ());
        }
    }
#undef MIME_TYPE_IN_LIST

    if (fdSaveOptions.colorDepthIsInvalid ())
    {
        SETUP_READ_CFG ();

        fdSaveOptions.setColorDepth (kpDocumentSaveOptions::defaultColorDepth (cfg));
        fdSaveOptions.setDither (kpDocumentSaveOptions::defaultDither (cfg));
    }

    if (fdSaveOptions.qualityIsInvalid ())
    {
        SETUP_READ_CFG ();

        fdSaveOptions.setQuality (kpDocumentSaveOptions::defaultQuality (cfg));
    }
#if DEBUG_KP_MAIN_WINDOW
    fdSaveOptions.printDebug ("\tcorrected saveOptions passed to fileDialog");
#endif

    kpDocumentSaveOptionsWidget *saveOptionsWidget =
        new kpDocumentSaveOptionsWidget (imageToBeSaved,
            fdSaveOptions,
            docMetaInfo,
            this);

    KFileDialog fd (startURL, QString(), this,
                    saveOptionsWidget);
    saveOptionsWidget->setVisualParent (&fd);
    fd.setCaption (caption);
    fd.setOperationMode (KFileDialog::Saving);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tmimeTypes=" << mimeTypes;
#endif
    fd.setMimeFilter (mimeTypes, fdSaveOptions.mimeType ());
    if (localOnly)
        fd.setMode (KFile::File | KFile::LocalOnly);

    connect (&fd, SIGNAL (filterChanged (const QString &)),
             saveOptionsWidget, SLOT (setMimeType (const QString &)));

    //Honor the current filename if the mimetype is the same
    KMimeType::Ptr mime = KMimeType::findByUrl(startURL);
    if (mime->is(fdSaveOptions.mimeType()))
        fd.locationEdit()->setUrl(KUrl(startURL).fileName());

    if (fd.exec ())
    {
        kpDocumentSaveOptions newSaveOptions = saveOptionsWidget->documentSaveOptions ();
    #if DEBUG_KP_MAIN_WINDOW
        newSaveOptions.printDebug ("\tnewSaveOptions");
    #endif

        KConfigGroup cfg (KGlobal::config (), forcedSaveOptionsGroup);

        // Save options user forced - probably want to use them in future
        kpDocumentSaveOptions::saveDefaultDifferences (cfg,
            fdSaveOptions, newSaveOptions);
        cfg.sync ();


        if (chosenSaveOptions)
            *chosenSaveOptions = newSaveOptions;


        bool shouldAllowOverwritePrompt =
                (fd.selectedUrl () != startURL ||
                 newSaveOptions.mimeType () != startSaveOptions.mimeType ());
        if (allowOverwritePrompt)
        {
            *allowOverwritePrompt = shouldAllowOverwritePrompt;
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tallowOverwritePrompt=" << *allowOverwritePrompt;
        #endif
        }

        if (allowLossyPrompt)
        {
            // SYNC: kpDocumentSaveOptions elements - everything except quality
            //       (one quality setting is "just as lossy" as another so no
            //        need to continually warn due to quality change)
            *allowLossyPrompt =
                (isSavingForFirstTime ||
                 shouldAllowOverwritePrompt ||
                 newSaveOptions.mimeType () != startSaveOptions.mimeType () ||
                 newSaveOptions.colorDepth () != startSaveOptions.colorDepth () ||
                 newSaveOptions.dither () != startSaveOptions.dither ());
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\tallowLossyPrompt=" << *allowLossyPrompt;
        #endif
        }


    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tselectedUrl=" << fd.selectedUrl ();
    #endif
        return fd.selectedUrl ();
    }
    else
        return KUrl ();
#undef SETUP_READ_CFG
}


// private slot
bool kpMainWindow::saveAs (bool localOnly)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::saveAs URL=" << d->document->url ();
#endif

    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KUrl chosenURL = askForSaveURL (i18n ("Save Image As"),
                                    d->document->url ().url (),
                                    d->document->imageWithSelection (),
                                    *d->document->saveOptions (),
                                    *d->document->metaInfo (),
                                    kpSettingsGroupFileSaveAs,
                                    localOnly,
                                    &chosenSaveOptions,
                                    !d->document->savedAtLeastOnceBefore (),
                                    &allowOverwritePrompt,
                                    &allowLossyPrompt);


    if (chosenURL.isEmpty ())
        return false;


    if (!d->document->saveAs (chosenURL, chosenSaveOptions,
                             allowOverwritePrompt,
                             allowLossyPrompt))
    {
        return false;
    }


    addRecentURL (chosenURL);

    return true;
}

// private slot
bool kpMainWindow::slotSaveAs ()
{
    toolEndShape ();

    return saveAs ();
}

// private slot
bool kpMainWindow::slotExport ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotExport()";
#endif

    toolEndShape ();


    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KUrl chosenURL = askForSaveURL (i18n ("Export"),
                                    d->lastExportURL.url (),
                                    d->document->imageWithSelection (),
                                    d->lastExportSaveOptions,
                                    *d->document->metaInfo (),
                                    kpSettingsGroupFileExport,
                                    false/*allow remote files*/,
                                    &chosenSaveOptions,
                                    d->exportFirstTime,
                                    &allowOverwritePrompt,
                                    &allowLossyPrompt);


    if (chosenURL.isEmpty ())
        return false;


    if (!kpDocument::savePixmapToFile (d->document->imageWithSelection (),
                                       chosenURL,
                                       chosenSaveOptions, *d->document->metaInfo (),
                                       allowOverwritePrompt,
                                       allowLossyPrompt,
                                       this))
    {
        return false;
    }


    addRecentURL (chosenURL);


    d->lastExportURL = chosenURL;
    d->lastExportSaveOptions = chosenSaveOptions;

    d->exportFirstTime = false;

    return true;
}


// private slot
void kpMainWindow::slotEnableReload ()
{
    d->actionReload->setEnabled (d->document);
}

// private slot
bool kpMainWindow::slotReload ()
{
    toolEndShape ();

    Q_ASSERT (d->document);


    KUrl oldURL = d->document->url ();


    if (d->document->isModified ())
    {
        int result = KMessageBox::Cancel;

        if (d->document->isFromURL (false/*don't bother checking exists*/) && !oldURL.isEmpty ())
        {
            result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes since you last saved it.\n"
                               "Are you sure?",
                               d->document->prettyFilename ()),
                         QString()/*caption*/,
                         KGuiItem(i18n ("&Reload")));
        }
        else
        {
            result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes.\n"
                               "Are you sure?",
                               d->document->prettyFilename ()),
                         QString()/*caption*/,
                         KGuiItem(i18n ("&Reload")));
        }

        if (result != KMessageBox::Continue)
            return false;
    }


    kpDocument *doc = 0;

    // If it's _supposed to_ come from a URL or it exists
    if (d->document->isFromURL (false/*don't bother checking exists*/) ||
        (!oldURL.isEmpty () && KIO::NetAccess::exists (oldURL, KIO::NetAccess::SourceSide/*open*/, this)))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "kpMainWindow::slotReload() reloading from disk!";
    #endif

        doc = new kpDocument (1, 1, documentEnvironment ());
        if (!doc->open (oldURL))
        {
            delete doc; doc = 0;
            return false;
        }

        addRecentURL (oldURL);
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "kpMainWindow::slotReload() create doc";
    #endif

        doc = new kpDocument (d->document->constructorWidth (),
                              d->document->constructorHeight (),
                              documentEnvironment ());
        doc->setURL (oldURL, false/*not from URL*/);
    }


    setDocument (doc);

    return true;
}


// private
void kpMainWindow::sendDocumentNameToPrinter (QPrinter *printer)
{
    KUrl url = d->document->url ();
    if (!url.isEmpty ())
    {
        int dot;

        QString fileName = url.fileName ();
        dot = fileName.lastIndexOf ('.');

        // file.ext but not .hidden-file?
        if (dot > 0)
            fileName.truncate (dot);

    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "kpMainWindow::sendDocumentNameToPrinter() fileName="
                   << fileName
                   << " dir="
                   << url.directory ()
                   << endl;
    #endif
        printer->setDocName (fileName);
    }
}


// private
void kpMainWindow::sendImageToPrinter (QPrinter *printer,
        bool showPrinterSetupDialog)
{
    // Get image to be printed.
    kpImage image = d->document->imageWithSelection ();


    // Get image DPI.
    double imageDotsPerMeterX =
        double (d->document->metaInfo ()->dotsPerMeterX ());
    double imageDotsPerMeterY =
        double (d->document->metaInfo ()->dotsPerMeterY ());
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::sendImageToPrinter() image:"
               << " width=" << image.width ()
               << " height=" << image.height ()
               << " dotsPerMeterX=" << imageDotsPerMeterX
               << " dotsPerMeterY=" << imageDotsPerMeterY
               << endl;
#endif

    // Image DPI invalid (e.g. new image, could not read from file
    // or Qt3 doesn't implement DPI for JPEG)?
    if (imageDotsPerMeterX <= 0 || imageDotsPerMeterY <= 0)
    {
        // Even if just one DPI dimension is invalid, mutate both DPI
        // dimensions as we have no information about the intended
        // aspect ratio anyway (and other dimension likely to be invalid).

        // When rendering text onto a document, the fonts are rasterised
        // according to the screen's DPI.
        // TODO: I think we should use the image's DPI.  Technically
        //       possible?
        //
        //       So no matter what computer you draw text on, you get
        //       the same pixels.
        //
        // So we must print at the screen's DPI to get the right text size.
        //
        // Unfortunately, this means that moving to a different screen DPI
        // affects printing.  If you edited the image at a different screen
        // DPI than when you print, you get incorrect results.  Furthermore,
        // this is bogus if you don't have text in your image.  Worse still,
        // what if you have multiple screens connected to the same computer
        // with different DPIs?
        // TODO: mysteriously, someone else is setting this to 96dpi always.
        QPixmap arbitraryScreenElement;
        const QPaintDevice *screenDevice = &arbitraryScreenElement;
        const int dpiX = screenDevice->logicalDpiX (),
            dpiY = screenDevice->logicalDpiY ();
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tusing screen dpi: x=" << dpiX << " y=" << dpiY;
    #endif

        imageDotsPerMeterX = dpiX * KP_INCHES_PER_METER;
        imageDotsPerMeterY = dpiY * KP_INCHES_PER_METER;
    }


    // Get page size (excluding margins).
    // Coordinate (0,0) is the X here:
    //     mmmmm
    //     mX  m
    //     m   m       m = margin
    //     m   m
    //     mmmmm
    const int printerWidthMM = printer->widthMM ();
    const int printerHeightMM = printer->heightMM ();
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\tprinter: widthMM=" << printerWidthMM
               << " heightMM=" << printerHeightMM
               << endl;
#endif


    double dpiX = imageDotsPerMeterX / KP_INCHES_PER_METER;
    double dpiY = imageDotsPerMeterY / KP_INCHES_PER_METER;
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\timage: dpiX=" << dpiX << " dpiY=" << dpiY;
#endif


    //
    // If image doesn't fit on page at intended DPI, change the DPI.
    //

    const double scaleDpiX =
        (image.width () / (printerWidthMM / KP_MILLIMETERS_PER_INCH))
            / dpiX;
    const double scaleDpiY =
        (image.height () / (printerHeightMM / KP_MILLIMETERS_PER_INCH))
            / dpiY;
    const double scaleDpi = qMax (scaleDpiX, scaleDpiY);
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\t\tscaleDpi: x=" << scaleDpiX << " y=" << scaleDpiY
               << " --> scale at " << scaleDpi << " to fit?"
               << endl;
#endif

    // Need to increase resolution to fit page?
    if (scaleDpi > 1.0)
    {
        dpiX *= scaleDpi;
        dpiY *= scaleDpi;
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\t\t\tto fit page, scaled to:"
                   << " dpiX=" << dpiX << " dpiY=" << dpiY << endl;
    #endif
    }


    // Make sure DPIs are equal as that's all QPrinter::setResolution()
    // supports.  We do this in such a way that we only ever stretch an
    // image, to avoid losing information.  Don't antialias as the printer
    // will do that to translate our DPI to its physical resolution and
    // double-antialiasing looks bad.
    if (dpiX > dpiY)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tdpiX > dpiY; stretching image height to equalise DPIs to dpiX="
                   << dpiX << endl;
    #endif
        kpPixmapFX::scale (&image,
             image.width (),
             qMax (1, qRound (image.height () * dpiX / dpiY)),
             false/*don't antialias*/);

        dpiY = dpiX;
    }
    else if (dpiY > dpiX)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tdpiY > dpiX; stretching image width to equalise DPIs to dpiY="
                   << dpiY << endl;
    #endif
        kpPixmapFX::scale (&image,
             qMax (1, qRound (image.width () * dpiY / dpiX)),
             image.height (),
             false/*don't antialias*/);

        dpiX = dpiY;
    }

    Q_ASSERT (dpiX == dpiY);


    // QPrinter::setResolution() has to be called before QPrinter::setup().
    printer->setResolution (qMax (1, qRound (dpiX)));


    sendDocumentNameToPrinter (printer);


    if (showPrinterSetupDialog)
    {
        kpPrintDialogPage *optionsPage = new kpPrintDialogPage (this);
        optionsPage->setPrintImageCenteredOnPage (d->configPrintImageCenteredOnPage);

        QPrintDialog *printDialog =
            KdePrint::createPrintDialog (
                printer,
                QList <QWidget *> () << optionsPage,
                this);
        printDialog->setWindowTitle (i18n ("Print Image"));

        // Display dialog.
        const bool wantToPrint = printDialog->exec ();

        if (optionsPage->printImageCenteredOnPage () !=
            d->configPrintImageCenteredOnPage)
        {
            // Save config option even if the dialog was cancelled.
            d->configPrintImageCenteredOnPage = optionsPage->printImageCenteredOnPage ();

            KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);
            cfg.writeEntry (kpSettingPrintImageCenteredOnPage,
                           d->configPrintImageCenteredOnPage);
            cfg.sync ();
        }

        delete printDialog;

        if (!wantToPrint)
            return;
    }


    double originX = 0, originY = 0;

    // Center image on page?
    if (d->configPrintImageCenteredOnPage)
    {
        originX =
            (printerWidthMM * dpiX / KP_MILLIMETERS_PER_INCH - image.width ())
                / 2;
        originY =
            (printerHeightMM * dpiY / KP_MILLIMETERS_PER_INCH - image.height ())
                / 2;
    }

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "\torigin: x=" << originX << " y=" << originY;
#endif


    // Send image to printer.
    QPainter painter;
    painter.begin (printer);
    painter.drawPixmap (qRound (originX), qRound (originY), image);
    painter.end ();
}


// private slot
void kpMainWindow::slotPrint ()
{
    toolEndShape ();

    QPrinter printer;

    sendImageToPrinter (&printer, true/*showPrinterSetupDialog*/);
}

// private slot
void kpMainWindow::slotPrintPreview ()
{
    toolEndShape ();

    QPrinter printer;

    KPrintPreview printPreview (&printer);

    sendImageToPrinter (&printer, false/*don't showPrinterSetupDialog*/);

    printPreview.exec ();
}


// private slot
void kpMainWindow::slotMail ()
{
    toolEndShape ();

    if (d->document->url ().isEmpty ()/*no name*/ ||
        !d->document->isFromURL () ||
        d->document->isModified ()/*needs to be saved*/)
    {
        int result = KMessageBox::questionYesNo (this,
                        i18n ("You must save this image before sending it.\n"
                              "Do you want to save it?"),
                        QString(),
                        KStandardGuiItem::save (), KStandardGuiItem::cancel ());

        if (result == KMessageBox::Yes)
        {
            if (!save ())
            {
                // save failed or aborted - don't email
                return;
            }
        }
        else
        {
            // don't want to save - don't email
            return;
        }
    }

    KToolInvocation::invokeMailer (
        QString::null/*to*/,	//krazy:exclude=nullstrassign for old broken gcc
        QString()/*cc*/,
        QString()/*bcc*/,
        d->document->prettyFilename()/*subject*/,
        QString()/*body*/,
        QString()/*messageFile*/,
        QStringList (d->document->url ().url ())/*attachments*/);
}


// private
void kpMainWindow::setAsWallpaper (bool centered)
{
    if (d->document->url ().isEmpty ()/*no name*/ ||
        !d->document->url ().isLocalFile ()/*remote file*/ ||
        !d->document->isFromURL () ||
        d->document->isModified ()/*needs to be saved*/)
    {
        QString question;

        if (!d->document->url ().isLocalFile ())
        {
            question = i18n ("Before this image can be set as the wallpaper, "
                             "you must save it as a local file.\n"
                             "Do you want to save it?");
        }
        else
        {
            question = i18n ("Before this image can be set as the wallpaper, "
                             "you must save it.\n"
                             "Do you want to save it?");
        }

        int result = KMessageBox::questionYesNo (this,
                         question, QString(),
                         KStandardGuiItem::save (), KStandardGuiItem::cancel ());

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
    QDataStream dataStream (&data, QIODevice::WriteOnly);

    // write path
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::setAsWallpaper() path="
               << d->document->url ().path () << endl;
#endif
    dataStream << QString (d->document->url ().path ());

    // write position:
    //
    // SYNC: kdebase/kcontrol/background/bgsettings.h:
    // 1 = Centered
    // 2 = Tiled
    // 6 = Scaled
    // 9 = lastWallpaperMode
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
// COMPAT

#ifdef Q_WS_X11
    int konq_screen_number = KApplication::desktop()->primaryScreen();
    QByteArray appname;
    if (konq_screen_number == 0)
        appname = "org.kde.kdesktop";
    else
        appname = "org.kde.kdesktop-screen-" + QByteArray::number(konq_screen_number);
    QDBusInterface kdesktop(appname, "/Background", "org.kde.kdesktop.Background");
    QDBusReply<void> retVal = kdesktop.call( "setWallpaper", QString (d->document->url ().path ()), int (centered ? 1 : 2) );
    if (!retVal.isValid())
    {
        KMessageBox::sorry (this, i18n ("Could not change wallpaper."));
    }
#else
#ifdef __GNUC__
    #warning "Setting wallpaper not implemented in non-X11"
#endif
#endif
}

// private slot
void kpMainWindow::slotSetAsWallpaperCentered ()
{
    toolEndShape ();

    setAsWallpaper (true/*centered*/);
}

// private slot
void kpMainWindow::slotSetAsWallpaperTiled ()
{
    toolEndShape ();

    setAsWallpaper (false/*tiled*/);
}


// private
bool kpMainWindow::queryCloseDocument ()
{
    toolEndShape ();

    if (!d->document || !d->document->isModified ())
        return true;  // ok to close current doc

    int result = KMessageBox::warningYesNoCancel (this,
                     i18n ("The document \"%1\" has been modified.\n"
                           "Do you want to save it?",
                           d->document->prettyFilename ()),
                    QString()/*caption*/,
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

// private virtual [base KMainWindow]
bool kpMainWindow::queryClose ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::queryClose()";
#endif
    toolEndShape ();

    if (!queryCloseDocument ())
        return false;

    if (!queryCloseColors ())
        return false;

    return true;
}

// private slot
void kpMainWindow::slotClose ()
{
    toolEndShape ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotClose()";
#endif

    if (!queryCloseDocument ())
        return;

    setDocument (0);
}

// private slot
void kpMainWindow::slotQuit ()
{
    toolEndShape ();

#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotQuit()";
#endif

    close ();  // will call queryClose()
}


