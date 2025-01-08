/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformSkewDialog_H
#define kpTransformSkewDialog_H

#include "imagelib/kpColor.h"
#include "kpTransformPreviewDialog.h"

class QSpinBox;

class kpTransformSkewDialog : public kpTransformPreviewDialog
{
    Q_OBJECT

public:
    kpTransformSkewDialog(bool actOnSelection, kpTransformDialogEnvironment *_env, QWidget *parent);
    ~kpTransformSkewDialog() override;

private:
    static int s_lastWidth, s_lastHeight;
    static int s_lastHorizontalAngle, s_lastVerticalAngle;

    void createAngleGroupBox();

    QSize newDimensions() const override;
    QImage transformPixmap(const QImage &image, int targetWidth, int targetHeight) const override;

    void updateLastAngles();

private Q_SLOTS:
    void slotUpdate() override;

public:
    // These are the angles the users sees in the dialog and...
    int horizontalAngle() const;
    int verticalAngle() const;

    // ...these functions translate them for use in kpPixmapFX::skew().
    static int horizontalAngleForPixmapFX(int hangle);
    static int verticalAngleForPixmapFX(int vangle);

    int horizontalAngleForPixmapFX() const;
    int verticalAngleForPixmapFX() const;

    bool isNoOp() const override;

private Q_SLOTS:
    void accept() override;

private:
    QSpinBox *m_horizontalSkewInput, *m_verticalSkewInput;
};

#endif // kpTransformSkewDialog_H
