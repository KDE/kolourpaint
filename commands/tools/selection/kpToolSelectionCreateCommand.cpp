
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "commands/tools/selection/kpToolSelectionCreateCommand.h"

#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/text/kpTextSelection.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

#include "kpLogCategories.h"

kpToolSelectionCreateCommand::kpToolSelectionCreateCommand(const QString &name, const kpAbstractSelection &fromSelection, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_fromSelection(nullptr)
    , m_textRow(0)
    , m_textCol(0)
{
    setFromSelection(fromSelection);
}

kpToolSelectionCreateCommand::~kpToolSelectionCreateCommand()
{
    delete m_fromSelection;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolSelectionCreateCommand::size() const
{
    return SelectionSize(m_fromSelection);
}

// public
const kpAbstractSelection *kpToolSelectionCreateCommand::fromSelection() const
{
    return m_fromSelection;
}

// public
void kpToolSelectionCreateCommand::setFromSelection(const kpAbstractSelection &fromSelection)
{
    delete m_fromSelection;
    m_fromSelection = fromSelection.clone();
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::execute()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "kpToolSelectionCreateCommand::execute()";
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);

    if (m_fromSelection) {
#if DEBUG_KP_TOOL_SELECTION
        qCDebug(kpLogCommands) << "\tusing fromSelection";
        qCDebug(kpLogCommands) << "\t\thave sel=" << doc->selection() << endl;
#endif
        kpAbstractImageSelection *imageSel = dynamic_cast<kpAbstractImageSelection *>(m_fromSelection);
        kpTextSelection *textSel = dynamic_cast<kpTextSelection *>(m_fromSelection);
        if (imageSel) {
            if (imageSel->transparency() != environ()->imageSelectionTransparency()) {
                environ()->setImageSelectionTransparency(imageSel->transparency());
            }
        } else if (textSel) {
            if (textSel->textStyle() != environ()->textStyle()) {
                environ()->setTextStyle(textSel->textStyle());
            }
        } else {
            Q_ASSERT(!"Unknown selection type");
        }

        viewManager()->setTextCursorPosition(m_textRow, m_textCol);
        doc->setSelection(*m_fromSelection);

        environ()->somethingBelowTheCursorChanged();
    }
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::unexecute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    if (!doc->selection()) {
        // Was just a border that got deselected?
        if (m_fromSelection && !m_fromSelection->hasContent()) {
            return;
        }

        Q_ASSERT(!"kpToolSelectionCreateCommand::unexecute() without sel region");
        return;
    }

    m_textRow = viewManager()->textCursorRow();
    m_textCol = viewManager()->textCursorCol();

    doc->selectionDelete();

    environ()->somethingBelowTheCursorChanged();
}
