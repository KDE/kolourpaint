
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


#include <kpmainwindow.h>

#include <qcstring.h>
#include <qdatastream.h>
#include <qpainter.h>
#include <qsize.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kstdaccel.h>
#include <kstdaction.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kptool.h>
#include <kpview.h>
#include <kpviewmanager.h>


// private
void kpMainWindow::setupFileMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionNew = KStdAction::openNew (this, SLOT (slotNew ()), ac);
    m_actionOpen = KStdAction::open (this, SLOT (slotOpen ()), ac);

    m_actionOpenRecent = KStdAction::openRecent (this, SLOT (slotOpenRecent (const KURL &)), ac);
    m_actionOpenRecent->loadEntries (kapp->config ());

    m_actionSave = KStdAction::save (this, SLOT (slotSave ()), ac);
    m_actionSaveAs = KStdAction::saveAs (this, SLOT (slotSaveAs ()), ac);

    d->m_actionExport = new KAction (i18n ("E&xport..."), 0,
        this, SLOT (slotExport ()), ac, "file_export");

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

    d->m_actionExport->setEnabled (enable);

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
bool kpMainWindow::shouldOpenInNewWindow () const
{
    return (m_document && !m_document->isEmpty ());
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
                mw->setRecentURLs (m_actionOpenRecent->items ());
        }
    }
}

// private
void kpMainWindow::setRecentURLs (const QStringList &items)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow(" << name () << ")::setRecentURLs()" << endl;
    kdDebug () << "\titems=" << items << endl;
#endif
    m_actionOpenRecent->setItems (items);
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
        docSize = QSize (400, 300);

    return docSize;
}

// private
void kpMainWindow::saveDefaultDocSize (const QSize &size)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tCONFIG: saving Last Doc Size = "
               << QSize (dialog.imageWidth (), dialog.imageHeight ())
               << endl;
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingLastDocSize, size);
    cfg->sync ();
}


// private
bool kpMainWindow::open (const KURL &url, bool newDocSameNameIfNotExist)
{
    QSize docSize = defaultDocSize ();

    // create doc
    kpDocument *newDoc = new kpDocument (docSize.width (), docSize.height (), 32, this);
    if (newDoc->open (url, newDocSameNameIfNotExist))
    {
        if (newDoc->isFromURL (false/*don't bother checking exists*/))
            addRecentURL (url);
    }
    else
    {
        delete newDoc;
        return false;
    }

    // need new window?
    if (shouldOpenInNewWindow ())
    {
        // send doc to new window
        kpMainWindow *win = new kpMainWindow (newDoc);
        win->show ();
    }
    else
    {
        // set up views, doc signals
        setDocument (newDoc);
    }

    return true;
}

// private
KURL::List kpMainWindow::askForOpenURLs (const QString &caption, const QString &startURL,
                                         bool allowMultipleURLs)
{
    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Reading);
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
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    open (url);
}


// private slot
bool kpMainWindow::save (bool localOnly)
{
    if (m_document->url ().isEmpty () ||
        KImageIO::mimeTypes (KImageIO::Writing).findIndex (m_document->mimetype ()) < 0 ||
        (localOnly && !m_document->url ().isLocalFile ()))
    {
        return saveAs (localOnly);
    }
    else
    {
        if (m_document->save ())
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
                                  const QString &startMimeType,
                                  const char *lastOutputMimeTypeSettingsPrefix,
                                  bool localOnly,
                                  QString &chosenMimeType)
{
    chosenMimeType = QString::null;


    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Writing);
    if (mimeTypes.isEmpty ())
    {
        kdError () << "No KImageIO output mimetypes!" << endl;
        return KURL ();
    }


    KFileDialog fd (startURL, QString::null, this, "fd", true/*modal*/);
    fd.setCaption (caption);
    fd.setOperationMode (KFileDialog::Saving);
    if (localOnly)
        fd.setMode (KFile::File | KFile::LocalOnly);


    QString defaultMimeType;

    // use the current mimetype of the document (if available)
    //
    // this is so as to not stuff up users who are just changing the filename
    // but want to save in the same type
    if (!startMimeType.isEmpty () && mimeTypes.findIndex (startMimeType) > -1)
        defaultMimeType = startMimeType;

    if (defaultMimeType.isEmpty ())
    {
        // KConfig::readEntry() does not actually reread from disk, hence doesn't
        // realise what other processes have done e.g. Settings / Show Path
        kapp->config ()->reparseConfiguration ();

        KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
        KConfigBase *cfg = cfgGroupSaver.config ();


        const QString lastOutputMimeTypeEntry =
            QString::fromLatin1 (lastOutputMimeTypeSettingsPrefix) +
            QString::fromLatin1 (" ") +
            kpSettingLastOutputMimeType;

        QString lastOutputMimeType = cfg->readEntry (lastOutputMimeTypeEntry,
                                                     QString::fromLatin1 ("image/png"));


        if (mimeTypes.findIndex (lastOutputMimeType) > -1)
            defaultMimeType = lastOutputMimeType;
        else if (mimeTypes.findIndex ("image/png") > -1)
            defaultMimeType = "image/png";
        else if (mimeTypes.findIndex ("image/x-bmp") > -1)
            defaultMimeType = "image/x-bmp";
        else
            defaultMimeType = mimeTypes.first ();
    }

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "mimeTypes=" << mimeTypes << endl;
#endif
    fd.setMimeFilter (mimeTypes, defaultMimeType);


    if (fd.exec ())
    {
        chosenMimeType = fd.currentMimeFilter ();
        return fd.selectedURL ();
    }
    else
        return KURL ();
}

// private
void kpMainWindow::saveLastOutputMimeType (const QString &mimeType,
                                           const char *lastOutputMimeTypeSettingsPrefix)
{

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "\tCONFIG: saveLastOutputMimeType(" << mimeType
               << "," << lastOutputMimeType
               << ")"
               << endl;
#endif
    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    const QString lastOutputMimeTypeEntry =
        QString::fromLatin1 (lastOutputMimeTypeSettingsPrefix) +
        QString::fromLatin1 (" ") +
        kpSettingLastOutputMimeType;

    cfg->writeEntry (lastOutputMimeTypeEntry, mimeType);
    cfg->sync ();
}


// private slot
bool kpMainWindow::saveAs (bool localOnly)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::saveAs URL=" << m_document->url () << endl;
#endif

    QString chosenMimeType;
    KURL chosenURL = askForSaveURL (i18n ("Save Image As"),
                                    m_document->url ().url (),
                                    m_document->mimetype (),
                                    "File/Save As",
                                    localOnly,
                                    chosenMimeType/*ref*/);


    if (chosenURL.isEmpty () || chosenMimeType.isEmpty ())
        return false;


    // user forced a mimetype (as opposed to selecting the same type as the current doc)
    // - probably wants to use it in the future
    if (chosenMimeType != m_document->mimetype ())
        saveLastOutputMimeType (chosenMimeType, "File/Save As");


    if (!m_document->saveAs (chosenURL, chosenMimeType))
        return false;


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


    QString chosenMimeType;
    KURL chosenURL = askForSaveURL (i18n ("Export"),
                                    d->m_lastExportURL.url (),
                                    d->m_lastExportMimeType,
                                    "File/Export",
                                    false/*allow remote files*/,
                                    chosenMimeType/*ref*/);


    if (chosenURL.isEmpty () || chosenMimeType.isEmpty ())
        return false;


    d->m_lastExportURL = chosenURL;
    d->m_lastExportMimeType = chosenMimeType;
    saveLastOutputMimeType (chosenMimeType, "File/Export");


    if (!kpDocument::savePixmapToFile (m_document->pixmapWithSelection (),
                                       chosenURL, chosenMimeType,
                                       true/*overwrite prompt*/,
                                       this))
    {
        return false;
    }


    addRecentURL (chosenURL);

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

        doc = new kpDocument (1, 1, 32, this);
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
                              32,
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

// private
void kpMainWindow::sendPixmapToPrinter (KPrinter *printer)
{
    QPainter painter;
    painter.begin (printer);
    painter.drawPixmap (0, 0, m_document->pixmapWithSelection ());
    painter.end ();
}


// private slot
void kpMainWindow::slotPrint ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    KPrinter printer;

    sendFilenameToPrinter (&printer);
    if (!printer.setup (this))
        return;

    sendPixmapToPrinter (&printer);
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
    sendFilenameToPrinter (&printer);

    sendPixmapToPrinter (&printer);
}


// private slot
void kpMainWindow::slotMail ()
{
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
