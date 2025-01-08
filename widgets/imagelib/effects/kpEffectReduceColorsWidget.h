
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectReduceColorsWidget_H
#define kpEffectReduceColorsWidget_H

#include "kpEffectWidgetBase.h"

class QRadioButton;

class kpEffectReduceColorsWidget : public kpEffectWidgetBase
{
    Q_OBJECT

public:
    kpEffectReduceColorsWidget(bool actOnSelection, QWidget *parent);

    int depth() const;
    bool dither() const;

    //
    // kpEffectWidgetBase interface
    //

    QString caption() const override;

    bool isNoOp() const override;
    kpImage applyEffect(const kpImage &image) override;

    kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const override;

protected:
    QRadioButton *m_blackAndWhiteRadioButton, *m_blackAndWhiteDitheredRadioButton, *m_8BitRadioButton, *m_8BitDitheredRadioButton, *m_24BitRadioButton;
    QRadioButton *m_defaultRadioButton;
};

#endif // kpEffectReduceColorsWidget_H
