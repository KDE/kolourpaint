
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


#ifndef kpToolFloodFillCommand_H
#define kpToolFloodFillCommand_H


#include "commands/kpCommand.h"
#include "imagelib/kpFloodFill.h"


class kpColor;
class kpCommandEnvironment;


struct kpToolFloodFillCommandPrivate;

class kpToolFloodFillCommand : public kpCommand, public kpFloodFill
{
public:
    kpToolFloodFillCommand (int x, int y,
                            const kpColor &color, int processedColorSimilarity,
                            kpCommandEnvironment *environ);
    ~kpToolFloodFillCommand () override;

    QString name () const override;

    kpCommandSize::SizeType size () const override;

    // Optimization hack: filling a fresh, unmodified document does not require
    //                    reading any pixels - just set the whole document to
    //                    <color>.
    void setFillEntireImage (bool yes = true);

    void execute () override;
    void unexecute () override;

private:
    kpToolFloodFillCommandPrivate * const d;
};


#endif  // kpToolFloodFillCommand_H
