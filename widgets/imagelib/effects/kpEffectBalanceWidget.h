
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectBalanceWidget_H
#define kpEffectBalanceWidget_H

#include "imagelib/kpImage.h"
#include "kpEffectWidgetBase.h"

class QLabel;

class QComboBox;
class kpIntNumInput;

class kpEffectBalanceWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectBalanceWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectBalanceWidget() override;

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected:
    int channels() const;

    int brightness() const;
    int contrast() const;
    int gamma() const;

protected Q_SLOTS:
    void recalculateGammaLabel();

    void resetBrightness();
    void resetContrast();
    void resetGamma();

    void resetAll();

protected:
    kpIntNumInput *m_brightnessInput, *m_contrastInput, *m_gammaInput;
    QLabel *m_gammaLabel;
    QComboBox *m_channelsComboBox;
};

#endif // kpEffectBalanceWidget_H
