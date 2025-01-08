/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2011 Martin Koller <kollix@aon.at>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformResizeScaleDialog_H
#define kpTransformResizeScaleDialog_H

#include <QDialog>

#include "commands/imagelib/transforms/kpTransformResizeScaleCommand.h"
#include "imagelib/kpColor.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QToolButton;
class QSpinBox;
class QDoubleSpinBox;

class kpAbstractSelection;
class kpDocument;
class kpTextSelection;
class kpTransformDialogEnvironment;

class kpTransformResizeScaleDialog : public QDialog
{
    Q_OBJECT

public:
    kpTransformResizeScaleDialog(kpTransformDialogEnvironment *_env, QWidget *parent);

    enum ActOn {
        Image,
        Selection
    };

    int imageWidth() const;
    int imageHeight() const;
    bool actOnSelection() const;
    kpTransformResizeScaleCommand::Type type() const;

    bool isNoOp() const;

public Q_SLOTS:
    void slotActOnChanged();
    void slotTypeChanged();

    void slotWidthChanged(int width);
    void slotHeightChanged(int height);

    void slotPercentWidthChanged(double percentWidth);
    void slotPercentHeightChanged(double percentHeight);

private:
    kpDocument *document() const;
    kpAbstractSelection *selection() const;
    kpTextSelection *textSelection() const;

    QWidget *createActOnBox(QWidget *baseWidget);
    QGroupBox *createOperationGroupBox(QWidget *baseWidget);
    QGroupBox *createDimensionsGroupBox(QWidget *baseWidget);

    void widthFitHeightToAspectRatio();
    void heightFitWidthToAspectRatio();

    bool resizeEnabled() const;
    bool scaleEnabled() const;
    bool smoothScaleEnabled() const;
    int originalWidth() const;
    int originalHeight() const;

private Q_SLOTS:
    void accept() override;
    void setKeepAspectRatio(bool on);

private:
    kpTransformDialogEnvironment *m_environ;

    QComboBox *m_actOnCombo;

    QToolButton *m_resizeButton, *m_scaleButton, *m_smoothScaleButton;

    QSpinBox *m_originalWidthInput, *m_originalHeightInput, *m_newWidthInput, *m_newHeightInput;
    QDoubleSpinBox *m_percentWidthInput, *m_percentHeightInput;
    QCheckBox *m_keepAspectRatioCheckBox;

    int m_ignoreKeepAspectRatio;

    kpTransformResizeScaleCommand::Type m_lastType;
};

#endif // kpTransformResizeScaleDialog_H
