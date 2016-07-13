
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


#define DEBUG_KP_COLOR 0


#include "kpColor.h"


static inline int RoundUp2 (int val)
{
    return val % 2 ? val + 1 : val;
}

static inline int Bound0_255 (int val)
{
    return qBound (0, val, 255);
}


enum
{
    BlendDark = 25,
    BlendNormal = 50,
    BlendLight = 75,
    BlendAdd = 100
};

// Adds the 2 given colors together and then multiplies by the given <percent>.
static inline kpColor Blend (const kpColor &a, const kpColor &b,
        int percent = ::BlendNormal)
{
    return kpColor (::Bound0_255 (::RoundUp2 (a.red () + b.red ()) * percent / 100),
                    ::Bound0_255 (::RoundUp2 (a.green () + b.green ()) * percent / 100),
                    ::Bound0_255 (::RoundUp2 (a.blue () + b.blue ()) * percent / 100));
}

static inline kpColor Add (const kpColor &a, const kpColor &b)
{
    return ::Blend (a, b, ::BlendAdd);
}


// (intentionally _not_ an HSV darkener)
static inline kpColor Dark (const kpColor &color)
{
    return ::Blend (color, kpColor::Black);
}


// public static
const int kpColor::Exact = 0;

// public static
const kpColor kpColor::Invalid;  // LOTODO: what's wrong with explicitly specifying () constructor?
const kpColor kpColor::Transparent (0, 0, 0, true/*isTransparent*/);


//
// Make our own colors in case weird ones like "Qt::cyan"
// (turquoise) get changed by Qt.
//


const kpColor kpColor::Red (255, 0, 0);
const kpColor kpColor::Green (0, 255, 0);
const kpColor kpColor::Blue (0, 0, 255);
const kpColor kpColor::Black (0, 0, 0);
const kpColor kpColor::White (255, 255, 255);

const kpColor kpColor::Yellow = ::Add (kpColor::Red, kpColor::Green);
const kpColor kpColor::Purple = ::Add (kpColor::Red, kpColor::Blue);
const kpColor kpColor::Aqua = ::Add (kpColor::Green, kpColor::Blue);

const kpColor kpColor::Gray = ::Blend (kpColor::Black, kpColor::White);
const kpColor kpColor::LightGray = ::Blend (kpColor::Gray, kpColor::White);
const kpColor kpColor::Orange = ::Blend (kpColor::Red, kpColor::Yellow);

const kpColor kpColor::Pink = ::Blend (kpColor::Red, kpColor::White);
const kpColor kpColor::LightGreen = ::Blend (kpColor::Green, kpColor::White);
const kpColor kpColor::LightBlue = ::Blend (kpColor::Blue, kpColor::White);
const kpColor kpColor::Tan = ::Blend (kpColor::Yellow, kpColor::White);

const kpColor kpColor::DarkRed = ::Dark (kpColor::Red);
const kpColor kpColor::DarkOrange = ::Dark (kpColor::Orange);
const kpColor kpColor::Brown = kpColor::DarkOrange;
const kpColor kpColor::DarkYellow = ::Dark (kpColor::Yellow);
const kpColor kpColor::DarkGreen = ::Dark (kpColor::Green);
const kpColor kpColor::DarkAqua = ::Dark (kpColor::Aqua);
const kpColor kpColor::DarkBlue = ::Dark (kpColor::Blue);
const kpColor kpColor::DarkPurple = ::Dark (kpColor::Purple);
const kpColor kpColor::DarkGray = ::Dark (kpColor::Gray);
