
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectGrayscaleCommand.h"
#include "imagelib/effects/kpEffectGrayscale.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectGrayscaleCommand::kpEffectGrayscaleCommand(bool actOnSelection, kpCommandEnvironment *environ)

    : kpEffectCommandBase(i18n("Reduce to Grayscale"), actOnSelection, environ)
{
}

kpEffectGrayscaleCommand::~kpEffectGrayscaleCommand() = default;

//
// kpEffectGrayscaleCommand implements kpEffectCommandBase interface
//

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectGrayscaleCommand::applyEffect(const kpImage &image)
{
    return kpEffectGrayscale::applyEffect(image);
}
