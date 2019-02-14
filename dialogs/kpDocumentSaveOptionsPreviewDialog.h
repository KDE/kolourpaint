
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


#ifndef kpDocumentSaveOptionsPreviewDialog_H
#define kpDocumentSaveOptionsPreviewDialog_H

#include "generic/widgets/kpSubWindow.h"

#include <QSize>


class QCloseEvent;
class QImage;
class QLabel;
class QMoveEvent;
class QResizeEvent;

class kpResizeSignallingLabel;


class kpDocumentSaveOptionsPreviewDialog : public kpSubWindow
{
Q_OBJECT

public:
    kpDocumentSaveOptionsPreviewDialog (QWidget *parent);
    ~kpDocumentSaveOptionsPreviewDialog () override;

    QSize preferredMinimumSize () const;

protected:
    static const QSize s_pixmapLabelMinimumSize;

signals:
    void moved ();
    void resized ();
    void finished ();

public slots:
    void setFilePixmapAndSize (const QImage &filePixmap, qint64 fileSize);
    void updatePixmapPreview ();

protected:
    void closeEvent (QCloseEvent *e) override;
    void moveEvent (QMoveEvent *e) override;
    void resizeEvent (QResizeEvent *e) override;

protected:
    QImage *m_filePixmap;
    qint64 m_fileSize;

    kpResizeSignallingLabel *m_filePixmapLabel;
    QLabel *m_fileSizeLabel;
};


#endif  // kpDocumentSaveOptionsPreviewDialog_H
