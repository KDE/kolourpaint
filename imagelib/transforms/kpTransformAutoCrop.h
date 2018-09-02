
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


#ifndef KP_TRANSFORM_AUTO_CROP_H
#define KP_TRANSFORM_AUTO_CROP_H


#include "commands/kpNamedCommand.h"


class QRect;

//class kpImage;
class kpMainWindow;
class kpTransformAutoCropBorder;


// REFACTOR: This should be moved into /commands/
class kpTransformAutoCropCommand : public kpNamedCommand
{
public:
    kpTransformAutoCropCommand (bool actOnSelection,
        const kpTransformAutoCropBorder &leftBorder,
        const kpTransformAutoCropBorder &rightBorder,
        const kpTransformAutoCropBorder &topBorder,
        const kpTransformAutoCropBorder &botBorder,
        kpCommandEnvironment *environ);
    ~kpTransformAutoCropCommand () override;

    enum NameOptions
    {
        DontShowAccel = 0,
        ShowAccel = 1
    };

    static QString text(bool actOnSelection, int options);

    SizeType size () const override;

private:
    void getUndoImage (const kpTransformAutoCropBorder &border, kpImage **image);
    void getUndoImages ();
    void deleteUndoImages ();

public:
    void execute () override;
    void unexecute () override;

private:
    QRect contentsRect () const;

    struct kpTransformAutoCropCommandPrivate *d;
};


// (returns true on success (even if it did nothing) or false on error)
bool kpTransformAutoCrop (kpMainWindow *mainWindow);


#endif  // KP_TRANSFORM_AUTO_CROP_H
