
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

#define DEBUG_KP_MAINWINDOW 1

#include <qdragobject.h>
#include <qscrollview.h>

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <ktoolbar.h>  // DEP thumbnail
#include <kurldrag.h>

#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kpmainwindow.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpMainWindow::kpMainWindow ()
    : KMainWindow (0/*parent*/, "mainWindow")
{
    initGUI ();
    slotNew ();

    m_alive = true;
}

kpMainWindow::kpMainWindow (const KURL &url)
    : KMainWindow (0/*parent*/, "mainWindow")
{
    initGUI ();
    slotNew (url);

    m_alive = true;
}

kpMainWindow::kpMainWindow (kpDocument *newDoc)
    : KMainWindow (0/*parent*/, "mainWindow")
{
    initGUI ();
    setDocument (newDoc);

    m_alive = true;
}

// private
void kpMainWindow::initGUI ()
{
    m_scrollView = 0;
    m_mainView = 0; m_thumbnailView = 0;
    m_document = 0;
    m_viewManager = 0;
    m_colorToolBar = 0;
    m_toolToolBar = 0;
    m_commandHistory = 0;


    //
    // set mainwindow properties
    //

    setMinimumSize (320, 260);
    setAcceptDrops (true);


    //
    // read config (TODO: clean config code, put in right place)
    //

    KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    m_configFirstTime = configGroupSaver.config ()->readBoolEntry (kpSettingFirstTime, true);
    m_configShowGrid = configGroupSaver.config ()->readBoolEntry (kpSettingShowGrid, false);
    m_configShowPath = configGroupSaver.config ()->readBoolEntry (kpSettingShowPath, false);
    m_configDefaultOutputMimetype = configGroupSaver.config ()->readEntry (kpSettingDefaultOutputMimetype, "image/png");
    if (m_configFirstTime)
    {
        configGroupSaver.config ()->writeEntry (kpSettingFirstTime, false);
        configGroupSaver.config ()->sync ();
    }
    kdDebug () << "read config: firstTime=" << m_configFirstTime
               << " showGrid=" << m_configShowGrid
               << " showPath=" << m_configShowPath
               << " outputMimeType=" << m_configDefaultOutputMimetype
               << endl;


    //
    // create GUI
    //

    setupActions ();

    createGUI ();


    //
    // create more GUI
    //

    m_colorToolBar = new kpColorToolBar (this, "Color Palette");

    setupTools ();

    m_scrollView = new QScrollView (this);
    setCentralWidget (m_scrollView);

    statusBar ()->insertItem (QString::null, StatusBarItemDocInfo, 2/*stretch*/);
    statusBar ()->insertItem (QString::null, StatusBarItemShapeEndPoints, 1/*stretch*/);
    statusBar ()->insertItem (QString::null, StatusBarItemShapeSize, 1/*stretch*/);


    //
    // set initial pos/size of GUI
    //

    setAutoSaveSettings ("kpmainwindow", true);

    // put our non-XMLGUI toolbars in a sane place, the first time around
    if (m_configFirstTime)
    {
        m_toolToolBar->setBarPos (KToolBar::Left);
    #if 0
        toolBar ("toolbar_thumbnaila")->setBarPos (KToolBar::Right);
        toolBar ("toolbar_thumbnaila")->hide ();  // it doesn't work well yet
    #endif
        m_colorToolBar->setBarPos (KToolBar::Bottom);
    }
}

kpMainWindow::~kpMainWindow ()
{
    m_alive = false;

    m_actionOpenRecent->saveEntries (kapp->config ());

    // delete document & views
    setDocument (0);

    delete m_commandHistory; m_commandHistory = 0;
    delete m_scrollView; m_scrollView = 0;
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
#if DEBUG_KP_MAINWINDOW
    kdDebug () << "kpMainWindow::setDocument (" << newDoc << ")" << endl;
#endif

    // is it a close operation?
    if (!newDoc)
    {
    #if DEBUG_KP_MAINWINDOW
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

        enableToolsDocumentActions (false);

        enableDocumentActions (false);

        m_actionReload->setEnabled (false);
    }

#if DEBUG_KP_MAINWINDOW
    kdDebug () << "\tdestroying views" << endl;
#endif

    delete m_mainView; m_mainView = 0;
    delete m_thumbnailView; m_thumbnailView = 0;

#if DEBUG_KP_MAINWINDOW
    kdDebug () << "\tdestroying viewManager" << endl;
#endif

    // viewManager will die and so will the selection
    m_actionCopy->setEnabled (false);
    m_actionCut->setEnabled (false);
    m_actionDeselect->setEnabled (false);
    m_actionDelete->setEnabled (false);

    delete m_viewManager; m_viewManager = 0;

#if DEBUG_KP_MAINWINDOW
    kdDebug () << "\tdestroying document" << endl;
    kdDebug () << "\t\tm_document=" << m_document << endl;
#endif
    // destroy current document
    delete m_document;
    m_document = newDoc;


    // not a close operation?
    if (m_document)
    {
    #if DEBUG_KP_MAINWINDOW
        kdDebug () <<"\tcreating viewManager" << endl;
    #endif
        m_viewManager = new kpViewManager (this);

        // Copy/Cut/Deselect/Delete are available only when:
        //
        // a) there is an m_document (hence why we're here)
        // b) the user has selected something
        //
        connect (m_viewManager, SIGNAL (selectionEnabled (bool)),
                 m_actionCut, SLOT (setEnabled (bool)));
        connect (m_viewManager, SIGNAL (selectionEnabled (bool)),
                 m_actionCopy, SLOT (setEnabled (bool)));
        connect (m_viewManager, SIGNAL (selectionEnabled (bool)),
                 m_actionDelete, SLOT (setEnabled (bool)));
        connect (m_viewManager, SIGNAL (selectionEnabled (bool)),
                 m_actionDeselect, SLOT (setEnabled (bool)));

        // this code won't actually enable any actions at this stage
        // (fresh viewManager) but better safe than sorry
        m_actionCopy->setEnabled (m_viewManager->selectionActive ());
        m_actionCut->setEnabled (m_viewManager->selectionActive ());
        m_actionDeselect->setEnabled (m_viewManager->selectionActive ());
        m_actionDelete->setEnabled (m_viewManager->selectionActive ());

    #if DEBUG_KP_MAINWINDOW
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

    #if 0

    #if DEBUG_KP_MAINWINDOW
        kdDebug () << "\tcreating thumbnail view" << endl;
    #endif

        // TODO: change to a child window
        toolBar ("toolbar_thumbnaila")->setFullSize ();
        m_thumbnailView = new kpView (toolBar ("toolbar_thumbnaila"),
                                      "thumbnailView", this,
                                      200, 200, true /* autoVariableZoom */);
        toolBar ("toolbar_thumbnaila")->insertWidget (2000,
            toolBar ("toolbar_thumbnaila")->width (), m_thumbnailView);

        toolBar ("toolbar_thumbnaila")->setFullSize ();

        m_viewManager->registerView (m_thumbnailView);
    #endif

    #if DEBUG_KP_MAINWINDOW
        kdDebug () << "\thooking up document signals" << endl;
    #endif

        // status bar
        connect (m_document, SIGNAL (documentOpened ()),
                 this, SLOT (slotUpdateStatusBar ()));
        connect (m_document, SIGNAL (sizeChanged (int, int)),
                 this, SLOT (slotUpdateStatusBar (int, int)));
        connect (m_document, SIGNAL (colorDepthChanged (int)),
                 this, SLOT (slotUpdateStatusBar (int)));

        // caption (url, modified)
        connect (m_document, SIGNAL (documentModified ()),
                 this, SLOT (slotUpdateCaption ()));
        connect (m_document, SIGNAL (documentOpened ()),
                 this, SLOT (slotUpdateCaption ()));
        connect (m_document, SIGNAL (documentSaved ()),
                 this, SLOT (slotUpdateCaption ()));

        // File/Reload action only available with non-empty URL
        connect (m_document, SIGNAL (documentSaved ()),
                 this, SLOT (slotEnableReload ()));
        slotEnableReload ();  // will check for non-empty URL

        // command history
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

        // sync document -> views
        connect (m_document, SIGNAL (contentsChanged (const QRect &)),
                 m_viewManager, SLOT (updateViews (const QRect &)));
        connect (m_document, SIGNAL (sizeChanged (int, int)),
                 m_viewManager, SLOT (resizeViews (int, int)));
        connect (m_document, SIGNAL (colorDepthChanged (int)),
                 m_viewManager, SLOT (updateViews ()));

    #if DEBUG_KP_MAINWINDOW
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

        enableToolsDocumentActions (true);

        enableDocumentActions (true);
    }

#if DEBUG_KP_MAINWINDOW
    kdDebug () << "\tupdating mainWindow elements" << endl;
#endif

    slotUpdateStatusBar ();  // update doc size
    slotUpdateCaption ();  // Untitled to start with

    if (m_commandHistory)
        m_commandHistory->clear ();

#if DEBUG_KP_MAINWINDOW
    kdDebug () << "\tdocument and views ready to go!" << endl;
#endif
}


// private virtual [base KMainWindow]
bool kpMainWindow::queryClose ()
{
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
    e->accept (QImageDrag::canDecode (e) || KURLDrag::canDecode (e));
}

// private virtual [base QWidget]
void kpMainWindow::dropEvent (QDropEvent *e)
{
    QPixmap pixmap;
    KURL::List urls;

    if (QImageDrag::decode (e, pixmap))
    {
        if (!pixmap.isNull ())
        {
            m_viewManager->setTempPixmapAt (pixmap, QPoint (0, 0), kpViewManager::SelectionPixmap);
            slotToolRectSelection ();
        }
    }
    else if (KURLDrag::decode (e, urls))
    {
        for (KURL::List::ConstIterator it = urls.begin (); it != urls.end (); it++)
        {
            open (*it);
        }
    }
}


/*
 * Status Bar
 */
// TODO: clean up

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


// private
// this prevents coords like "-2,400" from being shown
bool kpMainWindow::legalDocPoint (const QPoint &point) const
{
   return (m_document &&
           point.x () >= 0 && point.x () < m_document->width () &&
           point.y () >= 0 && point.y () < m_document->height ());
}

// private slot
void kpMainWindow::slotUpdateStatusBar ()
{
    if (m_document)
        slotUpdateStatusBar (m_document->width (), m_document->height (), m_document->colorDepth ());
    else
        statusBar ()->changeItem (QString::null, StatusBarItemDocInfo);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (int docWidth, int docHeight)
{
    if (m_document && docWidth > 0 && docHeight > 0)
        slotUpdateStatusBar (docWidth, docHeight, m_document->colorDepth ());
    else
        statusBar ()->changeItem (QString::null, StatusBarItemDocInfo);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (int docColorDepth)
{
    if (m_document && docColorDepth > 0)
        slotUpdateStatusBar (m_document->width (), m_document->height (), docColorDepth);
    else
        statusBar ()->changeItem (QString::null, StatusBarItemDocInfo);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (int docWidth, int docHeight, int docColorDepth)
{
#if DEBUG_KP_MAINWINDOW
    kdDebug () << "kpMainWindow::slotUpdateStatusBar ("
               << docWidth << "x" << docHeight << "@" << docColorDepth << endl;
#endif

    if (m_document && docWidth > 0 && docHeight > 0 && docColorDepth > 0)
    {
        statusBar ()->changeItem (i18n ("%1 x %2  (%3-bit)")
                                    .arg (docWidth).arg (docHeight)
                                    .arg (docColorDepth),
                                  StatusBarItemDocInfo);
    }
    else
        statusBar ()->changeItem (QString::null, StatusBarItemDocInfo);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (const QPoint &point)
{
    QString string;

    // we just don't display illegal points full stop
    if (legalDocPoint (point))
        string = i18n ("%1,%2").arg (point.x ()).arg (point.y ());

    statusBar ()->changeItem (string, StatusBarItemShapeEndPoints);
    statusBar ()->changeItem (QString::null, StatusBarItemShapeSize);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (const QRect &srect)
{
    int x1 = srect.left (), y1 = srect.top (), x2 = srect.right (), y2 = srect.bottom ();

    // we clip illegal points when dragging
    x1 = QMIN (QMAX (x1, 0), m_document->width () - 1);
    x2 = QMIN (QMAX (x2, 0), m_document->width () - 1);
    y1 = QMIN (QMAX (y1, 0), m_document->height () - 1);
    y2 = QMIN (QMAX (y2, 0), m_document->height () - 1);

    QRect rect = QRect (QPoint (x1, y1), QPoint (x2, y2));

    statusBar ()->changeItem (i18n ("%1,%2 - %3,%4")
                                    .arg (rect.left ())
                                    .arg (rect.top ())
                                    .arg (rect.right ())
                                    .arg (rect.bottom ()),
                                StatusBarItemShapeEndPoints);
    statusBar ()->changeItem (i18n ("%1x%2")
                                    .arg (rect.width ())
                                    .arg (rect.height ()),
                                StatusBarItemShapeSize);
}

#include <kpmainwindow.moc>
