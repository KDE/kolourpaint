
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

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstdaction.h>

#include <kpdefs.h>
#include <kpmainwindow.h>
#include <kpview.h>


// private
void kpMainWindow::setupViewMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    /*m_actionFullScreen = KStdAction::fullScreen (0, 0, ac);
    m_actionFullScreen->setEnabled (false);*/

    m_actionZoomIn = KStdAction::zoomIn (this, SLOT (slotZoomIn ()), ac);
    m_actionZoomOut = KStdAction::zoomOut (this, SLOT (slotZoomOut ()), ac);

    m_actionZoom = new KSelectAction (i18n ("&Zoom"), 0,
        this, SLOT (slotZoom ()), actionCollection (), "view_zoom");

    for (int multiple = 1; multiple <= 32; multiple++)
        m_zoomList << QString::number (multiple) + QString ("00%");

    m_actionZoom->setItems (m_zoomList);
    m_actionZoom->setCurrentItem (0);

    m_actionShowGrid = new KToggleAction (i18n ("Show &Grid"), CTRL + Key_G,
        this, SLOT (slotShowGrid ()), actionCollection (), "view_show_grid");
    m_actionShowGrid->setChecked (m_configShowGrid);
    connect (m_actionShowGrid, SIGNAL (toggled (bool)), this, SLOT (slotActionShowGridToggled (bool)));

    enableViewMenuDocumentActions (false);
}

// private
void kpMainWindow::enableViewMenuDocumentActions (bool enable)
{
    m_actionZoomIn->setEnabled (enable);
    m_actionZoomOut->setEnabled (enable);
    m_actionZoom->setEnabled (enable);
    m_actionShowGrid->setEnabled (enable);
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
    kdDebug () << "kpMainWindow::slotZoom ()" << endl;
#endif

    QString text = m_actionZoom->currentText ();
    text.truncate (text.length () - 1);
    int zoomLevel = text.toInt ();

#if DEBUG_KPMAINWINDOW
    kdDebug () << "\tzoomText=" << text << " zoomLevel=" << zoomLevel << "%" << endl;
#endif

    m_actionZoomIn->setEnabled (m_actionZoom->currentItem () < (int) m_zoomList.count () - 1);
    m_actionZoomOut->setEnabled (m_actionZoom->currentItem () > 0);

    m_mainView->setZoomLevel (zoomLevel, zoomLevel);
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
