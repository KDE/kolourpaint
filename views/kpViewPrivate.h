
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


struct kpViewPrivate
{
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
    QPointer <kpDocument> document;
    QPointer <kpToolToolBar> toolToolBar;
    QPointer <kpViewManager> viewManager;
    QPointer <kpView> buddyView;
    QPointer <kpViewScrollableContainer> scrollableContainer;

    int hzoom, vzoom;
    QPoint origin;
    bool showGrid;
    bool isBuddyViewScrollableContainerRectangleShown;
    QRect buddyViewScrollableContainerRectangle;

    QRegion queuedUpdateArea;
};


#endif  // kpViewPrivate_H
