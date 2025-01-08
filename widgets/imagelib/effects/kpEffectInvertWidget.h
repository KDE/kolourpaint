
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectInvertWidget_H
#define kpEffectInvertWidget_H

#include "kpEffectWidgetBase.h"

class QCheckBox;

class kpEffectInvertWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectInvertWidget(bool actOnSelection, QWidget *parent);
    ~kpEffectInvertWidget() override;

    int channels() const;

    //
    // kpEffectWidgetBase interface
    //

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected Q_SLOTS:
    void slotRGBCheckBoxToggled();
    void slotAllCheckBoxToggled();

protected:
    QCheckBox *m_redCheckBox, *m_greenCheckBox, *m_blueCheckBox, *m_allCheckBox;

    // blockSignals() didn't seem to work
    bool m_inSignalHandler;
};

#endif // kpEffectInvertWidget_H
