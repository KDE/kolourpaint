
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

#include <qscrollview.h>

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>  // DEP
#include <kstdaction.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpview.h>


// private
void kpMainWindow::setupViewMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    /*m_actionFullScreen = KStdAction::fullScreen (0, 0, ac);
    m_actionFullScreen->setEnabled (false);*/


    m_actionActualSize = KStdAction::actualSize (this, SLOT (slotActualSize ()), ac);
    /*m_actionFitToPage = KStdAction::fitToPage (this, SLOT (slotFitToPage ()), ac);
    m_actionFitToWidth = KStdAction::fitToWidth (this, SLOT (slotFitToWidth ()), ac);
    m_actionFitToHeight = KStdAction::fitToHeight (this, SLOT (slotFitToHeight ()), ac);*/


    m_actionZoomIn = KStdAction::zoomIn (this, SLOT (slotZoomIn ()), ac);
    m_actionZoomOut = KStdAction::zoomOut (this, SLOT (slotZoomOut ()), ac);


    m_actionZoom = new KSelectAction (i18n ("&Zoom"), 0,
        this, SLOT (slotZoom ()), actionCollection (), "view_zoom");
    m_actionZoom->setEditable (true);

    // create the zoom list for the 1st call to zoomTo() below
    m_zoomList.append (10); m_zoomList.append (25); m_zoomList.append (33);
    m_zoomList.append (50); m_zoomList.append (67); m_zoomList.append (75);
    m_zoomList.append (100);
    m_zoomList.append (200); m_zoomList.append (300);
    m_zoomList.append (400); m_zoomList.append (600); m_zoomList.append (800);
    m_zoomList.append (1000); m_zoomList.append (1200); m_zoomList.append (1600);


    m_actionShowGrid = new KToggleAction (i18n ("Show &Grid"), CTRL + Key_G,
        this, SLOT (slotShowGrid ()), actionCollection (), "view_show_grid");
    m_actionShowGrid->setChecked (m_configShowGrid);
    connect (m_actionShowGrid, SIGNAL (toggled (bool)), this, SLOT (slotActionShowGridToggled (bool)));

    enableViewMenuDocumentActions (false);
}
    
// private
void kpMainWindow::enableViewMenuDocumentActions (bool enable)
{
    m_actionActualSize->setEnabled (enable);
    /*m_actionFitToPage->setEnabled (enable);
    m_actionFitToWidth->setEnabled (enable);
    m_actionFitToHeight->setEnabled (enable);*/
    
    m_actionZoomIn->setEnabled (enable);
    m_actionZoomOut->setEnabled (enable);
    
    m_actionZoom->setEnabled (enable);
    
    m_actionShowGrid->setEnabled (enable);

    // TODO: for the time being, assume that we start at zoom 100%
    //       with no grid
    
    // This function is only called when a new document is created
    // or an existing document is closed.  So the following will
    // always be correct:
    
    zoomTo (100);
}


// private
void kpMainWindow::sendZoomListToActionZoom ()
{
    QStringList items;
    
    const QValueVector <int>::ConstIterator zoomListEnd (m_zoomList.end ());
    for (QValueVector <int>::ConstIterator it = m_zoomList.begin ();
         it != zoomListEnd;
         it++)
    {
        items << zoomLevelToString (*it);
    }

    m_actionZoom->setItems (items);
}

// private
int kpMainWindow::zoomLevelFromString (const QString &string)
{
    // loop until not digit
    int i;
    for (i = 0; i < (int) string.length () && string.at (i).isDigit (); i++)
        ;
    
    // convert zoom level to number
    bool ok = false;
    int zoomLevel = string.left (i).toInt (&ok);

    if (!ok || zoomLevel <= 0 || zoomLevel > 3200)
        return 0;  // error
    else    
        return zoomLevel;
}

// private
QString kpMainWindow::zoomLevelToString (int zoomLevel)
{
    return i18n ("%1%").arg (zoomLevel);
}

// private
void kpMainWindow::zoomTo (int zoomLevel)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::zoomTo (" << zoomLevel << ")" << endl;
#endif
    
    if (zoomLevel <= 0)
        zoomLevel = m_mainView ? m_mainView->zoomLevelX () : 100;
    else if (m_mainView && m_mainView->zoomLevelX () % 100 == 0 && zoomLevel % 100)
    {
        if (KMessageBox::warningContinueCancel (this,
            i18n ("Setting the Zoom Level to a value that is not a multiple of 100% "
                  "results in imprecise editing and redraw glitches.\n"
                  "Do you really want to set to Zoom Level to %1%?")
                .arg (zoomLevel),
            QString::null/*caption*/,
            i18n ("Set Zoom Level to %1%").arg (zoomLevel),
            "DoNotAskAgain_ZoomLevelNotMultipleOf100") != KMessageBox::Continue)
        {
            zoomLevel = m_mainView->zoomLevelX ();
        }
    }

    int index = 0;
    QValueVector <int>::Iterator it = m_zoomList.begin ();
    
    while (index < (int) m_zoomList.count () && zoomLevel > *it)
        it++, index++;
        
    if (zoomLevel != *it)
        m_zoomList.insert (it, zoomLevel);

    sendZoomListToActionZoom ();
    m_actionZoom->setCurrentItem (index);


    m_actionActualSize->setEnabled (zoomLevel != 100);

    m_actionZoomIn->setEnabled (m_actionZoom->currentItem () < (int) m_zoomList.count () - 1);
    m_actionZoomOut->setEnabled (m_actionZoom->currentItem () > 0);

    if (m_scrollView && m_mainView)
    {
    #if 1
        #if DEBUG_KPMAINWINDOW
            kdDebug () << "\tscrollView   contentsX=" << m_scrollView->contentsX ()
                    << " contentsY=" << m_scrollView->contentsY ()
                    << " contentsWidth=" << m_scrollView->contentsWidth ()
                    << " contentsHeight=" << m_scrollView->contentsHeight ()
                    << " visibleWidth=" << m_scrollView->visibleWidth ()
                    << " visibleHeight=" << m_scrollView->visibleHeight ()
                    << " oldZoomX=" << m_mainView->zoomLevelX ()
                    << " oldZoomY=" << m_mainView->zoomLevelY ()
                    << " newZoom=" << zoomLevel
                    << " mainViewX=" << m_scrollView->childX (m_mainView)
                    << " mainViewY=" << m_scrollView->childY (m_mainView)
                    << endl;
        #endif
            int newPosY = m_scrollView->contentsY () * zoomLevel / m_mainView->zoomLevelY ();
            int newPosX = m_scrollView->contentsX () * zoomLevel / m_mainView->zoomLevelX ();
    #endif
    
        m_mainView->setZoomLevel (zoomLevel, zoomLevel);
    
    #if 1
        #if DEBUG_KPMAINWINDOW && 1
            kdDebug () << "\tcurrently at (x=" << m_scrollView->contentsX ()
                    << ",y=" << m_scrollView->contentsY ()
                    << ") scrolling to (x=" << newPosX
                    << ",y=" << newPosY << ")" << endl;
        #endif
        
            m_scrollView->setContentsPos (newPosX, newPosY);
    
        #if DEBUG_KPMAINWINDOW && 1
            kdDebug () << "\t\tcheck (contentsX=" << m_scrollView->contentsX ()
                    << ",contentsY=" << m_scrollView->contentsY ()
                    << ")" << endl;
        #endif
    #endif
    }

    if (m_mainView)
    {    
        m_actionShowGrid->setEnabled (m_mainView->canShowGrid ());
        if (m_actionShowGrid->isEnabled ())
        {
            m_actionShowGrid->setChecked (m_configShowGrid);
            slotShowGrid ();
        }
        else
        {
            m_actionShowGrid->setChecked (false);
        }

        // Since Zoom Level KSelectAction on ToolBar grabs focus after changing
        // Zoom, switch back to the Main View.
        // TODO: back to the last view
        m_mainView->setFocus ();
    }
}


// private slot
void kpMainWindow::slotActualSize ()
{
    zoomTo (100);
}

// private slot
void kpMainWindow::slotFitToPage ()
{
    if (!m_scrollView || !m_document)
        return;

    // doc_width * zoom / 100 <= view_width &&
    // doc_height * zoom / 100 <= view_height &&
    // 1 <= zoom <= 3200
    
    zoomTo (QMIN (3200, QMAX (1, QMIN (m_scrollView->visibleWidth () * 100 / m_document->width (),
                              m_scrollView->visibleHeight () * 100 / m_document->height ()))));
}

// private slot
void kpMainWindow::slotFitToWidth ()
{
    if (!m_scrollView || !m_document)
        return;

    // doc_width * zoom / 100 <= view_width &&
    // 1 <= zoom <= 3200
    
    zoomTo (QMIN (3200, QMAX (1, m_scrollView->visibleWidth () * 100 / m_document->width ())));
}

// private slot
void kpMainWindow::slotFitToHeight ()
{
    if (!m_scrollView || !m_document)
        return;

    // doc_height * zoom / 100 <= view_height &&
    // 1 <= zoom <= 3200
    
    zoomTo (QMIN (3200, QMAX (1, m_scrollView->visibleHeight () * 100 / m_document->height ())));
}


// private slot
void kpMainWindow::slotZoomIn ()
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotZoomIn ()" << endl;
#endif

    m_actionZoom->setCurrentItem (m_actionZoom->currentItem () + 1);
    slotZoom ();
}

// private slot
void kpMainWindow::slotZoomOut ()
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotZoomOut ()" << endl;
#endif

    m_actionZoom->setCurrentItem (m_actionZoom->currentItem () - 1);
    slotZoom ();
}

// private slot
void kpMainWindow::slotZoom ()
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotZoom () index=" << m_actionZoom->currentItem ()
               << " text='" << m_actionZoom->currentText () << "'" << endl;
#endif
    zoomTo (zoomLevelFromString (m_actionZoom->currentText ()));
}


// private slot
void kpMainWindow::slotShowGrid ()
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotShowGrid ()" << endl;
#endif

    m_configShowGrid = m_actionShowGrid->isChecked ();
    m_mainView->showGrid (m_configShowGrid);
}

// private slot
void kpMainWindow::slotActionShowGridToggled (bool on)
{
    m_configShowGrid = on;

    KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    configGroupSaver.config ()->writeEntry (kpSettingShowGrid, m_configShowGrid);
    configGroupSaver.config ()->sync ();
}
