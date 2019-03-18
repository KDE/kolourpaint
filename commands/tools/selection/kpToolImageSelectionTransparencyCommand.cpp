
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


#define DEBUG_KP_TOOL_SELECTION 0


#include "kpToolImageSelectionTransparencyCommand.h"

#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "generic/kpSetOverrideCursorSaver.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"

#include <QCursor>

//--------------------------------------------------------------------------------

kpToolImageSelectionTransparencyCommand::kpToolImageSelectionTransparencyCommand (
        const QString &name,
        const kpImageSelectionTransparency &st,
        const kpImageSelectionTransparency &oldST,
        kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      m_st (st),
      m_oldST (oldST)
{
}

kpToolImageSelectionTransparencyCommand::~kpToolImageSelectionTransparencyCommand () = default;


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolImageSelectionTransparencyCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolImageSelectionTransparencyCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolImageSelectionTransparencyCommand::execute()";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);
        
    environ ()->setImageSelectionTransparency (m_st, true/*force colour change*/);

    if (imageSelection ()) {
        imageSelection ()->setTransparency (m_st);
    }
}

// public virtual [base kpCommand]
void kpToolImageSelectionTransparencyCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolImageSelectionTransparencyCommand::unexecute()";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);
        
    environ ()->setImageSelectionTransparency (m_oldST, true/*force colour change*/);

    if (imageSelection ()) {
        imageSelection ()->setTransparency (m_oldST);
    }
}

