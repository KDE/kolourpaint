
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectBlurSharpenCommand.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectBlurSharpenCommand::kpEffectBlurSharpenCommand(kpEffectBlurSharpen::Type type, int strength, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(kpEffectBlurSharpenCommand::nameForType(type), actOnSelection, environ)
    , m_type(type)
    , m_strength(strength)
{
}

//--------------------------------------------------------------------------------

// public static
QString kpEffectBlurSharpenCommand::nameForType(kpEffectBlurSharpen::Type type)
{
    switch (type) {
    case kpEffectBlurSharpen::Blur:
        return i18n("Soften");
    case kpEffectBlurSharpen::Sharpen:
        return i18n("Sharpen");
    default:
        return {};
    }
}

//--------------------------------------------------------------------------------

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectBlurSharpenCommand::applyEffect(const kpImage &image)
{
    return kpEffectBlurSharpen::applyEffect(image, m_type, m_strength);
}
