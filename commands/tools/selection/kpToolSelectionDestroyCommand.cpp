
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "kpToolSelectionDestroyCommand.h"
#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/manager/kpViewManager.h"

//---------------------------------------------------------------------

kpToolSelectionDestroyCommand::kpToolSelectionDestroyCommand(const QString &name, bool pushOntoDocument, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_pushOntoDocument(pushOntoDocument)
    , m_oldSelectionPtr(nullptr)
    , m_textRow(0)
    , m_textCol(0)
{
}

//---------------------------------------------------------------------

kpToolSelectionDestroyCommand::~kpToolSelectionDestroyCommand()
{
    delete m_oldSelectionPtr;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolSelectionDestroyCommand::size() const
{
    return ImageSize(m_oldDocImage) + SelectionSize(m_oldSelectionPtr);
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpToolSelectionDestroyCommand::execute()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "kpToolSelectionDestroyCommand::execute () CALLED";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);
    Q_ASSERT(doc->selection());

    m_textRow = viewManager()->textCursorRow();
    m_textCol = viewManager()->textCursorCol();

    Q_ASSERT(!m_oldSelectionPtr);
    m_oldSelectionPtr = doc->selection()->clone();

    if (m_pushOntoDocument) {
        m_oldDocImage = doc->getImageAt(doc->selection()->boundingRect());
        doc->selectionPushOntoDocument();
    } else {
        doc->selectionDelete();
    }

    environ()->somethingBelowTheCursorChanged();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void kpToolSelectionDestroyCommand::unexecute()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "kpToolSelectionDestroyCommand::unexecute () CALLED";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);

    if (doc->selection()) {
        // not error because it's possible that the user dragged out a new
        // region (without pulling image), and then CTRL+Z
#if DEBUG_KP_TOOL_SELECTION
        qCDebug(kpLogCommands) << "kpToolSelectionDestroyCommand::unexecute() already has sel region";
#endif

        if (doc->selection()->hasContent()) {
            Q_ASSERT(!"kpToolSelectionDestroyCommand::unexecute() already has sel content");
            return;
        }
    }

    Q_ASSERT(m_oldSelectionPtr);

    if (m_pushOntoDocument) {
#if DEBUG_KP_TOOL_SELECTION
        qCDebug(kpLogCommands) << "\tunpush oldDocImage onto doc first";
#endif
        doc->setImageAt(m_oldDocImage, m_oldSelectionPtr->topLeft());
    }

#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "\tsetting selection to: rect=" << m_oldSelectionPtr->boundingRect() << " hasContent=" << m_oldSelectionPtr->hasContent();
#endif
    kpAbstractImageSelection *imageSel = dynamic_cast<kpAbstractImageSelection *>(m_oldSelectionPtr);
    kpTextSelection *textSel = dynamic_cast<kpTextSelection *>(m_oldSelectionPtr);
    if (imageSel) {
        if (imageSel->transparency() != environ()->imageSelectionTransparency()) {
            environ()->setImageSelectionTransparency(imageSel->transparency());
        }
        if (dynamic_cast<kpTextSelection *>(doc->selection())) {
            doc->selectionPushOntoDocument();
        }
    } else if (textSel) {
        if (textSel->textStyle() != environ()->textStyle()) {
            environ()->setTextStyle(textSel->textStyle());
        }
        if (dynamic_cast<kpAbstractImageSelection *>(doc->selection())) {
            doc->selectionPushOntoDocument();
        }
    } else {
        Q_ASSERT(!"Unknown selection type");
    }

    viewManager()->setTextCursorPosition(m_textRow, m_textCol);
    doc->setSelection(*m_oldSelectionPtr);

    environ()->somethingBelowTheCursorChanged();

    delete m_oldSelectionPtr;
    m_oldSelectionPtr = nullptr;
}
