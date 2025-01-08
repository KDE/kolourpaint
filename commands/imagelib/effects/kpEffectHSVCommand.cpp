/*
   SPDX-FileCopyrightText: 2007 Mike Gashler <gashlerm@yahoo.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectHSVCommand.h"
#include "imagelib/effects/kpEffectHSV.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpEffectHSVCommand::kpEffectHSVCommand(double hue, double saturation, double value, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(i18n("Hue, Saturation, Value"), actOnSelection, environ)
    , m_hue(hue)
    , m_saturation(saturation)
    , m_value(value)
{
}

//---------------------------------------------------------------------

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectHSVCommand::applyEffect(const kpImage &image)
{
    return kpEffectHSV::applyEffect(image, m_hue, m_saturation, m_value);
}

//---------------------------------------------------------------------
