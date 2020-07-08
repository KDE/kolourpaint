
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


#include "kpColorSimilarityHolder.h"

#include "kpColorSimilarityCubeRenderer.h"
#include "imagelib/kpColor.h"
#include "kpDefs.h"

#include <cmath>

#include <QPainter>
#include <QPixmap>
#include <QPolygon>

#include "kpLogCategories.h"

#include <KLocalizedString>

// public static
const double kpColorSimilarityHolder::ColorCubeDiagonalDistance =
    std::sqrt (255.0 * 255 * 3);

// public static
const double kpColorSimilarityHolder::MaxColorSimilarity = 0.30;


kpColorSimilarityHolder::kpColorSimilarityHolder ()
    : m_colorSimilarity (0)
{
}

kpColorSimilarityHolder::~kpColorSimilarityHolder () = default;


// Don't cause the translators grief by appending strings etc.
// - duplicate text with 2 cases

// public static
QString kpColorSimilarityHolder::WhatsThisWithClickInstructions ()
{
    return i18n ("<qt>"
        "<p><b>Color Similarity</b> is how <i>similar</i> the colors of different pixels"
        " must be, for operations to consider them to be the same.</p>"

        "<p>If you set it to something other than <b>Exact Match</b>,"
        " you can work more effectively with dithered"
        " images and photos, in a comparable manner to the \"Magic Wand\""
        " feature of other paint programs.</p>"

        "<p>This feature applies to:</p>"

        "<ul>"
         
        "<li><b>Selections</b>: In <b>Transparent</b> mode, any color in the"
        " selection that is <i>similar</i> to the background color will"
        " be made transparent.</li>"

        "<li><b>Flood Fill</b>: For regions with <i>similar</i> - but not"
        " identical - colored pixels, a higher setting is likely to"
        " fill more pixels.</li>"
        
        "<li><b>Color Eraser</b>: Any pixel whose color is <i>similar</i>"
        " to the foreground color will be replaced with the background"
        " color.</li>"

        "<li><b>Autocrop</b> and <b>Remove Internal Border</b>: For"
        " borders with <i>similar</i> - but not identical - colored pixels,"
        " a higher setting is more likely to crop the whole border.</li>"

        "</ul>"
        
        "<p>Higher settings mean that operations consider an increased range"
        " of colors to be sufficiently <i>similar</i> so as to be the same. Therefore,"
        " you should increase the setting if the above operations are not"
        " affecting pixels whose colors you consider to be similar enough.</p>"

        "<p>However, if they are having too much of an effect and are changing"
        " pixels whose colors you do not consider to be similar"
        " (e.g. if <b>Flood Fill</b> is changing too many pixels), you"
        " should decrease this setting.</p>"

        // sync: Compared to the other string below, we've added this line.
        "<p>To configure it, click on the cube.</p>"

        "</qt>");
}

// public static
QString kpColorSimilarityHolder::WhatsThis ()
{
    return i18n ("<qt>"
        "<p><b>Color Similarity</b> is how <i>similar</i> the colors of different pixels"
        " must be, for operations to consider them to be the same.</p>"

        "<p>If you set it to something other than <b>Exact Match</b>,"
        " you can work more effectively with dithered"
        " images and photos, in a comparable manner to the \"Magic Wand\""
        " feature of other paint programs.</p>"

        "<p>This feature applies to:</p>"

        "<ul>"
         
        "<li><b>Selections</b>: In <b>Transparent</b> mode, any color in the"
        " selection that is <i>similar</i> to the background color will"
        " be made transparent.</li>"

        "<li><b>Flood Fill</b>: For regions with <i>similar</i> - but not"
        " identical - colored pixels, a higher setting is likely to"
        " fill more pixels.</li>"
        
        "<li><b>Color Eraser</b>: Any pixel whose color is <i>similar</i>"
        " to the foreground color will be replaced with the background"
        " color.</li>"

        "<li><b>Autocrop</b> and <b>Remove Internal Border</b>: For"
        " borders with <i>similar</i> - but not identical - colored pixels,"
        " a higher setting is more likely to crop the whole border.</li>"

        "</ul>"
        
        "<p>Higher settings mean that operations consider an increased range"
        " of colors to be sufficiently <i>similar</i> so as to be the same. Therefore,"
        " you should increase the setting if the above operations are not"
        " affecting pixels whose colors you consider to be similar enough.</p>"

        "<p>However, if they are having too much of an effect and are changing"
        " pixels whose colors you do not consider to be similar"
        " (e.g. if <b>Flood Fill</b> is changing too many pixels), you"
        " should decrease this setting.</p>"

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
    qCDebug(kpLogWidgets) << "kpColorSimilarityHolder::setColorSimilarity(" << similarity << ")";
#endif

    if (m_colorSimilarity == similarity) {
        return;
    }

    if (similarity < 0) {
        similarity = 0;
    }
    else if (similarity > MaxColorSimilarity) {
        similarity = MaxColorSimilarity;
    }

    m_colorSimilarity = similarity;
}
