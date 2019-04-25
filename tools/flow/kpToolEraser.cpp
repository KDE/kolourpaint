
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


#define DEBUG_KP_TOOL_ERASER 0

#include "kpToolEraser.h"

#include "commands/kpCommandHistory.h"
#include "commands/imagelib/effects/kpEffectClearCommand.h"
#include "environments/tools/kpToolEnvironment.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpToolEraser::kpToolEraser (kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowPixmapBase (i18n ("Eraser"),
        i18n ("Lets you rub out mistakes"),
        Qt::Key_A,
        environ, parent, QStringLiteral("tool_eraser"))
{
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolEraser::globalDraw ()
{
#if DEBUG_KP_TOOL_ERASER
    qCDebug(kpLogTools) << "kpToolEraser::globalDraw()";
#endif

    commandHistory ()->addCommand (
        new kpEffectClearCommand (
            false/*act on doc, not sel*/,
            backgroundColor (),
            environ ()->commandEnvironment ()));
}

//---------------------------------------------------------------------

// protected virtual [base kpToolFlowBase]
QString kpToolEraser::haventBegunDrawUserMessage () const
{
    return i18n ("Click or drag to erase.");
}

//---------------------------------------------------------------------

// See the our corresponding .h for brush selection.

// Logic is in kpToolFlowPixmapBase.


