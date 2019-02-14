
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef kpEffectCommandBase_H
#define kpEffectCommandBase_H


#include <QWidget>

#include "commands/kpCommand.h"
#include "imagelib/kpImage.h"


class kpEffectCommandBase : public kpCommand
{
public:
    kpEffectCommandBase (const QString &name,
        bool actOnSelection,
        kpCommandEnvironment *environ);
    ~kpEffectCommandBase () override;

    QString name () const override;
    SizeType size () const override;

public:
    void execute () override;
    void unexecute () override;

public:
    // Return true if applyEffect(applyEffect(image)) == image
    // to avoid storing the old image, saving memory.
    virtual bool isInvertible () const { return false; }

protected:
    virtual kpImage applyEffect (const kpImage &image) = 0;

private:
    struct kpEffectCommandBasePrivate *d;
};


#endif  // kpEffectCommandBase_H
