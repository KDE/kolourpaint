
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_SELECTION 0

#include "kpToolImageSelectionTransparencyCommand.h"

#include "environments/commands/kpCommandEnvironment.h"
#include "generic/kpSetOverrideCursorSaver.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"

#include <QCursor>

//--------------------------------------------------------------------------------

kpToolImageSelectionTransparencyCommand::kpToolImageSelectionTransparencyCommand(const QString &name,
                                                                                 const kpImageSelectionTransparency &st,
                                                                                 const kpImageSelectionTransparency &oldST,
                                                                                 kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , m_st(st)
    , m_oldST(oldST)
{
}

kpToolImageSelectionTransparencyCommand::~kpToolImageSelectionTransparencyCommand() = default;

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolImageSelectionTransparencyCommand::size() const
{
    return 0;
}

// public virtual [base kpCommand]
void kpToolImageSelectionTransparencyCommand::execute()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolImageSelectionTransparencyCommand::execute()";
#endif

    kpSetOverrideCursorSaver cursorSaver(Qt::WaitCursor);

    environ()->setImageSelectionTransparency(m_st, true /*force color change*/);

    if (imageSelection()) {
        imageSelection()->setTransparency(m_st);
    }
}

// public virtual [base kpCommand]
void kpToolImageSelectionTransparencyCommand::unexecute()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolImageSelectionTransparencyCommand::unexecute()";
#endif

    kpSetOverrideCursorSaver cursorSaver(Qt::WaitCursor);

    environ()->setImageSelectionTransparency(m_oldST, true /*force color change*/);

    if (imageSelection()) {
        imageSelection()->setTransparency(m_oldST);
    }
}
