
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_FLATTEN 0

#include "kpEffectFlattenCommand.h"
#include "imagelib/effects/kpEffectFlatten.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectFlattenCommand::kpEffectFlattenCommand(const QColor &color1, const QColor &color2, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(i18n("Flatten"), actOnSelection, environ)
    , m_color1(color1)
    , m_color2(color2)
{
}

kpEffectFlattenCommand::~kpEffectFlattenCommand() = default;

//
// kpEffectFlattenCommand implements kpEffectCommandBase interface
//

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectFlattenCommand::applyEffect(const kpImage &image)
{
    return kpEffectFlatten::applyEffect(image, m_color1, m_color2);
}
