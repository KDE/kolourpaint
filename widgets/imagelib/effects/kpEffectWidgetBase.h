
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectWidgetBase_H
#define kpEffectWidgetBase_H

#include <QWidget>

#include "imagelib/kpImage.h"

class kpCommandEnvironment;
class kpEffectCommandBase;

class kpEffectWidgetBase : public QWidget
{
    Q_OBJECT

public:
    kpEffectWidgetBase(bool actOnSelection, QWidget *parent);
    ~kpEffectWidgetBase() override;

Q_SIGNALS:
    void settingsChangedNoWaitCursor();

    void settingsChanged();

    // (same as settingsChanged() but preview doesn't update until there
    //  has been no activity for a while - used for sliders in slow effects)
    void settingsChangedDelayed();

public:
    virtual QString caption() const;

    virtual bool isNoOp() const = 0;
    virtual kpImage applyEffect(const kpImage &image) = 0;

    virtual kpEffectCommandBase *createCommand(kpCommandEnvironment *cmdEnviron) const = 0;

protected:
    bool m_actOnSelection;
};

#endif // kpEffectWidgetBase_H
