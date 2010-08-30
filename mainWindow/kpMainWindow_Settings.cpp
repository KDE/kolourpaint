
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <ktogglefullscreenaction.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpToolAction.h>
#include <kpToolToolBar.h>


// private
void kpMainWindow::setupSettingsMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    // Settings/Toolbars |> %s
    setStandardToolBarMenuEnabled (true);

    // Settings/Show Statusbar
    createStandardStatusBarAction ();


    d->actionFullScreen = KStandardAction::fullScreen (this, SLOT (slotFullScreen ()),
                                                      this/*window*/, ac);


    d->actionShowPath = ac->add<KToggleAction> ("settings_show_path");
    d->actionShowPath->setText (i18n ("Show &Path"));
    connect(d->actionShowPath, SIGNAL(triggered(bool) ), SLOT (slotShowPathToggled ()));
    //d->actionShowPath->setCheckedState (KGuiItem(i18n ("Hide &Path")));
    slotEnableSettingsShowPath ();


    d->actionKeyBindings = KStandardAction::keyBindings (this, SLOT (slotKeyBindings ()), ac);
    d->actionConfigureToolbars = KStandardAction::configureToolbars (this, SLOT (slotConfigureToolBars ()), ac);
    // d->actionConfigure = KStandardAction::preferences (this, SLOT (slotConfigure ()), ac);


    enableSettingsMenuDocumentActions (false);
}

// private
void kpMainWindow::enableSettingsMenuDocumentActions (bool /*enable*/)
{
}


// private slot
void kpMainWindow::slotFullScreen ()
{
    KToggleFullScreenAction::setFullScreen( this, d->actionFullScreen->isChecked ());
}


// private slot
void kpMainWindow::slotEnableSettingsShowPath ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotEnableSettingsShowPath()";
#endif

    const bool enable = (d->document && !d->document->url ().isEmpty ());

    d->actionShowPath->setEnabled (enable);
    d->actionShowPath->setChecked (enable && d->configShowPath);
}

// private slot
void kpMainWindow::slotShowPathToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotShowPathToggled()";
#endif

    d->configShowPath = d->actionShowPath->isChecked ();

    slotUpdateCaption ();


    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

    cfg.writeEntry (kpSettingShowPath, d->configShowPath);
    cfg.sync ();
}


// private slot
void kpMainWindow::slotKeyBindings ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotKeyBindings()";
#endif

    toolEndShape ();

    if (KShortcutsDialog::configure (actionCollection (),
            KShortcutsEditor::LetterShortcutsAllowed,
            this))
    {
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\tdialog accepted";
    #endif
        // TODO: PROPAGATE: thru mainWindow's and interprocess

        kpToolAction::updateAllActionsToolTips (actionCollection ());
    }
}


// private slot
void kpMainWindow::slotConfigureToolBars ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotConfigureToolBars()";
#endif

    toolEndShape ();

// TODO: Do we need this?
#if 0
    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    KGlobal::config ()->reparseConfiguration ();
#endif

    saveMainWindowSettings (KGlobal::config ()->group (autoSaveGroup ()));

    KEditToolBar dialog (actionCollection (), this);

    connect (&dialog, SIGNAL (newToolBarConfig()),
             this, SLOT (slotNewToolBarConfig ()));

    dialog.exec ();
}

// private slot
void kpMainWindow::slotNewToolBarConfig ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotNewToolBarConfig()";
#endif

    createGUI();
    applyMainWindowSettings (KGlobal::config ()->group (autoSaveGroup ()));
}


// private slot
void kpMainWindow::slotConfigure ()
{
    // TODO
}
