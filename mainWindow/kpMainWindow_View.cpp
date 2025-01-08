
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
#include <KToggleAction>

#include "document/kpDocument.h"
#include "generic/kpWidgetMapper.h"
#include "kpDefs.h"
#include "kpThumbnail.h"
#include "kpViewScrollableContainer.h"
#include "tools/kpTool.h"
#include "views/kpUnzoomedThumbnailView.h"
#include "views/kpZoomedThumbnailView.h"
#include "views/kpZoomedView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/kpToolToolBar.h"

// private
void kpMainWindow::setupViewMenuActions()
{
    KActionCollection *ac = actionCollection();

    /*d->actionFullScreen = KStandardAction::fullScreen (0, 0, ac);
    d->actionFullScreen->setEnabled (false);*/

    setupViewMenuZoomActions();

    d->actionShowGrid = ac->add<KToggleAction>(QStringLiteral("view_show_grid"));
    d->actionShowGrid->setText(i18n("Show &Grid"));
    ac->setDefaultShortcut(d->actionShowGrid, Qt::CTRL | Qt::Key_G);
    // d->actionShowGrid->setCheckedState (KGuiItem(i18n ("Hide &Grid")));
    connect(d->actionShowGrid, &KToggleAction::triggered, this, &kpMainWindow::slotShowGridToggled);

    setupViewMenuThumbnailActions();

    enableViewMenuDocumentActions(false);
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::viewMenuDocumentActionsEnabled() const
{
    return d->viewMenuDocumentActionsEnabled;
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableViewMenuDocumentActions(bool enable)
{
    d->viewMenuDocumentActionsEnabled = enable;

    enableViewMenuZoomDocumentActions(enable);

    actionShowGridUpdate();

    enableViewMenuThumbnailDocumentActions(enable);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::actionShowGridUpdate()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::actionShowGridUpdate()";
#endif
    const bool enable = (viewMenuDocumentActionsEnabled() && d->mainView && d->mainView->canShowGrid());

    d->actionShowGrid->setEnabled(enable);
    d->actionShowGrid->setChecked(enable && d->configShowGrid);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotShowGridToggled()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotActionShowGridToggled()";
#endif

    updateMainViewGrid();

    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral(kpSettingsGroupGeneral));

    cfg.writeEntry(kpSettingShowGrid, d->configShowGrid = d->actionShowGrid->isChecked());
    cfg.sync();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::updateMainViewGrid()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::updateMainViewGrid ()";
#endif

    if (d->mainView) {
        d->mainView->showGrid(d->actionShowGrid->isChecked());
    }
}

//---------------------------------------------------------------------

// private
QRect kpMainWindow::mapToGlobal(const QRect &rect) const
{
    return kpWidgetMapper::toGlobal(this, rect);
}

//---------------------------------------------------------------------

// private
QRect kpMainWindow::mapFromGlobal(const QRect &rect) const
{
    return kpWidgetMapper::fromGlobal(this, rect);
}

//---------------------------------------------------------------------
