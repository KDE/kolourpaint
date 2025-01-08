
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_EFFECT_REDUCE_COLORS 0

#include "kpEffectReduceColorsCommand.h"
#include "imagelib/effects/kpEffectReduceColors.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpEffectReduceColorsCommand::kpEffectReduceColorsCommand(int depth, bool dither, bool actOnSelection, kpCommandEnvironment *environ)
    : kpEffectCommandBase(commandName(depth, dither), actOnSelection, environ)
    , m_depth(depth)
    , m_dither(dither)
{
}

//---------------------------------------------------------------------

// public
QString kpEffectReduceColorsCommand::commandName(int depth, int dither) const
{
    switch (depth) {
    case 1:
        if (dither) {
            return i18n("Reduce to Monochrome (Dithered)");
        }
        return i18n("Reduce to Monochrome");

    case 8:
        if (dither) {
            return i18n("Reduce to 256 Color (Dithered)");
        }
        return i18n("Reduce to 256 Color");

    default:
        return {};
    }
}

//---------------------------------------------------------------------

//
// kpEffectReduceColorsCommand implements kpEffectCommandBase interface
//

// protected virtual [base kpEffectCommandBase]
kpImage kpEffectReduceColorsCommand::applyEffect(const kpImage &image)
{
    return kpEffectReduceColors::applyEffect(image, m_depth, m_dither);
}

//---------------------------------------------------------------------
