
// REFACTOR: For all files involved in the class, refactor remaining bits and pieces and add APIDoc

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


#include "tools/selection/text/kpToolText.h"
#include "kpToolTextPrivate.h"
#include "kpLogCategories.h"

#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "commands/tools/selection/text/kpToolTextBackspaceCommand.h"
#include "commands/tools/selection/text/kpToolTextChangeStyleCommand.h"
#include "commands/tools/selection/text/kpToolTextGiveContentCommand.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/text/kpToolTextDeleteCommand.h"
#include "commands/tools/selection/text/kpToolTextEnterCommand.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

#include <KLocalizedString>

kpToolText::kpToolText (kpToolSelectionEnvironment *environ, QObject *parent)
    : kpAbstractSelectionTool (i18n ("Text"), i18n ("Writes text"),
                       Qt::Key_T,
                       environ, parent, QStringLiteral("tool_text")),
      d (new kpToolTextPrivate ())
{
}

kpToolText::~kpToolText ()
{
    delete d;
}


// protected virtual [kpAbstractSelectionTool]
kpAbstractSelectionContentCommand *kpToolText::newGiveContentCommand () const
{
    kpTextSelection *textSel = document ()->textSelection ();
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::newGiveContentCommand()"
              << " textSel=" << textSel
              << "; hasContent=" << textSel->hasContent ();
#endif
    Q_ASSERT (textSel && !textSel->hasContent ());
    
    return new kpToolTextGiveContentCommand (
        *textSel,
        QString()/*uninteresting child of macro cmd*/,
        environ ()->commandEnvironment ());
}


// protected virtual [kpAbstractSelectionTool]
QString kpToolText::nameOfCreateCommand () const
{
    return i18n ("Text: Create Box");
}


// protected virtual [base kpAbstractSelectionTool]
void kpToolText::setSelectionBorderForHaventBegunDraw ()
{
    viewManager ()->setQueueUpdates ();
    {
        kpAbstractSelectionTool::setSelectionBorderForHaventBegunDraw ();
        viewManager ()->setTextCursorEnabled (true);
    }
    viewManager ()->restoreQueueUpdates ();
}


// public virtual [base kpAbstractSelectionTool]
void kpToolText::begin ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "kpToolText::begin()";
#endif

    environ ()->enableTextToolBarActions (true);

    // We don't actually need this since begin() already calls it via
    // setSelectionBorderForHaventBegunDraw().  We leave this in for
    // consistency with end().
    viewManager ()->setTextCursorEnabled (true);
    viewManager()->setInputMethodEnabled (true);

    endTypingCommands ();

    kpAbstractSelectionTool::begin ();
}

// public virtual [base kpAbstractSelectionTool]
void kpToolText::end ()
{
#if DEBUG_KP_TOOL_TEXT && 1
    qCDebug(kpLogTools) << "kpToolText::end()";
#endif

    kpAbstractSelectionTool::end ();

    viewManager()->setInputMethodEnabled (false);
    viewManager ()->setTextCursorEnabled (false);
    environ ()->enableTextToolBarActions (false);
}


// public
bool kpToolText::hasBegunText () const
{
    return (d->insertCommand ||
            d->enterCommand ||
            d->backspaceCommand ||
            d->backspaceWordCommand ||
            d->deleteCommand ||
            d->deleteWordCommand);
}

// public virtual [base kpTool]
bool kpToolText::hasBegunShape () const
{
    return (hasBegunDraw () || hasBegunText ());
}


// protected virtual [base kpAbstractSelectionTool]
kpAbstractSelectionTool::DrawType kpToolText::calculateDrawTypeInsideSelection () const
{
    if (onSelectionToSelectText () && !controlOrShiftPressed ())
    {
        return kpAbstractSelectionTool::SelectText;
    }

    return kpAbstractSelectionTool::calculateDrawTypeInsideSelection ();
}


// public virtual [base kpAbstractSelectionTool]
void kpToolText::cancelShape ()
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::cancelShape()";
#endif

    if (drawType () != None) {
        kpAbstractSelectionTool::cancelShape ();
    }
    else if (hasBegunText ())
    {
        endTypingCommands ();

        commandHistory ()->undo ();
    }
    else {
        kpAbstractSelectionTool::cancelShape ();
    }
}


// public virtual [base kpTool]
void kpToolText::endShape (const QPoint &thisPoint, const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::endShape()";
#endif

    if (drawType () != None) {
        kpAbstractSelectionTool::endDraw (thisPoint, normalizedRect);
    }
    else if (hasBegunText ()) {
        endTypingCommands ();
    }
    else {
        kpAbstractSelectionTool::endDraw (thisPoint, normalizedRect);
    }
}


// protected virtual [base kpAbstractSelectionTool]
QVariant kpToolText::operation (DrawType drawType, Operation op,
        const QVariant &data1, const QVariant &data2)
{
    if (drawType == SelectText) {
        return selectTextOperation (op, data1, data2);
    }

    return kpAbstractSelectionTool::operation (drawType, op, data1, data2);
}


