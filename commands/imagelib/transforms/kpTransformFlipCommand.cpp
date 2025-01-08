
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpTransformFlipCommand.h"

#include <QApplication>

#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "pixmapfx/kpPixmapFX.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpTransformFlipCommand::kpTransformFlipCommand(bool actOnSelection, bool horiz, bool vert, kpCommandEnvironment *environ)
    : kpCommand(environ)
    , m_actOnSelection(actOnSelection)
    , m_horiz(horiz)
    , m_vert(vert)
{
}

//---------------------------------------------------------------------

kpTransformFlipCommand::~kpTransformFlipCommand() = default;

//---------------------------------------------------------------------
// public virtual [base kpCommand]

QString kpTransformFlipCommand::name() const
{
    QString opName;

#if 1
    opName = i18n("Flip");
#else // re-enable when giving full descriptions for all actions
    if (m_horiz && m_vert)
        opName = i18n("Flip horizontally and vertically");
    else if (m_horiz)
        opName = i18n("Flip horizontally");
    else if (m_vert)
        opName = i18n("Flip vertically");
    else {
        qCCritical(kpLogCommands) << "kpTransformFlipCommand::name() not asked to flip";
        return {};
    }
#endif

    if (m_actOnSelection) {
        return i18n("Selection: %1", opName);
    }

    return opName;
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

kpCommandSize::SizeType kpTransformFlipCommand::size() const
{
    return 0;
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

void kpTransformFlipCommand::execute()
{
    flip();
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

void kpTransformFlipCommand::unexecute()
{
    flip();
}

//---------------------------------------------------------------------
// private

void kpTransformFlipCommand::flip()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_actOnSelection) {
        Q_ASSERT(doc->imageSelection());
        doc->imageSelection()->flip(m_horiz, m_vert);
        environ()->somethingBelowTheCursorChanged();
    } else {
        doc->setImage(doc->image().mirrored(m_horiz, m_vert));
    }

    QApplication::restoreOverrideCursor();
}
