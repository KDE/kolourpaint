
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectReduceColorsCommand_H
#define kpEffectReduceColorsCommand_H

#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectReduceColorsCommand : public kpEffectCommandBase
{
public:
    // depth must be 1 or 8
    kpEffectReduceColorsCommand(int depth, bool dither, bool actOnSelection, kpCommandEnvironment *environ);

    QString commandName(int depth, int dither) const;

    //
    // kpEffectCommandBase interface
    //

protected:
    kpImage applyEffect(const kpImage &image) override;

    int m_depth;
    bool m_dither;
};

#endif // kpEffectReduceColorsCommand_H
