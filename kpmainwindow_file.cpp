
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


#include <qcstring.h>
#include <qdatastream.h>
#include <qpainter.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kaction.h>
#include <kcommand.h>
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
#include <kpmainwindow.h>
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
    //m_actionRevert = KStdAction::revert (this, SLOT (slotRevert ()), ac);
    m_actionReload = new KAction (i18n ("Reloa&d"), KStdAccel::reload (),
        this, SLOT (slotReload ()), ac, "file_revert");

    m_actionPrint = KStdAction::print (this, SLOT (slotPrint ()), ac);
    m_actionPrintPreview = KStdAction::printPreview (this, SLOT (slotPrintPreview ()), ac);

    m_actionMail = KStdAction::mail (this, SLOT (slotMail ()), ac);

    m_actionSetAsWallpaperCentered = new KAction (i18n ("Set As Wa&llpaper (Centered)"), 0,
        this, SLOT (slotSetAsWallpaperCentered ()), ac, "file_set_as_wallpaper_centered");
    m_actionSetAsWallpaperTiled = new KAction (i18n ("Set As &Wallpaper (Tiled)"), 0,
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
    if (!url.isEmpty ())
        m_actionOpenRecent->addURL (url);
}


// private slot
void kpMainWindow::slotNew (const KURL &url)
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    open (url, true/*create an empty doc with the same url if url !exist*/);
}

// private slot
bool kpMainWindow::open (const KURL &url, bool newDocSameNameIfNotExist)
{
    // create doc
    kpDocument *newDoc = new kpDocument (400, 300, 32, this);
    if (newDoc->open (url, newDocSameNameIfNotExist))
        addRecentURL (url);
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

// private slot
void kpMainWindow::slotOpen ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    QString startURL = m_document ? m_document->url ().url () : QString::null;
    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Reading);
    QString filter = mimeTypes.join (" ");

    KFileDialog fd (startURL, filter, this, "fd", true/*modal*/);
    fd.setCaption (i18n ("Open Image"));
    fd.setOperationMode (KFileDialog::Opening);
    fd.setMode (KFile::Files);
    fd.setPreviewWidget (new KImageFilePreview (&fd));

    if (fd.exec ())
    {
        KURL::List urls = fd.selectedURLs ();

        for (KURL::List::ConstIterator it = urls.begin ();
             it != urls.end ();
             it++)
        {
            open (*it);
        }
    }
}

// private slot
void kpMainWindow::slotOpenRecent (const KURL &url)
{
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


// private slot
bool kpMainWindow::saveAs (bool localOnly)
{
    QStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Writing);
    if (mimeTypes.isEmpty ())
    {
        kdError () << "No KImageIO output mimetypes!" << endl;
        return false;
    }

    KURL url = m_document->url ();
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::saveAs URL=" << url << endl;
#endif
    KFileDialog fd (url.url (), QString::null, this, "fd", true/*modal*/);
    fd.setCaption (i18n ("Save Image As"));
    fd.setOperationMode (KFileDialog::Saving);
    if (localOnly)
        fd.setMode (KFile::File | KFile::LocalOnly);

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

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "mimeTypes=" << mimeTypes << endl;
#endif
    fd.setMimeFilter (mimeTypes, defaultMimeType);
    if (fd.exec ())
    {
        QString mimetype = fd.currentMimeFilter ();
        if (!m_document->saveAs (fd.selectedURL (), mimetype))
        {
            return false;
        }
        else
        {
            addRecentURL (fd.selectedURL ());

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
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "fd aborted" << endl;
    #endif
        return false;
    }

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
void kpMainWindow::slotEnableReload ()
{
    m_actionReload->setEnabled (m_document && !m_document->url ().isEmpty ());
}

// private slot
bool kpMainWindow::slotReload ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (m_document && m_document->isModified ())
    {
        int result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes since you last saved it.\n"
                               "Are you sure?")
                             .arg (m_document->prettyFilename ()),
                        QString::null/*caption*/,
                        i18n ("&Reload"));

        if (result != KMessageBox::Continue)
            return false;
    }

    KURL oldURL = m_document->url ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotReload() reloading!" << endl;
#endif
    setDocument (0);  // make sure we don't open in a new window

    return open (oldURL);
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

        kdDebug () << "kpMainWindow::sendFilenameToPrinter() fileName="
                   << fileName
                   << " dir="
                   << url.directory ()
                   << endl;
        printer->setDocName (fileName);
        printer->setDocFileName (fileName);  // TODO: wrong if filename has space!
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
    if (m_document->url ().isEmpty () ||  // no name
        m_document->isModified ())        // hasn't been saved
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
    if (!m_document->url ().isLocalFile () ||  // remote file
        m_document->url ().isEmpty ()      ||  // no name
        m_document->isModified ())             // hasn't been saved
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
