
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectClearCommand.h"

#include "document/kpDocument.h"
#include "kpDefs.h"
#include "layers/selections/image/kpAbstractImageSelection.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectClearCommand::kpEffectClearCommand(bool actOnSelection, const kpColor &newColor, kpCommandEnvironment *environ)
    : kpCommand(environ)
    , m_actOnSelection(actOnSelection)
    , m_newColor(newColor)
    , m_oldImagePtr(nullptr)
{
}

kpEffectClearCommand::~kpEffectClearCommand()
{
    delete m_oldImagePtr;
}

// public virtual [base kpCommand]
QString kpEffectClearCommand::name() const
{
    QString opName = i18n("Clear");

    return (m_actOnSelection) ? i18n("Selection: %1", opName) : opName;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpEffectClearCommand::size() const
{
    return ImageSize(m_oldImagePtr);
}

// public virtual [base kpCommand]
void kpEffectClearCommand::execute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    m_oldImagePtr = new kpImage();
    *m_oldImagePtr = doc->image(m_actOnSelection);

    // REFACTOR: Would like to derive entire class from kpEffectCommandBase but
    //           this code makes it difficult since it's not just acting on pixels
    //           (kpAbstractImageSelection::fill() takes into account the shape of a selection).
    if (m_actOnSelection) {
        // OPT: could just edit pixmap directly and signal change
        kpAbstractImageSelection *sel = doc->imageSelection();
        Q_ASSERT(sel);
        sel->fill(m_newColor);
    } else {
        doc->fill(m_newColor);
    }
}

// public virtual [base kpCommand]
void kpEffectClearCommand::unexecute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    doc->setImage(m_actOnSelection, *m_oldImagePtr);

    delete m_oldImagePtr;
    m_oldImagePtr = nullptr;
}
