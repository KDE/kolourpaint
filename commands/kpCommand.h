
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


#ifndef kpCommand_H
#define kpCommand_H


#include "kpCommandSize.h"
#undef environ  // macro on win32


class QString;

class kpAbstractImageSelection;
class kpAbstractSelection;
class kpCommandEnvironment;
class kpDocument;
class kpTextSelection;
class kpViewManager;


class kpCommand : public kpCommandSize
{
public:
    kpCommand (kpCommandEnvironment *environ);
    virtual ~kpCommand ();

public:
    virtual QString name () const = 0;

    // Returns the estimated size in bytes.
    //
    // You only have to factor in the size of variables that change according
    // to the amount of input e.g. pixmap size, text size.  There is no need
    // to include the size of O(1) variables unless they are huge.
    //
    // If in doubt, return the largest possible amount of memory that your
    // command will take.  This is better than making the user unexpectedly
    // run out of memory.
    //
    // Implement this by measuring the size of all of your fields, using
    // kpCommandSize.
    virtual SizeType size () const = 0;

    virtual void execute () = 0;
    virtual void unexecute () = 0;

protected:
    kpCommandEnvironment *environ () const;

    // Commonly used accessors - simply forwards to environ().
    kpDocument *document () const;

    kpAbstractSelection *selection () const;
    kpAbstractImageSelection *imageSelection () const;
    kpTextSelection *textSelection () const;

    kpViewManager *viewManager () const;

private:
    kpCommandEnvironment * const m_environ;
};


#endif  // kpCommand_H
