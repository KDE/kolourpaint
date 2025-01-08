
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectBalanceCommand.h"

#include "imagelib/effects/kpEffectBalance.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectBalanceCommand::kpEffectBalanceCommand(int channels, int brightness, int contrast, int gamma, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(i18n("Balance"), actOnSelection, environ)
    , m_channels(channels)
    , m_brightness(brightness)
    , m_contrast(contrast)
    , m_gamma(gamma)
{
}

kpEffectBalanceCommand::~kpEffectBalanceCommand() = default;

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectBalanceCommand::applyEffect(const kpImage &image)
{
    return kpEffectBalance::applyEffect(image, m_channels, m_brightness, m_contrast, m_gamma);
}
