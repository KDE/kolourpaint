
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
