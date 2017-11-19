
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


#ifndef KP_TOOL_TEXT_CHANGE_STYLE_COMMAND_H
#define KP_TOOL_TEXT_CHANGE_STYLE_COMMAND_H


#include "commands/kpNamedCommand.h"
#include "layers/selections/text/kpTextStyle.h"


class kpToolTextChangeStyleCommand : public kpNamedCommand
{
public:
    kpToolTextChangeStyleCommand (const QString &name,
        const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle,
        kpCommandEnvironment *environ);
    ~kpToolTextChangeStyleCommand () override;

    kpCommandSize::SizeType size () const override;

    void execute () override;
    void unexecute () override;

protected:
    kpTextStyle m_newTextStyle, m_oldTextStyle;
};


#endif  // KP_TOOL_TEXT_CHANGE_STYLE_COMMAND_H
