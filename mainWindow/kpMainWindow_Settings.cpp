
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpLogCategories.h"
#include "kpMainWindowPrivate.h"
#include "mainWindow/kpMainWindow.h"

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KToggleFullScreenAction>

#include "document/kpDocument.h"
#include "environments/tools/kpToolEnvironment.h"
#include "kpDefs.h"
#include "widgets/toolbars/kpToolToolBar.h"

//---------------------------------------------------------------------

// private
void kpMainWindow::setupSettingsMenuActions()
{
    KActionCollection *ac = actionCollection();

    // Settings/Toolbars |> %s
    setStandardToolBarMenuEnabled(true);

    // Settings/Show Statusbar
    createStandardStatusBarAction();

    d->actionFullScreen = KStandardAction::fullScreen(this, SLOT(slotFullScreen()), this /*window*/, ac);

    d->actionShowPath = ac->add<KToggleAction>(QStringLiteral("settings_show_path"));
    d->actionShowPath->setText(i18n("Show &Path"));
    connect(d->actionShowPath, &QAction::triggered, this, &kpMainWindow::slotShowPathToggled);
    slotEnableSettingsShowPath();

    auto *action = ac->add<KToggleAction>(QStringLiteral("settings_draw_antialiased"));
    action->setText(i18n("Draw Anti-Aliased"));
    action->setChecked(kpToolEnvironment::drawAntiAliased);
    connect(action, &KToggleAction::triggered, this, &kpMainWindow::slotDrawAntiAliasedToggled);

    d->actionKeyBindings = KStandardAction::keyBindings(this, SLOT(slotKeyBindings()), ac);

    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

    enableSettingsMenuDocumentActions(false);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableSettingsMenuDocumentActions(bool /*enable*/)
{
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotFullScreen()
{
    KToggleFullScreenAction::setFullScreen(this, d->actionFullScreen->isChecked());
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotEnableSettingsShowPath()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotEnableSettingsShowPath()";
#endif

    const bool enable = (d->document && !d->document->url().isEmpty());

    d->actionShowPath->setEnabled(enable);
    d->actionShowPath->setChecked(enable && d->configShowPath);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotShowPathToggled()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotShowPathToggled()";
#endif

    d->configShowPath = d->actionShowPath->isChecked();

    slotUpdateCaption();

    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral(kpSettingsGroupGeneral));

    cfg.writeEntry(kpSettingShowPath, d->configShowPath);
    cfg.sync();
}

//---------------------------------------------------------------------

void kpMainWindow::slotDrawAntiAliasedToggled(bool on)
{
    kpToolEnvironment::drawAntiAliased = on;

    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral(kpSettingsGroupGeneral));

    cfg.writeEntry(kpSettingDrawAntiAliased, kpToolEnvironment::drawAntiAliased);
    cfg.sync();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotKeyBindings()
{
    toolEndShape();

    auto *dlg = new KShortcutsDialog(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->addCollection(actionCollection());

    // TODO: PROPAGATE: through mainWindow's and interprocess, by connecting to
    // KShortcutsDialog::saved() signal
    dlg->configure(true /* save settings */);
}

//---------------------------------------------------------------------
