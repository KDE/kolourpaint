
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpDefaultColorCollection.h"

#include "imagelib/kpColor.h"

kpDefaultColorCollection::kpDefaultColorCollection()
{
    kpColor colors[] = {kpColor::Black,    kpColor::Gray,      kpColor::Red,        kpColor::Orange,     kpColor::Yellow,     kpColor::Green,
                        kpColor::Aqua,     kpColor::Blue,      kpColor::Purple,     kpColor::Pink,       kpColor::LightGreen,

                        kpColor::White,    kpColor::LightGray, kpColor::DarkRed,    kpColor::DarkOrange, kpColor::DarkYellow, kpColor::DarkGreen,
                        kpColor::DarkAqua, kpColor::DarkBlue,  kpColor::DarkPurple, kpColor::LightBlue,  kpColor::Tan};

    for (const auto &color : colors) {
        addColor(color.toQColor());
    }
}

kpDefaultColorCollection::~kpDefaultColorCollection() = default;
