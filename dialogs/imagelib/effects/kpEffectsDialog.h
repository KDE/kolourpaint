
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_EFFECTS_DIALOG_H
#define KP_EFFECTS_DIALOG_H

#include "dialogs/imagelib/transforms/kpTransformPreviewDialog.h"

class QComboBox;
class QGroupBox;
class QImage;
class QTimer;
class QVBoxLayout;

class kpEffectCommandBase;
class kpEffectWidgetBase;

class kpEffectsDialog : public kpTransformPreviewDialog
{
    Q_OBJECT

public:
    // Specifying <defaultSelectedEffect> is more efficient than leaving it
    // as 0 and then calling selectEffect() afterwards.
    kpEffectsDialog(bool actOnSelection, kpTransformDialogEnvironment *_env, QWidget *parent, int defaultSelectedEffect = 0);
    ~kpEffectsDialog() override;

    bool isNoOp() const override;
    kpEffectCommandBase *createCommand() const;

protected:
    QSize newDimensions() const override;
    QImage transformPixmap(const QImage &pixmap, int targetWidth, int targetHeight) const override;

public:
    int selectedEffect() const;
public Q_SLOTS:
    void selectEffect(int which);

protected Q_SLOTS:
    void slotUpdate() override;
    void slotUpdateWithWaitCursor() override;

    void slotDelayedUpdate();

protected:
    static int s_lastWidth, s_lastHeight;

    QTimer *m_delayedUpdateTimer;

    QComboBox *m_effectsComboBox;
    QGroupBox *m_settingsGroupBox;
    QVBoxLayout *m_settingsLayout;

    kpEffectWidgetBase *m_effectWidget;
};

#endif // KP_EFFECTS_DIALOG_H
