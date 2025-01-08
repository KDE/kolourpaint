
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
    explicit kpDocumentSaveOptionsPreviewDialog(QWidget *parent);
    ~kpDocumentSaveOptionsPreviewDialog() override;

    QSize preferredMinimumSize() const;

protected:
    static const QSize s_pixmapLabelMinimumSize;

Q_SIGNALS:
    void moved();
    void resized();
    void finished();

public Q_SLOTS:
    void setFilePixmapAndSize(const QImage &filePixmap, qint64 fileSize);
    void updatePixmapPreview();

protected:
    void closeEvent(QCloseEvent *e) override;
    void moveEvent(QMoveEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

protected:
    QImage *m_filePixmap;
    qint64 m_fileSize;

    kpResizeSignallingLabel *m_filePixmapLabel;
    QLabel *m_fileSizeLabel;
};

#endif // kpDocumentSaveOptionsPreviewDialog_H
