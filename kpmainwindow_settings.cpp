
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

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kptool.h>
#include <kptooltoolbar.h>


// private
void kpMainWindow::setupSettingsMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    // Settings/Toolbars |> %s
    setStandardToolBarMenuEnabled (true);

    // Settings/Show Statusbar
    createStandardStatusBarAction ();


    m_actionFullScreen = KStdAction::fullScreen (this, SLOT (slotFullScreen ()), ac,
                                                 this/*window*/);


    m_actionShowPath = new KToggleAction (i18n ("Show &Path"), 0,
        this, SLOT (slotShowPathToggled ()), ac, "settings_show_path");
    m_actionShowPath->setCheckedState (i18n ("Hide &Path"));
    slotEnableSettingsShowPath ();


    m_actionKeyBindings = KStdAction::keyBindings (this, SLOT (slotKeyBindings ()), ac);
    m_actionConfigureToolbars = KStdAction::configureToolbars (this, SLOT (slotConfigureToolBars ()), ac);
    // m_actionConfigure = KStdAction::preferences (this, SLOT (slotConfigure ()), ac);


    enableSettingsMenuDocumentActions (false);
}

// private
void kpMainWindow::enableSettingsMenuDocumentActions (bool /*enable*/)
{
}


// private slot
void kpMainWindow::slotFullScreen ()
{
    if (m_actionFullScreen->isChecked ())
        showFullScreen ();
    else
        showNormal ();
}


// private slot
void kpMainWindow::slotEnableSettingsShowPath ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotEnableSettingsShowPath()" << endl;
#endif

    const bool enable = (m_document && !m_document->url ().isEmpty ());

    m_actionShowPath->setEnabled (enable);
    m_actionShowPath->setChecked (enable && m_configShowPath);
}

// private slot
void kpMainWindow::slotShowPathToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotShowPathToggled()" << endl;
#endif

    m_configShowPath = m_actionShowPath->isChecked ();

    slotUpdateCaption ();


    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingShowPath, m_configShowPath);
    cfg->sync ();
}


// private slot
void kpMainWindow::slotKeyBindings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotKeyBindings()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    bool singleKeyTriggersDisabled = !actionsSingleKeyTriggersEnabled ();

    if (singleKeyTriggersDisabled)
        enableActionsSingleKeyTriggers (true);


    if (KKeyDialog::configure (actionCollection (), this))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\tdialog accepted" << endl;
    #endif
        // TODO: PROPAGATE: thru mainWindow's and interprocess

        if (singleKeyTriggersDisabled)
            enableActionsSingleKeyTriggers (false);
    }
}


// private slot
void kpMainWindow::slotConfigureToolBars ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotConfigureToolBars()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    //saveMainWindowSettings (kapp->config (), autoSaveGroup ());

    KEditToolbar dialog (actionCollection (),
                         QString::null/*default ui.rc file*/,
                         true/*global resource*/,
                         this/*parent*/);
    // Clicking on OK after Apply brings up the dialog (below) again.
    // Bug with KEditToolBar.
    dialog.showButtonApply (false);
    connect (&dialog, SIGNAL (newToolbarConfig ()),
             this, SLOT (slotNewToolBarConfig ()));

    dialog.exec ();
}

// private slot
void kpMainWindow::slotNewToolBarConfig ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotNewToolBarConfig()" << endl;
#endif

    // Wouldn't it be nice if createGUI () didn't nuke all the KToolBar's?
    // (including my non-XMLGUI ones whose states take a _lot_ of effort to
    //  restore).
    // TODO: this message is probably unacceptable - so restore the state of
    //       my toolbars instead.
    KMessageBox::information (this,
        i18n ("You have to restart KolourPaint for these changes to take effect."),
        i18n ("Toolbar Settings Changed"),
        QString::fromLatin1 ("ToolBarSettingsChanged"));

    //createGUI();
    //applyMainWindowSettings (kapp->config (), autoSaveGroup ());
}


// private slot
void kpMainWindow::slotConfigure ()
{
    // TODO
}
