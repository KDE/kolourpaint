
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

// TODO: clean up kpMainWindow class

#define KP_STATUSBAR_ITEM_DOC 1111
#define KP_STATUSBAR_ITEM_POS 1234
#define KP_STATUSBAR_ITEM_SIZE 4321


kpMainWindow::kpMainWindow (const KURL &url)
    : KMainWindow (0, "kpMainWindow"),
      m_mainView (0), m_document (0), m_viewManager (0)
{
    setMinimumSize (80, 60);
    setAcceptDrops (true);

    // read config
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

    setupActions ();

    createGUI ();

    m_colorToolBar = new kpColorToolBar (this, "Color Palette");

    m_scrollView = new QScrollView (this);
    setCentralWidget (m_scrollView);


    m_viewManager = new kpViewManager (this);
    connect (m_viewManager, SIGNAL (selectionEnabled (bool)), m_actionCopy, SLOT (setEnabled (bool)));
    connect (m_viewManager, SIGNAL (selectionEnabled (bool)), m_actionCut, SLOT (setEnabled (bool)));
    connect (m_viewManager, SIGNAL (selectionEnabled (bool)), m_actionDeselect, SLOT (setEnabled (bool)));

    setupTools ();
    // create initial document
    slotNew (url);

    // TODO: change to a child window
    toolBar ("toolbar_thumbnaila")->setFullSize ();
    m_thumbnailView = new kpView (toolBar ("toolbar_thumbnaila"), "thumbnailView", this, 200, 200, true /* autoVariableZoom */);
    m_viewManager->registerView (m_thumbnailView);    // TODO: unregister
    toolBar ("toolbar_thumbnaila")->insertWidget (2000, toolBar ("toolbar_thumbnaila")->width (), m_thumbnailView);
    toolBar ("toolbar_thumbnaila")->setFullSize ();

    statusBar ()->insertItem (QString::null, KP_STATUSBAR_ITEM_DOC, 0/*stretch*/);
    statusBar ()->insertItem (QString::null, KP_STATUSBAR_ITEM_POS, 1/*stretch*/, true/*permanent*/);
    statusBar ()->insertItem (QString::null, KP_STATUSBAR_ITEM_SIZE, 1/*stretch*/, true/*permanent*/);

    setAutoSaveSettings ("kpmainwindow", true);
 
    // put our non-XMLGUI toolbars in a sane place, the first time around
    if (m_configFirstTime)
    {
        m_toolToolBar->setBarPos (KToolBar::Left);
        toolBar ("toolbar_thumbnaila")->setBarPos (KToolBar::Right);
        toolBar ("toolbar_thumbnaila")->hide ();  // it doesn't work well yet
        m_colorToolBar->setBarPos (KToolBar::Bottom);
    }
}

kpMainWindow::~kpMainWindow ()
{
    m_actionOpenRecent->saveEntries (kapp->config ());

    delete m_commandHistory;

    delete m_thumbnailView;
    delete m_mainView;
    delete m_document;
    delete m_scrollView;
    delete m_viewManager;
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
KCommandHistory *kpMainWindow::commandHistory () const
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
void kpMainWindow::setMainView (kpView *view)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::setMainView (" << (int) view << ")" << endl;
#endif

    m_viewManager->unregisterView (m_mainView);
    delete m_mainView;

    m_mainView = view;
    if (m_mainView)
    {
        m_scrollView->addChild (m_mainView);
        m_viewManager->registerView (m_mainView);
        m_mainView->show ();

        m_actionSave->setEnabled (true);
        m_actionSaveAs->setEnabled (true);
        m_actionSaveAs->setEnabled (true);
        m_actionRevert->setEnabled (true);
        m_actionPrint->setEnabled (true);
        m_actionPrintPreview->setEnabled (true);
        m_actionClose->setEnabled (true);

        m_actionZoomIn->setEnabled (true);
        m_actionZoomOut->setEnabled (false);
        m_actionZoom->setEnabled (true);
        m_actionShowGrid->setEnabled (false);
    }
    else
    {
        m_actionSave->setEnabled (false);
        m_actionSaveAs->setEnabled (false);
        m_actionSaveAs->setEnabled (false);
        m_actionRevert->setEnabled (false);
        m_actionPrint->setEnabled (false);
        m_actionPrintPreview->setEnabled (false);
        m_actionClose->setEnabled (false);

        m_actionZoomIn->setEnabled (false);
        m_actionZoomOut->setEnabled (false);
        m_actionZoom->setEnabled (false);
        m_actionShowGrid->setEnabled (false);
    }
}


// private virtual [base KMainWindow]
bool kpMainWindow::queryClose ()
{
    if (!m_document || !m_document->isModified ())
        return true;  // ok to close current doc

    int result = KMessageBox::warningYesNoCancel (this,
                     i18n ("The document \"%1\" has been modified.\n"
                           "Do you want to save it?")
                         .arg (m_document->filename ()),
                    QString::null,
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


// private slot
void kpMainWindow::slotUpdateCaption ()
{
    setCaption (m_configShowPath ? m_document->prettyURL () : m_document->filename (),
                m_document->isModified ());
}


// private
// this prevents coords like "-2,400" from being shown
bool kpMainWindow::legalDocPoint (const QPoint &point) const
{
   return (point.x () >= 0 && point.x () < m_document->width () &&
           point.y () >= 0 && point.y () < m_document->height ());
}

// private slot
void kpMainWindow::slotUpdateStatusBar ()
{
    if (m_document)
        slotUpdateStatusBar (m_document->width (), m_document->height (), m_document->colorDepth ());
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (int docWidth, int docHeight)
{
    if (m_document && docWidth > 0 && docHeight > 0)
        slotUpdateStatusBar (docWidth, docHeight, m_document->colorDepth ());
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (int docColorDepth)
{
    if (m_document && docColorDepth > 0)
        slotUpdateStatusBar (m_document->width (), m_document->height (), docColorDepth);
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (int docWidth, int docHeight, int docColorDepth)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotUpdateStatusBar ("
               << docWidth << "x" << docHeight << "@" << docColorDepth << endl;
#endif

    if (m_document && docWidth > 0 && docHeight > 0 && docColorDepth > 0)
        statusBar ()->changeItem (i18n ("%1 x %2  (%3-bit)")
                                    .arg (docWidth).arg (docHeight)
                                    .arg (docColorDepth),
                                  KP_STATUSBAR_ITEM_DOC);
    else
        statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_DOC);
}

// private slot
void kpMainWindow::slotUpdateStatusBar (const QPoint &point)
{
    QString string;

    // we just don't display illegal points full stop
    if (legalDocPoint (point))
        string = i18n ("%1,%2").arg (point.x ()).arg (point.y ());

    statusBar ()->changeItem (string, KP_STATUSBAR_ITEM_POS);
    statusBar ()->changeItem (QString::null, KP_STATUSBAR_ITEM_SIZE);
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
                                KP_STATUSBAR_ITEM_POS);
    statusBar ()->changeItem (i18n ("%1x%2")
                                    .arg (rect.width ())
                                    .arg (rect.height ()),
                                KP_STATUSBAR_ITEM_SIZE);
}

#include <kpmainwindow.moc>
