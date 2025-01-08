/* ============================================================
 *
 * Date        : 2008-04-17
 * Description : Sane plugin interface for KDE
 *
 * SPDX-FileCopyrightText: 2008 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef SANEDIALOG_H
#define SANEDIALOG_H

#include <KSaneWidget>

#include <KPageDialog>

class SaneDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit SaneDialog(QWidget *parent = nullptr);
    ~SaneDialog() override;

    bool setup();

Q_SIGNALS:
    /**
     * Informs you that an image has scanned. @p id is the same as in the
     * @p preview() signal, if this image had been previewed before.
     *
     * Note, that those id's may not be properly implemented in the current
     * libkscan.
     * @param img the image
     * @param id the image's id
     */
    void finalImage(const QImage &img, int id);

protected Q_SLOTS:
    void imageReady(const QImage &img);

private:
    int nextId();

    KSaneIface::KSaneWidget *m_ksanew;
    QString m_openDev;

    int m_currentId;
};

#endif // SANEDIALOG_H
