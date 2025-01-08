
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_THUMBNAIL_H
#define KP_THUMBNAIL_H

#include "generic/widgets/kpSubWindow.h"

class QMoveEvent;
class QResizeEvent;

class kpMainWindow;
class kpThumbnailView;

struct kpThumbnailPrivate;

class kpThumbnail : public kpSubWindow
{
    Q_OBJECT

public:
    explicit kpThumbnail(kpMainWindow *parent);
    ~kpThumbnail() override;

public:
    kpThumbnailView *view() const;
    void setView(kpThumbnailView *view);

public Q_SLOTS:
    void updateCaption();

protected Q_SLOTS:
    void slotViewDestroyed();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void moveEvent(QMoveEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

Q_SIGNALS:
    void windowClosed();

private:
    kpThumbnailPrivate *const d;
};

#endif // KP_THUMBNAIL_H
