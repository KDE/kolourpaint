
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectCommandBase_H
#define kpEffectCommandBase_H

#include "commands/kpCommand.h"
#include "imagelib/kpImage.h"

class kpEffectCommandBase : public kpCommand
{
public:
    kpEffectCommandBase(const QString &name, bool actOnSelection, kpCommandEnvironment *environ);
    ~kpEffectCommandBase() override;

    QString name() const override;
    SizeType size() const override;

public:
    void execute() override;
    void unexecute() override;

public:
    // Return true if applyEffect(applyEffect(image)) == image
    // to avoid storing the old image, saving memory.
    virtual bool isInvertible() const
    {
        return false;
    }

protected:
    virtual kpImage applyEffect(const kpImage &image) = 0;

private:
    struct kpEffectCommandBasePrivate *d;
};

#endif // kpEffectCommandBase_H
