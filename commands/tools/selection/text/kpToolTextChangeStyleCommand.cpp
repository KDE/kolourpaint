
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

#define DEBUG_KP_TOOL_TEXT 0


#include "kpToolTextChangeStyleCommand.h"

#include "environments/commands/kpCommandEnvironment.h"
#include "layers/selections/text/kpTextSelection.h"

#include "kpLogCategories.h"


kpToolTextChangeStyleCommand::kpToolTextChangeStyleCommand (const QString &name,
        const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle,
        kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      m_newTextStyle (newTextStyle),
      m_oldTextStyle (oldTextStyle)
{
}

kpToolTextChangeStyleCommand::~kpToolTextChangeStyleCommand () = default;


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolTextChangeStyleCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolTextChangeStyleCommand::execute ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogCommands) << "kpToolTextChangeStyleCommand::execute()"
               << " font=" << m_newTextStyle.fontFamily ()
               << " fontSize=" << m_newTextStyle.fontSize ()
               << " isBold=" << m_newTextStyle.isBold ()
               << " isItalic=" << m_newTextStyle.isItalic ()
               << " isUnderline=" << m_newTextStyle.isUnderline ()
               << " isStrikeThru=" << m_newTextStyle.isStrikeThru ();
#endif

    environ ()->setTextStyle (m_newTextStyle);

    if (textSelection ()) {
        textSelection ()->setTextStyle (m_newTextStyle);
    }
}

// public virtual [base kpCommand]
void kpToolTextChangeStyleCommand::unexecute ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogCommands) << "kpToolTextChangeStyleCommand::unexecute()"
               << " font=" << m_newTextStyle.fontFamily ()
               << " fontSize=" << m_newTextStyle.fontSize ()
               << " isBold=" << m_newTextStyle.isBold ()
               << " isItalic=" << m_newTextStyle.isItalic ()
               << " isUnderline=" << m_newTextStyle.isUnderline ()
               << " isStrikeThru=" << m_newTextStyle.isStrikeThru ();
#endif

    environ ()->setTextStyle (m_oldTextStyle);

    if (textSelection ())
        textSelection ()->setTextStyle (m_oldTextStyle);
}

