
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_EMBOSS 0

#include "kpEffectEmbossCommand.h"
#include "imagelib/effects/kpEffectEmboss.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

kpEffectEmbossCommand::kpEffectEmbossCommand(int strength, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(i18n("Emboss"), actOnSelection, environ)
    , m_strength(strength)
{
}

kpEffectEmbossCommand::~kpEffectEmbossCommand() = default;

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectEmbossCommand::applyEffect(const kpImage &image)
{
    return kpEffectEmboss::applyEffect(image, m_strength);
}
