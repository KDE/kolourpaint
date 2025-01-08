
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectInvertCommand.h"

#include "imagelib/effects/kpEffectInvert.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectInvertCommand::kpEffectInvertCommand(int channels, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(channels == kpEffectInvert::RGB ? i18n("Invert Colors") : i18n("Invert"), actOnSelection, environ)
    , m_channels(channels)
{
}

kpEffectInvertCommand::kpEffectInvertCommand(bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectInvertCommand(kpEffectInvert::RGB, actOnSelection, environ)
{
}

kpEffectInvertCommand::~kpEffectInvertCommand() = default;

//
// kpEffectInvertCommand implements kpEffectCommandBase interface
//

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectInvertCommand::applyEffect(const kpImage &image)
{
    return kpEffectInvert::applyEffect(image, m_channels);
}
