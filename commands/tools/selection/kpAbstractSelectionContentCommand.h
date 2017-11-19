
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


#ifndef kpAbstractSelectionContentCommand_H
#define kpAbstractSelectionContentCommand_H


#include "commands/kpNamedCommand.h"


// Converts a selection border to a selection with content.
// This must be executed before any manipulations can be made
// to a selection.
//
// Its construction and execution always follows that of a
// kpToolSelectionCreateCommand, which must be given a selection with
// no content.
//
// It's always the first subcommand of a kpMacroCommand, with the following
// subcommands being whatever the selection operation is (e.g. movement,
// resizing).
class kpAbstractSelectionContentCommand : public kpNamedCommand
{
// LOREFACTOR: Pull up more methods into here?  Looking at the code, not
//             much could be dragged up without unnecessarily complicated
//             abstraction.
public:
    // <originalSelBorder> must be a border i.e. have no content.
    kpAbstractSelectionContentCommand (
        const kpAbstractSelection &originalSelBorder,
        const QString &name,
        kpCommandEnvironment *environ);
    ~kpAbstractSelectionContentCommand () override;

    kpCommandSize::SizeType size () const override;

    // Note: Returned pointer is only valid for as long as this command is
    //       alive.
    const kpAbstractSelection *originalSelection () const;

private:
    struct kpAbstractSelectionContentCommandPrivate * const d;
};


#endif  // kpAbstractSelectionContentCommand_H
