
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

#include <qcstring.h>
#include <qdatastream.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qsize.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimagefilepreview.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kscan.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpdocumentsaveoptionswidget.h>
#include <kptool.h>
#include <kpview.h>
#include <kpviewmanager.h>


// private
void kpMainWindow::setupFileMenuActions ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::setupFileMenuActions()" << endl;
#endif
    KActionCollection *ac = actionCollection ();

    m_actionNew = KStdAction::openNew (this, SLOT (slotNew ()), ac);
    m_actionOpen = KStdAction::open (this, SLOT (slotOpen ()), ac);

    m_actionOpenRecent = KStdAction::openRecent (this, SLOT (slotOpenRecent (const KURL &)), ac);
    m_actionOpenRecent->loadEntries (kapp->config ());
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\trecent URLs=" << m_actionOpenRecent->items () << endl;
#endif

    m_actionSave = KStdAction::save (this, SLOT (slotSave ()), ac);
    m_actionSaveAs = KStdAction::saveAs (this, SLOT (slotSaveAs ()), ac);

    m_actionExport = new KAction (i18n ("E&xport..."), 0,
        this, SLOT (slotExport ()), ac, "file_export");

    m_actionScan = new KAction (i18n ("Scan..."), SmallIcon ("scanner"), 0,
        this, SLOT (slotScan ()), ac, "file_scan");

    //m_actionRevert = KStdAction::revert (this, SLOT (slotRevert ()), ac);
    m_actionReload = new KAction (i18n ("Reloa&d"), KStdAccel::reload (),
        this, SLOT (slotReload ()), ac, "file_revert");
    slotEnableReload ();

    m_actionPrint = KStdAction::print (this, SLOT (slotPrint ()), ac);
    m_actionPrintPreview = KStdAction::printPreview (this, SLOT (slotPrintPreview ()), ac);

    m_actionMail = KStdAction::mail (this, SLOT (slotMail ()), ac);

    m_actionSetAsWallpaperCentered = new KAction (i18n ("Set as Wa&llpaper (Centered)"), 0,
        this, SLOT (slotSetAsWallpaperCentered ()), ac, "file_set_as_wallpaper_centered");
    m_actionSetAsWallpaperTiled = new KAction (i18n ("Set as Wallpaper (&Tiled)"), 0,
        this, SLOT (slotSetAsWallpaperTiled ()), ac, "file_set_as_wallpaper_tiled");

    m_actionClose = KStdAction::close (this, SLOT (slotClose ()), ac);
    m_actionQuit = KStdAction::quit (this, SLOT (slotQuit ()), ac);

    m_scanDialog = 0;

    enableFileMenuDocumentActions (false);
}

// private
void kpMainWindow::enableFileMenuDocumentActions (bool enable)
{
    // m_actionNew
    // m_actionOpen

    // m_actionOpenRecent

    m_actionSave->setEnabled (enable);
    m_actionSaveAs->setEnabled (enable);

    m_actionExport->setEnabled (enable);

    // m_actionReload

    m_actionPrint->setEnabled (enable);
    m_actionPrintPreview->setEnabled (enable);

    m_actionMail->setEnabled (enable);

    m_actionSetAsWallpaperCentered->setEnabled (enable);
    m_actionSetAsWallpaperTiled->setEnabled (enable);

    m_actionClose->setEnabled (enable);
    // m_actionQuit->setEnabled (enable);
}


// private
void kpMainWindow::addRecentURL (const KURL &url)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::addRecentURL(" << url << ")" << endl;
#endif
    if (url.isEmpty ())
        return;


    KConfig *cfg = kapp->config ();

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realise what other processes have done e.g. Settings / Show Path
    cfg->reparseConfiguration ();

    // HACK: Something might have changed interprocess.
    // If we could PROPAGATE: interprocess, then this wouldn't be required.
    m_actionOpenRecent->loadEntries (cfg);

    m_actionOpenRecent->addURL (url);

    m_actionOpenRecent->saveEntries (cfg);
    cfg->sync ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tnew recent URLs=" << m_actionOpenRecent->items () << endl;
#endif


    // TODO: PROPAGATE: interprocess
    if (KMainWindow::memberList)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\thave memberList" << endl;
    #endif

        for (QPtrList <KMainWindow>::const_iterator it = KMainWindow::memberList->begin ();
             it != KMainWindow::memberList->end ();
             it++)
        {
            kpMainWindow *mw = dynamic_cast <kpMainWindow *> (*it);

            if (!mw)
            {
                kdError () << "kpMainWindow::addRecentURL() given fake kpMainWindow: " << (*it) << endl;
                continue;
            }
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\t\tmw=" << mw << endl;
        #endif

            if (mw != this)
            {
                // WARNING: Do not use KRecentFilesAction::setItems()
                //          - it does not work since only its superclass,
                //          KSelectAction, implements setItems() and can't
                //          update KRecentFilesAction's URL list.

                // Avoid URL memory leak in KRecentFilesAction::loadEntries().
                mw->m_actionOpenRecent->clearURLList ();

                mw->m_actionOpenRecent->loadEntries (cfg);
            #if DEBUG_KP_MAIN_WINDOW
                kdDebug () << "\t\t\tcheck recent URLs="
                           << mw->m_actionOpenRecent->items () << endl;
            #endif
            }
        }
    }
}


// private slot
void kpMainWindow::slotNew ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (m_document)
    {
        kpMainWindow *win = new kpMainWindow ();
        win->show ();
    }
    else
    {
        open (KURL (), true/*create an empty doc*/);
    }
}


// private
QSize kpMainWindow::defaultDocSize () const
{
    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realise what other processes have done e.g. Settings / Show Path
    kapp->config ()->reparseConfiguration ();

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    QSize docSize = cfg->readSizeEntry (kpSettingLastDocSize);

    if (docSize.isEmpty ())
    {
        docSize = QSize (400, 300);
    }
    else
    {
        // Don't get too big or you'll thrash (or even lock up) the computer
        // just by opening a window
        docSize = QSize (QMIN (2048, docSize.width ()),
                         QMIN (2048, docSize.height ()));
    }

    return docSize;
}

// private
void kpMainWindow::saveDefaultDocSize (const QSize &size)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tCONFIG: saving Last Doc Size = " << size << endl;
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingLastDocSize, size);
    cfg->sync ();
}


// private
bool kpMainWindow::shouldOpenInNewWindow () const
{
    return (m_document && !m_document->isEmpty ());
}

// private
void kpMainWindow::setDocumentChoosingWindow (kpDocument *doc)
{
    // need new window?
    if (shouldOpenInNewWindow ())
    {
        // send doc to new window
        kpMainWindow *win = new kpMainWindow (doc);
        win->show ();
    }
    else
    {
        // set up views, doc signals
        setDocument (doc);
    }
}


// private
kpDocument *kpMainWindow::openInternal (const KURL &url,
        const QSize &fallbackDocSize,
        bool newDocSameNameIfNotExist)
{
    // create doc
    kpDocument *newDoc = new kpDocument (fallbackDocSize.width (),
                                         fallbackDocSize.height (),
                                         this);
    if (!newDoc->open (url, newDocSameNameIfNotExist))
    {
        delete newDoc;
        return 0;
    }

    // Send document to current or new window.
    setDocumentChoosingWindow (newDoc);

    return newDoc;
}

// private
bool kpMainWindow::open (const KURL &url, bool newDocSameNameIfNotExist)
{
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
KURL::List kpMainWindow::askForOpenURLs (const QString &caption, const QString &startURL,
                                         bool allowMultipleURLs)
{
    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Reading);
#if DEBUG_KP_MAIN_WINDOW
    QStringList sortedMimeTypes = mimeTypes;
    sortedMimeTypes.sort ();
    kdDebug () << "kpMainWindow::askForURLs(allowMultiple="
               << allowMultipleURLs
               << ")" << endl
               << "\tmimeTypes=" << mimeTypes << endl
               << "\tsortedMimeTypes=" << sortedMimeTypes << endl;
#endif
    QString filter = mimeTypes.join (" ");

    KFileDialog fd (startURL, filter, this, "fd", true/*modal*/);
    fd.setCaption (caption);
    fd.setOperationMode (KFileDialog::Opening);
    if (allowMultipleURLs)
        fd.setMode (KFile::Files);
    fd.setPreviewWidget (new KImageFilePreview (&fd));

    if (fd.exec ())
        return fd.selectedURLs ();
    else
        return KURL::List ();
}

// private slot
void kpMainWindow::slotOpen ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    const KURL::List urls = askForOpenURLs (i18n ("Open Image"),
        m_document ? m_document->url ().url () : QString::null);

    for (KURL::List::const_iterator it = urls.begin ();
         it != urls.end ();
         it++)
    {
        open (*it);
    }
}

// private slot
void kpMainWindow::slotOpenRecent (const KURL &url)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotOpenRecent(" << url << ")" << endl;
    kdDebug () << "\titems=" << m_actionOpenRecent->items () << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

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
    m_actionOpenRecent->setCurrentItem (-1);
}


// private slot
void kpMainWindow::slotScan ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotScan() scanDialog=" << m_scanDialog << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    if (!m_scanDialog)
    {
        // Create scan dialog by looking for plugin.
        // [takes about 500ms on 350Mhz]
        m_scanDialog = KScanDialog::getScanDialog (this, "scandialog", true/*modal*/);

        // No scanning support (kdegraphics/libkscan) installed?
        // [Remove $KDEDIR/share/servicetypes/kscan.desktop and
        //         $KDEDIR/share/services/scanservice.desktop to simulate this]
        if (!m_scanDialog)
        {
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tcould not create scan dialog" << endl;
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
        kdDebug () << "\tcreated scanDialog=" << m_scanDialog << endl;
    #endif
        connect (m_scanDialog, SIGNAL (finalImage (const QImage &, int)),
                 SLOT (slotScanned (const QImage &, int)));
    }


#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tcalling setup" << endl;
#endif
    // Bring up dialog to select scan device.
    if (m_scanDialog->setup ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\t\tOK - showing dialog" << endl;
    #endif
        // Called only if scanner configured/available.
        //
        // In reality, this seems to be called even if you press "Cancel" in
        // the KScanDialog::setup() dialog!
        m_scanDialog->show ();
    }
    else
    {
        // Have never seen this code path execute even if "Cancel" is pressed.
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\t\tFAIL" << endl;
    #endif
    }
}

// private slot
void kpMainWindow::slotScanned (const QImage &image, int)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotScanned() image.rect=" << image.rect () << endl;
#endif

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\thiding dialog" << endl;
#endif
    // (KScanDialog does not close itself after a scan is made)
    //
    // Close the dialog, first thing:
    //
    // 1. This means that any dialogs we bring up won't be nested on top.
    //
    // 2. We don't want to return from this method but forget to close
    //    the dialog.  So do it before anything else.
    m_scanDialog->hide ();

    // (just in case there's some drawing between slotScan() exiting and
    //  us being called)
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


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
        kdDebug () << "\tcould not convert to pixmap" << endl;
    #endif
        KMessageBox::sorry (this,
                            i18n ("Cannot scan - out of graphics memory."),
                            i18n ("Cannot Scan"));
        return;
    }


    // Create document from image and meta info.
    kpDocument *doc = new kpDocument (pixmap.width (), pixmap.height (), this);
    doc->setPixmap (pixmap);
    doc->setSaveOptions (saveOptions);
    doc->setMetaInfo (metaInfo);


    // Send document to current or new window.
    setDocumentChoosingWindow (doc);
}


// private slot
bool kpMainWindow::save (bool localOnly)
{
    if (m_document->url ().isEmpty () ||
        KImageIO::mimeTypes (KImageIO::Writing)
            .findIndex (m_document->saveOptions ()->mimeType ()) < 0 ||
        // SYNC: kpDocument::getPixmapFromFile() can't determine quality
        //       from file so it has been set initially to an invalid value.
        (m_document->saveOptions ()->mimeTypeHasConfigurableQuality () &&
            m_document->saveOptions ()->qualityIsInvalid ()) ||
        (localOnly && !m_document->url ().isLocalFile ()))
    {
        return saveAs (localOnly);
    }
    else
    {
        if (m_document->save (false/*no overwrite prompt*/,
                              !m_document->savedAtLeastOnceBefore ()/*lossy prompt*/))
        {
            addRecentURL (m_document->url ());
            return true;
        }
        else
            return false;
    }
}

// private slot
bool kpMainWindow::slotSave ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    return save ();
}

// private
KURL kpMainWindow::askForSaveURL (const QString &caption,
                                  const QString &startURL,
                                  const QPixmap &pixmapToBeSaved,
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
    kdDebug () << "kpMainWindow::askForURL() startURL=" << startURL << endl;
    startSaveOptions.printDebug ("\tstartSaveOptions");
#endif

    bool reparsedConfiguration = false;

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realise what other processes have done e.g. Settings / Show Path
    // so reparseConfiguration() must be called
#define SETUP_READ_CFG()                                                          \
    if (!reparsedConfiguration)                                                   \
    {                                                                             \
        kapp->config ()->reparseConfiguration ();                                 \
        reparsedConfiguration = true;                                             \
    }                                                                             \
                                                                                  \
    KConfigGroupSaver cfgGroupSaver (kapp->config (), forcedSaveOptionsGroup);    \
    KConfigBase *cfg = cfgGroupSaver.config ();


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
    kdDebug () << "\tmimeTypes=" << mimeTypes << endl
               << "\tsortedMimeTypes=" << sortedMimeTypes << endl;
#endif
    if (mimeTypes.isEmpty ())
    {
        kdError () << "No KImageIO output mimetypes!" << endl;
        return KURL ();
    }

#define MIME_TYPE_IS_VALID() (!fdSaveOptions.mimeTypeIsInvalid () &&                 \
                              mimeTypes.findIndex (fdSaveOptions.mimeType ()) >= 0)
    if (!MIME_TYPE_IS_VALID ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tmimeType=" << fdSaveOptions.mimeType ()
                   << " not valid, get default" << endl;
    #endif

        SETUP_READ_CFG ();

        fdSaveOptions.setMimeType (kpDocumentSaveOptions::defaultMimeType (cfg));


        if (!MIME_TYPE_IS_VALID ())
        {
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tmimeType=" << fdSaveOptions.mimeType ()
                       << " not valid, get hardcoded" << endl;
        #endif
            if (mimeTypes.findIndex ("image/png") > -1)
                fdSaveOptions.setMimeType ("image/png");
            else if (mimeTypes.findIndex ("image/x-bmp") > -1)
                fdSaveOptions.setMimeType ("image/x-bmp");
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
        new kpDocumentSaveOptionsWidget (pixmapToBeSaved,
            fdSaveOptions,
            docMetaInfo,
            this);

    KFileDialog fd (startURL, QString::null, this, "fd", true/*modal*/,
                    saveOptionsWidget);
    saveOptionsWidget->setVisualParent (&fd);
    fd.setCaption (caption);
    fd.setOperationMode (KFileDialog::Saving);
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tmimeTypes=" << mimeTypes << endl;
#endif
    fd.setMimeFilter (mimeTypes, fdSaveOptions.mimeType ());
    if (localOnly)
        fd.setMode (KFile::File | KFile::LocalOnly);

    connect (&fd, SIGNAL (filterChanged (const QString &)),
             saveOptionsWidget, SLOT (setMimeType (const QString &)));


    if (fd.exec ())
    {
        kpDocumentSaveOptions newSaveOptions = saveOptionsWidget->documentSaveOptions ();
    #if DEBUG_KP_MAIN_WINDOW
        newSaveOptions.printDebug ("\tnewSaveOptions");
    #endif

        KConfigGroupSaver cfgGroupSaver (kapp->config (), forcedSaveOptionsGroup);
        KConfigBase *cfg = cfgGroupSaver.config ();

        // Save options user forced - probably want to use them in future
        kpDocumentSaveOptions::saveDefaultDifferences (cfg,
            fdSaveOptions, newSaveOptions);
        cfg->sync ();


        if (chosenSaveOptions)
            *chosenSaveOptions = newSaveOptions;


        bool shouldAllowOverwritePrompt =
                (fd.selectedURL () != startURL ||
                 newSaveOptions.mimeType () != startSaveOptions.mimeType ());
        if (allowOverwritePrompt)
        {
            *allowOverwritePrompt = shouldAllowOverwritePrompt;
        #if DEBUG_KP_MAIN_WINDOW
            kdDebug () << "\tallowOverwritePrompt=" << *allowOverwritePrompt << endl;
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
            kdDebug () << "\tallowLossyPrompt=" << *allowLossyPrompt << endl;
        #endif
        }


    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tselectedURL=" << fd.selectedURL () << endl;
    #endif
        return fd.selectedURL ();
    }
    else
        return KURL ();
#undef SETUP_READ_CFG
}


// private slot
bool kpMainWindow::saveAs (bool localOnly)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::saveAs URL=" << m_document->url () << endl;
#endif

    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KURL chosenURL = askForSaveURL (i18n ("Save Image As"),
                                    m_document->url ().url (),
                                    m_document->pixmapWithSelection (),
                                    *m_document->saveOptions (),
                                    *m_document->metaInfo (),
                                    kpSettingsGroupFileSaveAs,
                                    localOnly,
                                    &chosenSaveOptions,
                                    !m_document->savedAtLeastOnceBefore (),
                                    &allowOverwritePrompt,
                                    &allowLossyPrompt);


    if (chosenURL.isEmpty ())
        return false;


    if (!m_document->saveAs (chosenURL, chosenSaveOptions,
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
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    return saveAs ();
}

// private slot
bool kpMainWindow::slotExport ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotExport()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KURL chosenURL = askForSaveURL (i18n ("Export"),
                                    m_lastExportURL.url (),
                                    m_document->pixmapWithSelection (),
                                    m_lastExportSaveOptions,
                                    *m_document->metaInfo (),
                                    kpSettingsGroupFileExport,
                                    false/*allow remote files*/,
                                    &chosenSaveOptions,
                                    m_exportFirstTime,
                                    &allowOverwritePrompt,
                                    &allowLossyPrompt);


    if (chosenURL.isEmpty ())
        return false;


    if (!kpDocument::savePixmapToFile (m_document->pixmapWithSelection (),
                                       chosenURL,
                                       chosenSaveOptions, *m_document->metaInfo (),
                                       allowOverwritePrompt,
                                       allowLossyPrompt,
                                       this))
    {
        return false;
    }


    addRecentURL (chosenURL);


    m_lastExportURL = chosenURL;
    m_lastExportSaveOptions = chosenSaveOptions;

    m_exportFirstTime = false;

    return true;
}


// private slot
void kpMainWindow::slotEnableReload ()
{
    m_actionReload->setEnabled (m_document);
}

// private slot
bool kpMainWindow::slotReload ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (!m_document)
        return false;


    KURL oldURL = m_document->url ();


    if (m_document->isModified ())
    {
        int result = KMessageBox::Cancel;

        if (m_document->isFromURL (false/*don't bother checking exists*/) && !oldURL.isEmpty ())
        {
            result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes since you last saved it.\n"
                               "Are you sure?")
                             .arg (m_document->prettyFilename ()),
                         QString::null/*caption*/,
                         i18n ("&Reload"));
        }
        else
        {
            result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes.\n"
                               "Are you sure?")
                             .arg (m_document->prettyFilename ()),
                         QString::null/*caption*/,
                         i18n ("&Reload"));
        }

        if (result != KMessageBox::Continue)
            return false;
    }


    kpDocument *doc = 0;

    // If it's _supposed to_ come from a URL or it exists
    if (m_document->isFromURL (false/*don't bother checking exists*/) ||
        (!oldURL.isEmpty () && KIO::NetAccess::exists (oldURL, true/*open*/, this)))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "kpMainWindow::slotReload() reloading from disk!" << endl;
    #endif

        doc = new kpDocument (1, 1, this);
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
        kdDebug () << "kpMainWindow::slotReload() create doc" << endl;
    #endif

        doc = new kpDocument (m_document->constructorWidth (),
                              m_document->constructorHeight (),
                              this);
        doc->setURL (oldURL, false/*not from URL*/);
    }


    setDocument (doc);

    return true;
}


// private
void kpMainWindow::sendFilenameToPrinter (KPrinter *printer)
{
    KURL url = m_document->url ();
    if (!url.isEmpty ())
    {
        int dot;

        QString fileName = url.fileName ();
        dot = fileName.findRev ('.');

        // file.ext but not .hidden-file?
        if (dot > 0)
            fileName.truncate (dot);

    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "kpMainWindow::sendFilenameToPrinter() fileName="
                   << fileName
                   << " dir="
                   << url.directory ()
                   << endl;
    #endif
        printer->setDocName (fileName);
        printer->setDocFileName (fileName);
        printer->setDocDirectory (url.directory ());
    }
}


static const double InchesPerMeter = 100 / 2.54;


// TODO: GUI should allow viewing & changing of DPI.


static bool shouldPrintImageCenteredOnPage ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpmainwindow_file.cpp:shouldPrintImageCenteredOnPage()" << endl;
#endif
    bool ret;

    KConfigGroupSaver cfgGroupSaver (KGlobal::config (),
                                     kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    if (cfg->hasKey (kpSettingPrintImageCenteredOnPage))
    {
        ret = cfg->readBoolEntry (kpSettingPrintImageCenteredOnPage);
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tread: " << ret << endl;
    #endif
    }
    else
    {
        ret = true;
#if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tfirst time - writing default: " << ret << endl;
#endif
        cfg->writeEntry (kpSettingPrintImageCenteredOnPage, ret);
        cfg->sync ();
    }

    return ret;
}


// private
void kpMainWindow::sendPixmapToPrinter (KPrinter *printer,
        bool showPrinterSetupDialog)
{
    // Get image to be printed.
    QPixmap pixmap = m_document->pixmapWithSelection ();


    // Get image DPI.
    double pixmapDotsPerMeterX =
        double (m_document->metaInfo ()->dotsPerMeterX ());
    double pixmapDotsPerMeterY =
        double (m_document->metaInfo ()->dotsPerMeterY ());
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::sendPixmapToPrinter() pixmap:"
               << " width=" << pixmap.width ()
               << " height=" << pixmap.height ()
               << " dotsPerMeterX=" << pixmapDotsPerMeterX
               << " dotsPerMeterY=" << pixmapDotsPerMeterY
               << endl;
#endif

    // Image DPI invalid (e.g. new image, could not read from file
    // or Qt3 doesn't implement DPI for JPEG)?
    if (pixmapDotsPerMeterX < 1 || pixmapDotsPerMeterY < 1)
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
        QPaintDeviceMetrics screenMetrics (&pixmap/*screen element*/);
        const int dpiX = screenMetrics.logicalDpiX (),
            dpiY = screenMetrics.logicalDpiY ();
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tusing screen dpi: x=" << dpiX << " y=" << dpiY << endl;
    #endif

        pixmapDotsPerMeterX = dpiX * InchesPerMeter;
        pixmapDotsPerMeterY = dpiY * InchesPerMeter;
    }


    // Get page size (excluding margins).
    // Coordinate (0,0) is the X here:
    //     mmmmm
    //     mX  m
    //     m   m       m = margin
    //     m   m
    //     mmmmm
    QPaintDeviceMetrics printerMetrics (printer);
    const int printerWidthMM = printerMetrics.widthMM ();
    const int printerHeightMM = printerMetrics.heightMM ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tprinter: widthMM=" << printerWidthMM
               << " heightMM=" << printerHeightMM
               << endl;
#endif


    double dpiX = pixmapDotsPerMeterX / InchesPerMeter;
    double dpiY = pixmapDotsPerMeterY / InchesPerMeter;
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tpixmap: dpiX=" << dpiX << " dpiY=" << dpiY << endl;
#endif


    //
    // If image doesn't fit on page at intended DPI, change the DPI.
    //

    const double scaleDpiX = (pixmap.width () / (printerWidthMM / 25.4))
        / dpiX;
    const double scaleDpiY = (pixmap.height () / (printerHeightMM / 25.4))
        / dpiY;
    const double scaleDpi = QMAX (scaleDpiX, scaleDpiY);
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\t\tscaleDpi: x=" << scaleDpiX << " y=" << scaleDpiY
               << " --> scale at " << scaleDpi << " to fit?"
               << endl;
#endif

    // Need to increase resolution to fit page?
    if (scaleDpi > 1.0)
    {
        dpiX *= scaleDpi;
        dpiY *= scaleDpi;
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\t\t\tto fit page, scaled to:"
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
        kdDebug () << "\tdpiX > dpiY; stretching pixmap height to equalise DPIs to dpiX="
                   << dpiX << endl;
    #endif
        kpPixmapFX::scale (&pixmap,
             pixmap.width (),
             QMAX (1, qRound (pixmap.height () * dpiX / dpiY)),
             false/*don't antialias*/);

        dpiY = dpiX;
    }
    else if (dpiY > dpiX)
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tdpiY > dpiX; stretching pixmap width to equalise DPIs to dpiY="
                   << dpiY << endl;
    #endif
        kpPixmapFX::scale (&pixmap,
             QMAX (1, qRound (pixmap.width () * dpiY / dpiX)),
             pixmap.height (),
             false/*don't antialias*/);

        dpiX = dpiY;
    }


    // ASSERT: dpiX == dpiY
    // QPrinter::setResolution() has to be called before QPrinter::setup().
    printer->setResolution (QMAX (1, qRound (dpiX)));


    double originX = 0, originY = 0;

    // Centre image on page?
    if (shouldPrintImageCenteredOnPage ())
    {
        originX = (printerWidthMM * dpiX / 25.4 - pixmap.width ()) / 2;
        originY = (printerHeightMM * dpiY / 25.4 - pixmap.height ()) / 2;
    }

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\torigin: x=" << originX << " y=" << originY << endl;
#endif


    sendFilenameToPrinter (printer);

    if (showPrinterSetupDialog)
    {
        // The user can mutate margins at their own risk in this dialog.
        // It doesn't seem to affect the size of the page as reported
        // by QPaintDeviceMetrics::{width,height}MM().
        if (!printer->setup (this))
            return;
    }


    // Send pixmap to printer.
    QPainter painter;
    painter.begin (printer);
    painter.drawPixmap (qRound (originX), qRound (originY), pixmap);
    painter.end ();
}


// private slot
void kpMainWindow::slotPrint ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    KPrinter printer;

    sendPixmapToPrinter (&printer, true/*showPrinterSetupDialog*/);
}

// private slot
void kpMainWindow::slotPrintPreview ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    // TODO: get it to reflect default printer's settings
    KPrinter printer (false/*separate settings from ordinary printer*/);

    // TODO: pass "this" as parent
    printer.setPreviewOnly (true);

    sendPixmapToPrinter (&printer, false/*don't showPrinterSetupDialog*/);
}


// private slot
void kpMainWindow::slotMail ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (m_document->url ().isEmpty ()/*no name*/ ||
        !m_document->isFromURL () ||
        m_document->isModified ()/*needs to be saved*/)
    {
        int result = KMessageBox::questionYesNo (this,
                        i18n ("You must save this image before sending it.\n"
                              "Do you want to save it?"),
                        QString::null,
                        KStdGuiItem::save (), KStdGuiItem::cancel ());

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

    kapp->invokeMailer (
        QString::null/*to*/,
        QString::null/*cc*/,
        QString::null/*bcc*/,
        m_document->prettyFilename()/*subject*/,
        QString::null/*body*/,
        QString::null/*messageFile*/,
        QStringList (m_document->url ().url ())/*attachments*/);
}


// private
void kpMainWindow::setAsWallpaper (bool centered)
{
    if (m_document->url ().isEmpty ()/*no name*/ ||
        !m_document->url ().isLocalFile ()/*remote file*/ ||
        !m_document->isFromURL () ||
        m_document->isModified ()/*needs to be saved*/)
    {
        QString question;

        if (!m_document->url ().isLocalFile ())
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
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::setAsWallpaper() path="
               << m_document->url ().path () << endl;
#endif
    dataStream << QString (m_document->url ().path ());

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
    if (!KApplication::dcopClient ()->send ("kdesktop", "KBackgroundIface",
                                            "setWallpaper(QString,int)", data))
    {
        KMessageBox::sorry (this, i18n ("Could not change wallpaper."));
    }
}

// private slot
void kpMainWindow::slotSetAsWallpaperCentered ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    setAsWallpaper (true/*centered*/);
}

// private slot
void kpMainWindow::slotSetAsWallpaperTiled ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    setAsWallpaper (false/*tiled*/);
}


// private slot
void kpMainWindow::slotClose ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotClose()" << endl;
#endif

    if (!queryClose ())
        return;

    setDocument (0);
}

// private slot
void kpMainWindow::slotQuit ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotQuit()" << endl;
#endif

    close ();  // will call queryClose()
}

