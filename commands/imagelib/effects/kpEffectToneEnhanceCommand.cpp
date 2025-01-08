
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2006 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectToneEnhanceCommand.h"

#include "imagelib/effects/kpEffectToneEnhance.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectToneEnhanceCommand::kpEffectToneEnhanceCommand(double granularity, double amount, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(i18n("Histogram Equalizer"), actOnSelection, environ)
    , m_granularity(granularity)
    , m_amount(amount)
{
}

kpEffectToneEnhanceCommand::~kpEffectToneEnhanceCommand() = default;

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectToneEnhanceCommand::applyEffect(const kpImage &image)
{
    return kpEffectToneEnhance::applyEffect(image, m_granularity, m_amount);
}
