
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_COLOR_SIMILARITY_CUBE 0

#include "kpColorSimilarityHolder.h"

#include "imagelib/kpColor.h"
#include "kpColorSimilarityCubeRenderer.h"
#include "kpDefs.h"

#include <cmath>

#include "kpLogCategories.h"

#include <KLocalizedString>

// public static
const double kpColorSimilarityHolder::ColorCubeDiagonalDistance = std::sqrt(255.0 * 255 * 3);

// public static
const double kpColorSimilarityHolder::MaxColorSimilarity = 0.30;

kpColorSimilarityHolder::kpColorSimilarityHolder()
    : m_colorSimilarity(0)
{
}

kpColorSimilarityHolder::~kpColorSimilarityHolder() = default;

// Don't cause the translators grief by appending strings etc.
// - duplicate text with 2 cases

// public static
QString kpColorSimilarityHolder::WhatsThisWithClickInstructions()
{
    return i18n(
        "<qt>"
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
QString kpColorSimilarityHolder::WhatsThis()
{
    return i18n(
        "<qt>"
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
double kpColorSimilarityHolder::colorSimilarity() const
{
    return m_colorSimilarity;
}

// public virtual
void kpColorSimilarityHolder::setColorSimilarity(double similarity)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    qCDebug(kpLogWidgets) << "kpColorSimilarityHolder::setColorSimilarity(" << similarity << ")";
#endif

    if (m_colorSimilarity == similarity) {
        return;
    }

    if (similarity < 0) {
        similarity = 0;
    } else if (similarity > MaxColorSimilarity) {
        similarity = MaxColorSimilarity;
    }

    m_colorSimilarity = similarity;
}
