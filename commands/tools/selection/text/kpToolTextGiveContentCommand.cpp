
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "kpToolTextGiveContentCommand.h"

#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpLogCategories.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

kpToolTextGiveContentCommand::kpToolTextGiveContentCommand (
        const kpTextSelection &originalSelBorder,
        const QString &name,
        kpCommandEnvironment *environ)
    : kpAbstractSelectionContentCommand (originalSelBorder, name, environ)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolTextGiveContentCommand::<ctor>() environ="
               << environ
#endif
}

kpToolTextGiveContentCommand::~kpToolTextGiveContentCommand () = default;

// public virtual [base kpCommand]
void kpToolTextGiveContentCommand::execute()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolTextGiveContentCommand::execute()";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);

    kpViewManager *vm = viewManager();
    Q_ASSERT(vm);

    // See API Doc for kpViewManager::textCursorRow() & textCursorCol().
    Q_ASSERT(vm->textCursorRow() == 0 && vm->textCursorCol() == 0);

    vm->setQueueUpdates();
    {
        //
        // Recreate border
        //

        // The previously executed command is required to have been a
        // kpToolSelectionCreateCommand, which must have been given a text
        // selection with no content.
        //
        // However, there is a tricky case.  Suppose we are called for the first
        // time, where the above precondition holds.  We would add content
        // to the selection as expected.  But the user then undoes (CTRL+Z) the
        // operation, calling unexecute().  There is now no content again.
        // Since selection is only a border, the user can freely deselect it
        // and/or select another region without changing the command history
        // or document modified state.  Therefore, if they now call us again
        // by redoing (CTRL+Shift+Z), there is potentially no selection at all
        // or it is at an arbitrary location.
        //
        // This assertion covers all 3 possibilities:
        //
        // 1. First call: text selection with no content
        // 2. Later calls:
        //    a) no text selection (due to deselection)
        //    b) text selection with no content, at an arbitrary location
        Q_ASSERT(!textSelection() || !textSelection()->hasContent());

        const auto *originalTextSel = dynamic_cast<const kpTextSelection *>(originalSelection());

        if (originalTextSel->textStyle() != environ()->textStyle()) {
            environ()->setTextStyle(originalTextSel->textStyle());
        }

        doc->setSelection(*originalSelection());

        //
        // Add Content
        //

        QList<QString> listOfOneEmptyString;
        listOfOneEmptyString.append(QString());
        textSelection()->setTextLines(listOfOneEmptyString);
    }
    vm->restoreQueueUpdates();

    // This should not have changed from the start of the method.
    Q_ASSERT(vm->textCursorRow() == 0 && vm->textCursorCol() == 0);
}

// public virtual [base kpCommand]
void kpToolTextGiveContentCommand::unexecute()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolTextGiveContentCommand::unexecute()";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);
    // Must have selection text content.
    Q_ASSERT(doc->textSelection() && doc->textSelection()->hasContent());

    kpViewManager *vm = viewManager();
    Q_ASSERT(vm);
    // All the commands after us have been unexecuted, so we must be back
    // to the state we were after our execute().
    Q_ASSERT(vm->textCursorRow() == 0 && vm->textCursorCol() == 0);

    // We can have faith that this is the state of the selection after
    // execute(), rather than after the user tried to throw us off by
    // simply selecting another region as to do that, a destroy command
    // must have been used.
    doc->textSelection()->deleteContent();

    // This should not have changed from the start of the method.
    Q_ASSERT(vm->textCursorRow() == 0 && vm->textCursorCol() == 0);
}
