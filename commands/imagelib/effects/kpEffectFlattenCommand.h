
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectFlattenCommand_H
#define kpEffectFlattenCommand_H

#include <QColor>

#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectFlattenCommand : public kpEffectCommandBase
{
public:
    kpEffectFlattenCommand(const QColor &color1, const QColor &color2, bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectFlattenCommand() override;

    //
    // kpEffectCommandBase interface
    //

protected:
    kpImage applyEffect(const kpImage &image) override;

    QColor m_color1, m_color2;
};

#endif // kpEffectFlattenCommand_H
