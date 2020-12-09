
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


#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"
#include "kpLogCategories.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KToggleAction>
#include <KActionCollection>

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "kpThumbnail.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "views/kpUnzoomedThumbnailView.h"
#include "views/manager/kpViewManager.h"
#include "kpViewScrollableContainer.h"
#include "generic/kpWidgetMapper.h"
#include "views/kpZoomedView.h"
#include "views/kpZoomedThumbnailView.h"


// private
void kpMainWindow::setupViewMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    /*d->actionFullScreen = KStandardAction::fullScreen (0, 0, ac);
    d->actionFullScreen->setEnabled (false);*/


    setupViewMenuZoomActions ();


    d->actionShowGrid = ac->add <KToggleAction> (QStringLiteral("view_show_grid"));
    d->actionShowGrid->setText (i18n ("Show &Grid"));
    ac->setDefaultShortcut (d->actionShowGrid, Qt::CTRL | Qt::Key_G);
    //d->actionShowGrid->setCheckedState (KGuiItem(i18n ("Hide &Grid")));
    connect (d->actionShowGrid, &KToggleAction::triggered,
             this, &kpMainWindow::slotShowGridToggled);


    setupViewMenuThumbnailActions ();


    enableViewMenuDocumentActions (false);
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::viewMenuDocumentActionsEnabled () const
{
    return d->viewMenuDocumentActionsEnabled;
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableViewMenuDocumentActions (bool enable)
{
    d->viewMenuDocumentActionsEnabled = enable;


    enableViewMenuZoomDocumentActions (enable);

    actionShowGridUpdate ();

    enableViewMenuThumbnailDocumentActions (enable);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::actionShowGridUpdate ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::actionShowGridUpdate()";
#endif
    const bool enable = (viewMenuDocumentActionsEnabled () &&
                         d->mainView && d->mainView->canShowGrid ());

    d->actionShowGrid->setEnabled (enable);
    d->actionShowGrid->setChecked (enable && d->configShowGrid);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotShowGridToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotActionShowGridToggled()";
#endif

    updateMainViewGrid ();

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);

    cfg.writeEntry (kpSettingShowGrid, d->configShowGrid = d->actionShowGrid->isChecked ());
    cfg.sync ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::updateMainViewGrid ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::updateMainViewGrid ()";
#endif

    if (d->mainView) {
        d->mainView->showGrid (d->actionShowGrid->isChecked ());
    }
}

//---------------------------------------------------------------------

// private
QRect kpMainWindow::mapToGlobal (const QRect &rect) const
{
    return kpWidgetMapper::toGlobal (this, rect);
}

//---------------------------------------------------------------------

// private
QRect kpMainWindow::mapFromGlobal (const QRect &rect) const
{
    return kpWidgetMapper::fromGlobal (this, rect);
}

//---------------------------------------------------------------------
