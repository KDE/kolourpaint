
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectGrayscaleCommand_H
#define kpEffectGrayscaleCommand_H

#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectGrayscaleCommand : public kpEffectCommandBase
{
public:
    kpEffectGrayscaleCommand(bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectGrayscaleCommand() override;

    //
    // kpEffectCommandBase interface
    //

public:
    bool isInvertible() const override
    {
        return false;
    }

protected:
    kpImage applyEffect(const kpImage &image) override;
};

#endif // kpEffectGrayscaleCommand_H
