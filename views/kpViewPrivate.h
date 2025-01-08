
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpViewPrivate_H
#define kpViewPrivate_H

#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QRegion>

class kpDocument;
class kpToolToolBar;
class kpView;
class kpViewScrollableContainer;
class kpViewManager;

struct kpViewPrivate {
    // sync: kpView::paintEvent()
    //
    // Normally, these pointers must be valid while the kpView is alive.
    // Generally, the objects they point to are deleted only after kpView
    // is deleted.
    //
    // However, sometimes we use deleteLater() for the kpView.
    // Before the delayed deletion is executed, those objects are deleted
    // and then our paintEvent() is called.  paintEvent() must therefore
    // have some way of realising that those objects have been deleted so
    // we use guarded pointers.
    //
    // For more details, see SVN commit:
    //     "r385274 | dang | 2005-02-02 22:08:27 +1100 (Wed, 02 Feb 2005) | 21 lines".
    QPointer<kpDocument> document;
    QPointer<kpToolToolBar> toolToolBar;
    QPointer<kpViewManager> viewManager;
    QPointer<kpView> buddyView;
    QPointer<kpViewScrollableContainer> scrollableContainer;

    int hzoom, vzoom;
    QPoint origin;
    bool showGrid;
    bool isBuddyViewScrollableContainerRectangleShown;
    QRect buddyViewScrollableContainerRectangle;

    QRegion queuedUpdateArea;
};

#endif // kpViewPrivate_H
