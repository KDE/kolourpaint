
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectInvertCommand_H
#define kpEffectInvertCommand_H

#include "imagelib/kpImage.h"
#include "kpEffectCommandBase.h"

class kpEffectInvertCommand : public kpEffectCommandBase
{
public:
    kpEffectInvertCommand(int channels, bool actOnSelection, kpCommandEnvironment *environ);
    kpEffectInvertCommand(bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectInvertCommand() override;

    //
    // kpEffectCommandBase interface
    //

public:
    bool isInvertible() const override
    {
        return true;
    }

protected:
    kpImage applyEffect(const kpImage &image) override;

    int m_channels;
};

#endif // kpEffectInvertCommand_H
