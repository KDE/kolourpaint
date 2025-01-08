
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "kpToolSelectionPullFromDocumentCommand.h"

#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "views/manager/kpViewManager.h"

kpToolSelectionPullFromDocumentCommand::kpToolSelectionPullFromDocumentCommand(const kpAbstractImageSelection &originalSelBorder,
                                                                               const kpColor &backgroundColor,
                                                                               const QString &name,
                                                                               kpCommandEnvironment *environ)
    : kpAbstractSelectionContentCommand(originalSelBorder, name, environ)
    , m_backgroundColor(backgroundColor)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolSelectionPullFromDocumentCommand::<ctor>() environ=" << environ;
#endif
}

kpToolSelectionPullFromDocumentCommand::~kpToolSelectionPullFromDocumentCommand() = default;

// public virtual [base kpCommand]
void kpToolSelectionPullFromDocumentCommand::execute()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolSelectionPullFromDocumentCommand::execute()";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);

    kpViewManager *vm = viewManager();
    Q_ASSERT(vm);

    vm->setQueueUpdates();
    {
        //
        // Recreate border
        //

        // The previously executed command is required to have been a
        // kpToolSelectionCreateCommand, which must have been given an image
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
        // 1. First call: image selection with no content
        // 2. Later calls:
        //    a) no image selection (due to deselection)
        //    b) image selection with no content, at an arbitrary location
        Q_ASSERT(!imageSelection() || !imageSelection()->hasContent());

        const auto *originalImageSel = dynamic_cast<const kpAbstractImageSelection *>(originalSelection());

        if (originalImageSel->transparency() != environ()->imageSelectionTransparency()) {
            environ()->setImageSelectionTransparency(originalImageSel->transparency());
        }

        doc->setSelection(*originalSelection());

        //
        // Add content
        //

        doc->imageSelectionPullFromDocument(m_backgroundColor);
    }
    vm->restoreQueueUpdates();
}

// public virtual [base kpCommand]
void kpToolSelectionPullFromDocumentCommand::unexecute()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolSelectionPullFromDocumentCommand::unexecute()";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);
    // Must have selection image content.
    Q_ASSERT(doc->imageSelection() && doc->imageSelection()->hasContent());

    // We can have faith that this is the state of the selection after
    // execute(), rather than after the user tried to throw us off by
    // simply selecting another region as to do that, a destroy command
    // must have been used.
    doc->selectionCopyOntoDocument(false /*use opaque pixmap*/);
    doc->imageSelection()->deleteContent();
}
