
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
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kstdaction.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
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


// private slot
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

    m_document = new kpDocument (400, 300, 32, this);
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

    setMainView (new kpView (m_scrollView->viewport (), "mainView", this,
                             m_document->width (), m_document->height ()));
    m_viewManager->resizeViews (m_document->width (), m_document->height ());

    slotUpdateStatusBar ();  // update doc size
    slotUpdateCaption ();  // Untitled to start with
    m_commandHistory->clear ();
}

// private slot
void kpMainWindow::slotDocumentRestored ()
{
    m_document->setModified (false);
    slotUpdateCaption ();
}


// private slot
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

// private slot
bool kpMainWindow::open (const QString &str, bool newDocSameNameIfNotExist)
{
    KURL url;
    url.setPath (str);

    return open (url, newDocSameNameIfNotExist);
}

// private slot
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

// private slot
bool kpMainWindow::slotOpenRecent (const KURL &url)
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    /*if (!queryClose ())
        return false;*/

    return open (url);
}


// private slot
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

// private slot
bool kpMainWindow::slotSave ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

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

// private slot
bool kpMainWindow::slotSaveAs ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;

    return saveAs ();
}


// private slot
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
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    KPrinter printer;

    sendFilenameToPrinter (&printer);
    if (!printer.setup (this))
        return;

    sendPixmapToPrinter (&printer);
}

// private slots
void kpMainWindow::slotPrintPreview ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    // TODO: get it to reflect default printer's settings
    KPrinter printer (false/*separate settings from ordinary printer*/);
    
    printer.setPreviewOnly (true);
    sendFilenameToPrinter (&printer);
    
    sendPixmapToPrinter (&printer);
}


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

// private slot
void kpMainWindow::slotSetAsWallpaperCentered ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    setAsWallpaper (true/*centered*/);
}

// private slot
void kpMainWindow::slotSetAsWallpaperTiled ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    setAsWallpaper (false/*tiled*/);
}


// private slot
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

// private slot
void kpMainWindow::slotQuit ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotQuit()" << endl;
#endif

    close ();  // will call queryClose()
}
