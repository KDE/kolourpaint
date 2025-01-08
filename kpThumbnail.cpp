
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_THUMBNAIL 0

#include "kpThumbnail.h"

#include "document/kpDocument.h"
#include "kpDefs.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"
#include "views/kpThumbnailView.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QAction>
#include <QLayout>

struct kpThumbnailPrivate {
    kpMainWindow *mainWindow;
    kpThumbnailView *view;
    QHBoxLayout *lay;
};

kpThumbnail::kpThumbnail(kpMainWindow *parent)
    : kpSubWindow(parent)
    , d(new kpThumbnailPrivate())
{
    Q_ASSERT(parent);

    d->mainWindow = parent;
    d->view = nullptr;
    d->lay = new QHBoxLayout(this);

    setMinimumSize(64, 64);

    updateCaption();
}

kpThumbnail::~kpThumbnail()
{
    delete d;
}

// public
kpThumbnailView *kpThumbnail::view() const
{
    return d->view;
}

// public
void kpThumbnail::setView(kpThumbnailView *view)
{
#if DEBUG_KP_THUMBNAIL
    qCDebug(kpLogMisc) << "kpThumbnail::setView(" << view << ")";
#endif

    if (d->view == view) {
        return;
    }

    if (d->view) {
        disconnect(d->view, &kpThumbnailView::destroyed, this, &kpThumbnail::slotViewDestroyed);

        disconnect(d->view, &kpThumbnailView::zoomLevelChanged, this, &kpThumbnail::updateCaption);

        d->lay->removeWidget(d->view);
    }

    d->view = view;

    if (d->view) {
        connect(d->view, &kpThumbnailView::destroyed, this, &kpThumbnail::slotViewDestroyed);

        connect(d->view, &kpThumbnailView::zoomLevelChanged, this, &kpThumbnail::updateCaption);

        Q_ASSERT(d->view->parent() == this);
        d->lay->addWidget(d->view, Qt::AlignCenter);

        d->view->show();
    }

    updateCaption();
}

// public slot
void kpThumbnail::updateCaption()
{
    setWindowTitle(view() ? view()->caption() : i18nc("@title:window", "Thumbnail"));
}

// protected slot
void kpThumbnail::slotViewDestroyed()
{
#if DEBUG_KP_THUMBNAIL
    qCDebug(kpLogMisc) << "kpThumbnail::slotViewDestroyed()";
#endif

    d->view = nullptr;
    updateCaption();
}

// protected virtual [base QWidget]
void kpThumbnail::resizeEvent(QResizeEvent *e)
{
#if DEBUG_KP_THUMBNAIL
    qCDebug(kpLogMisc) << "kpThumbnail::resizeEvent(" << width() << "," << height() << ")";
#endif

    QWidget::resizeEvent(e);

    // updateVariableZoom ();  TODO: is below a good idea since this commented out?

    if (d->mainWindow) {
        d->mainWindow->notifyThumbnailGeometryChanged();

        if (d->mainWindow->tool()) {
            d->mainWindow->tool()->somethingBelowTheCursorChanged();
        }
    }
}

// protected virtual [base QWidget]
void kpThumbnail::moveEvent(QMoveEvent * /*e*/)
{
    if (d->mainWindow) {
        d->mainWindow->notifyThumbnailGeometryChanged();
    }
}

// protected virtual [base QWidget]
void kpThumbnail::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);

    Q_EMIT windowClosed();
}

#include "moc_kpThumbnail.cpp"
