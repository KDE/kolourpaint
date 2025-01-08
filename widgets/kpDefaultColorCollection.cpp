
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
