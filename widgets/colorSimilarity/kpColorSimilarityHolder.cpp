
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


#define DEBUG_KP_COLOR_SIMILARITY_CUBE 0


#include <kpColorSimilarityHolder.h>

#include <math.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpColor.h>
#include <kpColorSimilarityCubeRenderer.h>
#include <kpDefs.h>


// public static
const double kpColorSimilarityHolder::ColorCubeDiagonalDistance =
    sqrt (255 * 255 * 3);

// public static
const double kpColorSimilarityHolder::MaxColorSimilarity = .30;


kpColorSimilarityHolder::kpColorSimilarityHolder ()
    : m_colorSimilarity (0)
{
}

kpColorSimilarityHolder::~kpColorSimilarityHolder ()
{
}


// HITODO: We need a better explanation.  Get text from the handbook?

// Don't cause the translators grief by appending strings etc.
// - duplicate text with 2 cases

// public static
QString kpColorSimilarityHolder::WhatsThisWithClickInstructions ()
{
    return i18n ("<qt><p><b>Color Similarity</b> is how close "
        "colors must be in the RGB Color Cube "
        "to be considered the same.</p>"

        "<p>If you set it to something "
        "other than <b>Exact Match</b>, "
        "you can work more effectively with dithered "
        "images and photos.</p>"

        "<p>This feature applies to transparent selections, as well as "
        "the Flood Fill, Color Eraser and Autocrop / Remove Internal Border "
        "tools.</p>"

        // sync: Compared to the other string below, we've added this line.
        "<p>To configure it, click on the cube.</p>"

        "</qt>");
}

// public static
QString kpColorSimilarityHolder::WhatsThis ()
{
    return i18n ("<qt><p><b>Color Similarity</b> is how close "
        "colors must be in the RGB Color Cube "
        "to be considered the same.</p>"

        "<p>If you set it to something "
        "other than <b>Exact Match</b>, "
        "you can work more effectively with dithered "
        "images and photos.</p>"

        "<p>This feature applies to transparent selections, as well as "
        "the Flood Fill, Color Eraser and Autocrop / Remove Internal Border "
        "tools.</p>"

        "</qt>");
}


// public
double kpColorSimilarityHolder::colorSimilarity () const
{
    return m_colorSimilarity;
}

// public virtual
void kpColorSimilarityHolder::setColorSimilarity (double similarity)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kDebug () << "kpColorSimilarityHolder::setColorSimilarity(" << similarity << ")";
#endif

    if (m_colorSimilarity == similarity)
        return;

    if (similarity < 0)
        similarity = 0;
    else if (similarity > MaxColorSimilarity)
        similarity = MaxColorSimilarity;

    m_colorSimilarity = similarity;
}
