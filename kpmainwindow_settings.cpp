
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


#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kstdaction.h>

#include <kpconfigdialog.h>
#include <kpdefs.h>
#include <kpmainwindow.h>


// private
void kpMainWindow::setupSettingsMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    // Settings/Toolbars |> %s
    setStandardToolBarMenuEnabled (true);

    // Settings/Show Statusbar
    createStandardStatusBarAction ();

    m_actionShowPath = new KToggleAction (i18n ("Sho&w Path"), 0,
        this, SLOT (slotShowPath ()), ac, "settings_show_path");
    m_actionShowPath->setChecked (m_configShowPath);
    kdDebug () << "BLAH: m_configShowPath=" << m_configShowPath << " isCheck=" << m_actionShowPath->isChecked () << endl;
    connect (m_actionShowPath, SIGNAL (toggled (bool)),
             this, SLOT (slotActionShowPathToggled (bool)));

    m_actionKeyBindings = KStdAction::keyBindings (this, SLOT (slotKeyBindings ()), ac);
    m_actionConfigureToolbars = KStdAction::configureToolbars (this, SLOT (slotConfigureToolBars ()), ac);
    m_actionConfigure = KStdAction::preferences (this, SLOT (slotConfigure ()), ac);
    
    enableSettingsMenuDocumentActions (false);
}

// private
void kpMainWindow::enableSettingsMenuDocumentActions (bool enable)
{
    m_actionShowPath->setEnabled (enable);
}


// private slot
void kpMainWindow::slotShowPath ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotShowPath ()" << endl;
#endif

    m_configShowPath = m_actionShowPath->isChecked ();
    slotUpdateCaption ();
}

// private slot
void kpMainWindow::slotActionShowPathToggled (bool on)
{
    m_configShowPath = on;

    KConfigGroupSaver configGroupSaver (kapp->config (), kpSettingsGroupGeneral);
    configGroupSaver.config ()->writeEntry (kpSettingShowPath, m_configShowPath);
    configGroupSaver.config ()->sync ();
}


// private slot
void kpMainWindow::slotConfigure ()
{
    // TODO: parent
    kpConfigDialog *dialog = new kpConfigDialog ();
    dialog->exec ();
    delete dialog;
}

// private slot
void kpMainWindow::slotKeyBindings ()
{
    // TODO: wrong - need propagate to other mainWindows
    KKeyDialog::configure (actionCollection (), this);
    actionCollection ()->readShortcutSettings ();
}

// private slot
void kpMainWindow::slotConfigureToolBars ()
{
    // TODO: wrong
    // TODO: parent
    KEditToolbar dialog (actionCollection ());
    if (dialog.exec ())
        createGUI ();
}
