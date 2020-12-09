
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

#include <QTimer>

#include <KSharedConfig>
#include <KConfigGroup>
#include <KToggleAction>
#include <KActionCollection>
#include <KLocalizedString>

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
void kpMainWindow::setupViewMenuThumbnailActions ()
{
    d->thumbnailSaveConfigTimer = nullptr;

    KActionCollection *ac = actionCollection ();


    d->actionShowThumbnail = ac->add <KToggleAction> (QStringLiteral("view_show_thumbnail"));
    d->actionShowThumbnail->setText (i18n ("Show T&humbnail"));
    // TODO: This doesn't work when the thumbnail has focus.
    //       Testcase: Press CTRL+H twice on a fresh KolourPaint.
    //                 The second CTRL+H doesn't close the thumbnail.
    ac->setDefaultShortcut (d->actionShowThumbnail, Qt::CTRL | Qt::Key_H);
    //d->actionShowThumbnail->setCheckedState (KGuiItem(i18n ("Hide T&humbnail")));
    connect (d->actionShowThumbnail, &KToggleAction::triggered,
             this, &kpMainWindow::slotShowThumbnailToggled);

    // Please do not use setCheckedState() here - it wouldn't make sense
    d->actionZoomedThumbnail = ac->add <KToggleAction> (QStringLiteral("view_zoomed_thumbnail"));
    d->actionZoomedThumbnail->setText (i18n ("Zoo&med Thumbnail Mode"));
    connect (d->actionZoomedThumbnail, &KToggleAction::triggered,
             this, &kpMainWindow::slotZoomedThumbnailToggled);

    // For consistency with the above action, don't use setCheckedState()
    //
    // Also, don't use "Show Thumbnail Rectangle" because if entire doc
    // can be seen in scrollView, checking option won't "Show" anything
    // since rect _surrounds_ entire doc (hence, won't be rendered).
    d->actionShowThumbnailRectangle = ac->add <KToggleAction> (QStringLiteral("view_show_thumbnail_rectangle"));
    d->actionShowThumbnailRectangle->setText (i18n ("Enable Thumbnail &Rectangle"));
    connect (d->actionShowThumbnailRectangle, &KToggleAction::triggered,
             this, &kpMainWindow::slotThumbnailShowRectangleToggled);
}

// private
void kpMainWindow::enableViewMenuThumbnailDocumentActions (bool enable)
{
    d->actionShowThumbnail->setEnabled (enable);
    enableThumbnailOptionActions (enable);
}

// private slot
void kpMainWindow::slotDestroyThumbnail ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotDestroyThumbnail()";
#endif

    d->actionShowThumbnail->setChecked (false);
    enableThumbnailOptionActions (false);
    updateThumbnail ();
}

// private slot
void kpMainWindow::slotDestroyThumbnailInitatedByUser ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotDestroyThumbnailInitiatedByUser()";
#endif

    d->actionShowThumbnail->setChecked (false);
    slotShowThumbnailToggled ();
}

// private slot
void kpMainWindow::slotCreateThumbnail ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotCreateThumbnail()";
#endif

    d->actionShowThumbnail->setChecked (true);
    enableThumbnailOptionActions (true);
    updateThumbnail ();
}

// public
void kpMainWindow::notifyThumbnailGeometryChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::notifyThumbnailGeometryChanged()";
#endif

    if (!d->thumbnailSaveConfigTimer)
    {
        d->thumbnailSaveConfigTimer = new QTimer (this);
        d->thumbnailSaveConfigTimer->setSingleShot (true);
        connect (d->thumbnailSaveConfigTimer, &QTimer::timeout,
                 this, &kpMainWindow::slotSaveThumbnailGeometry);
    }

    // (single shot)
    d->thumbnailSaveConfigTimer->start (500/*msec*/);
}

// private slot
void kpMainWindow::slotSaveThumbnailGeometry ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::saveThumbnailGeometry()";
#endif

    if (!d->thumbnail) {
        return;
    }

    QRect rect (d->thumbnail->x (), d->thumbnail->y (),
                d->thumbnail->width (), d->thumbnail->height ());
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tthumbnail relative geometry=" << rect;
#endif

    d->configThumbnailGeometry = mapFromGlobal (rect);

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tCONFIG: saving thumbnail geometry "
                << d->configThumbnailGeometry;
#endif

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupThumbnail);

    cfg.writeEntry (kpSettingThumbnailGeometry, d->configThumbnailGeometry);
    cfg.sync ();
}

// private slot
void kpMainWindow::slotShowThumbnailToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotShowThumbnailToggled()";
#endif

    d->configThumbnailShown = d->actionShowThumbnail->isChecked ();

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupThumbnail);

    cfg.writeEntry (kpSettingThumbnailShown, d->configThumbnailShown);
    cfg.sync ();


    enableThumbnailOptionActions (d->actionShowThumbnail->isChecked ());
    updateThumbnail ();
}

// private slot
void kpMainWindow::updateThumbnailZoomed ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::updateThumbnailZoomed() zoomed="
               << d->actionZoomedThumbnail->isChecked ();
#endif

    if (!d->thumbnailView) {
        return;
    }

    destroyThumbnailView ();
    createThumbnailView ();
}

// private slot
void kpMainWindow::slotZoomedThumbnailToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotZoomedThumbnailToggled()";
#endif

    d->configZoomedThumbnail = d->actionZoomedThumbnail->isChecked ();

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupThumbnail);

    cfg.writeEntry (kpSettingThumbnailZoomed, d->configZoomedThumbnail);
    cfg.sync ();


    updateThumbnailZoomed ();
}

// private slot
void kpMainWindow::slotThumbnailShowRectangleToggled ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotThumbnailShowRectangleToggled()";
#endif

    d->configThumbnailShowRectangle = d->actionShowThumbnailRectangle->isChecked ();

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupThumbnail);

    cfg.writeEntry (kpSettingThumbnailShowRectangle, d->configThumbnailShowRectangle);
    cfg.sync ();


    if (d->thumbnailView)
    {
        d->thumbnailView->showBuddyViewScrollableContainerRectangle (
            d->actionShowThumbnailRectangle->isChecked ());
    }
}

// private
void kpMainWindow::enableViewZoomedThumbnail (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::enableSettingsViewZoomedThumbnail()";
#endif

    d->actionZoomedThumbnail->setEnabled (enable &&
        d->actionShowThumbnail->isChecked ());

    // Note: Don't uncheck if disabled - being able to see the zoomed state
    //       before turning on the thumbnail can be useful.
    d->actionZoomedThumbnail->setChecked (d->configZoomedThumbnail);
}

// private
void kpMainWindow::enableViewShowThumbnailRectangle (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::enableViewShowThumbnailRectangle()";
#endif

    d->actionShowThumbnailRectangle->setEnabled (enable &&
        d->actionShowThumbnail->isChecked ());

    // Note: Don't uncheck if disabled for consistency with
    //       enableViewZoomedThumbnail()
    d->actionShowThumbnailRectangle->setChecked (
        d->configThumbnailShowRectangle);
}

// private
void kpMainWindow::enableThumbnailOptionActions (bool enable)
{
    enableViewZoomedThumbnail (enable);
    enableViewShowThumbnailRectangle (enable);
}


// private
void kpMainWindow::createThumbnailView ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\t\tcreating new kpView:";
#endif

    if (d->thumbnailView)
    {
        qCDebug(kpLogMainWindow) << "kpMainWindow::createThumbnailView() had to destroy view";
        destroyThumbnailView ();
    }

    if (d->actionZoomedThumbnail->isChecked ())
    {
        d->thumbnailView = new kpZoomedThumbnailView (
            d->document, d->toolToolBar, d->viewManager,
            d->mainView,
            nullptr/*scrollableContainer*/,
            d->thumbnail);
        d->thumbnailView->setObjectName ( QStringLiteral("thumbnailView" ));
    }
    else
    {
        d->thumbnailView = new kpUnzoomedThumbnailView (
            d->document, d->toolToolBar, d->viewManager,
            d->mainView,
            nullptr/*scrollableContainer*/,
            d->thumbnail);
        d->thumbnailView->setObjectName ( QStringLiteral("thumbnailView" ));
    }

    d->thumbnailView->showBuddyViewScrollableContainerRectangle (
        d->actionShowThumbnailRectangle->isChecked ());


#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\t\tgive kpThumbnail the kpView:";
#endif

    if (d->thumbnail) {
        d->thumbnail->setView (d->thumbnailView);
    }
    else {
        qCCritical(kpLogMainWindow) << "kpMainWindow::createThumbnailView() no thumbnail";
    }

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\t\tregistering the kpView:";
#endif
    if (d->viewManager) {
        d->viewManager->registerView (d->thumbnailView);
    }
}

// private
void kpMainWindow::destroyThumbnailView ()
{
    if (!d->thumbnailView) {
        return;
    }

    if (d->viewManager) {
        d->viewManager->unregisterView (d->thumbnailView);
    }

    if (d->thumbnail) {
        d->thumbnail->setView (nullptr);
    }

    d->thumbnailView->deleteLater (); d->thumbnailView = nullptr;
}


// private
void kpMainWindow::updateThumbnail ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::updateThumbnail()";
#endif
    bool enable = d->actionShowThumbnail->isChecked ();

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tthumbnail="
               << bool (d->thumbnail)
               << " action_isChecked="
               << enable;
#endif

    if (bool (d->thumbnail) == enable) {
        return;
    }

    if (!d->thumbnail)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tcreating thumbnail";
    #endif

        // Read last saved geometry before creating thumbnail & friends
        // in case they call notifyThumbnailGeometryChanged()
        QRect thumbnailGeometry = d->configThumbnailGeometry;
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tlast used geometry=" << thumbnailGeometry;
    #endif

        d->thumbnail = new kpThumbnail (this);

        createThumbnailView ();

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tmoving thumbnail to right place";
    #endif
        if (!thumbnailGeometry.isEmpty () &&
            QRect (0, 0, width (), height ()).intersects (thumbnailGeometry))
        {
            const QRect geometry = mapToGlobal (thumbnailGeometry);
            d->thumbnail->resize (geometry.size ());
            d->thumbnail->move (geometry.topLeft ());
        }
        else
        {
            if (d->scrollView)
            {
                const int margin = 20;
                const int initialWidth = 160, initialHeight = 120;

                QRect geometryRect (width () - initialWidth - margin * 2,
                                    d->scrollView->y () + margin,
                                    initialWidth,
                                    initialHeight);

            #if DEBUG_KP_MAIN_WINDOW
                qCDebug(kpLogMainWindow) << "\t\tcreating geometry=" << geometryRect;
            #endif

                geometryRect = mapToGlobal (geometryRect);
            #if DEBUG_KP_MAIN_WINDOW
                qCDebug(kpLogMainWindow) << "\t\tmap to global=" << geometryRect;
            #endif
                d->thumbnail->resize (geometryRect.size ());
                d->thumbnail->move (geometryRect.topLeft ());
            }
        }

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tshowing thumbnail";
    #endif
        d->thumbnail->show ();

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tconnecting signal thumbnail::windowClosed to destroy slot";
    #endif
        connect (d->thumbnail, &kpThumbnail::windowClosed,
                 this, &kpMainWindow::slotDestroyThumbnailInitatedByUser);
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tDONE";
    #endif
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tdestroying thumbnail d->thumbnail="
            << d->thumbnail;
    #endif

        if (d->thumbnailSaveConfigTimer && d->thumbnailSaveConfigTimer->isActive ())
        {
            d->thumbnailSaveConfigTimer->stop ();
            slotSaveThumbnailGeometry ();
        }

        // Must be done before hiding the thumbnail to avoid triggering
        // this signal - re-entering this code.
        disconnect (d->thumbnail, &kpThumbnail::windowClosed,
                    this, &kpMainWindow::slotDestroyThumbnailInitatedByUser);

        // Avoid change/flicker of caption due to view delete
        // (destroyThumbnailView())
        d->thumbnail->hide ();

        destroyThumbnailView ();

        d->thumbnail->deleteLater (); d->thumbnail = nullptr;
    }
}
