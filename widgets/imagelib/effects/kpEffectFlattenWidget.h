
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectFlattenWidget_H
#define kpEffectFlattenWidget_H

#include <QColor>

#include "kpEffectWidgetBase.h"

class QCheckBox;

class KColorButton;

class kpEffectFlattenWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectFlattenWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectFlattenWidget() override;

    static QColor s_lastColor1, s_lastColor2;

    QColor color1() const;
    QColor color2() const;

    //
    // kpEffectWidgetBase interface
    //

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected Q_SLOTS:
    void slotEnableChanged(bool enable);

protected:
    QCheckBox *m_enableCheckBox;
    KColorButton *m_color1Button, *m_color2Button;
};

#endif // kpEffectFlattenWidget_H
