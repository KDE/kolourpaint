
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformRotateDialog_H
#define kpTransformRotateDialog_H

#include <QImage>

#include "dialogs/imagelib/transforms/kpTransformPreviewDialog.h"
#include "imagelib/kpColor.h"

class QButtonGroup;
class QRadioButton;
class QSpinBox;

class kpTransformRotateDialog : public kpTransformPreviewDialog
{
    Q_OBJECT

public:
    kpTransformRotateDialog(bool actOnSelection, kpTransformDialogEnvironment *_env, QWidget *parent);
    ~kpTransformRotateDialog() override;

private:
    static int s_lastWidth, s_lastHeight;
    static bool s_lastIsClockwise;
    static int s_lastAngleCustom;

    void createDirectionGroupBox();
    void createAngleGroupBox();

public:
    bool isNoOp() const override;
    int angle() const; // 0 <= angle < 360 (clockwise);

private:
    QSize newDimensions() const override;
    QImage transformPixmap(const QImage &pixmap, int targetWidth, int targetHeight) const override;

private Q_SLOTS:
    void slotAngleCustomRadioButtonToggled(bool isChecked);
    void slotUpdate() override;

private Q_SLOTS:
    void accept() override;

private:
    QRadioButton *m_antiClockwiseRadioButton, *m_clockwiseRadioButton;

    QRadioButton *m_angle90RadioButton, *m_angle180RadioButton, *m_angle270RadioButton, *m_angleCustomRadioButton;
    QSpinBox *m_angleCustomInput;
};

#endif // kpTransformRotateDialog_H
