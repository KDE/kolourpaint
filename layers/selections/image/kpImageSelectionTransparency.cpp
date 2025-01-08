
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_SELECTION_TRANSPARENCY 0

#include "layers/selections/image/kpImageSelectionTransparency.h"

#include "kpLogCategories.h"

#include "widgets/colorSimilarity/kpColorSimilarityHolder.h"

//---------------------------------------------------------------------

kpImageSelectionTransparency::kpImageSelectionTransparency()
    : m_isOpaque(true)
{
    setColorSimilarity(0);
}

//---------------------------------------------------------------------

kpImageSelectionTransparency::kpImageSelectionTransparency(const kpColor &transparentColor, double colorSimilarity)
    : m_isOpaque(false)
    , m_transparentColor(transparentColor)
{
    setColorSimilarity(colorSimilarity);
}

//---------------------------------------------------------------------

kpImageSelectionTransparency::kpImageSelectionTransparency(bool isOpaque, const kpColor &transparentColor, double colorSimilarity)
    : m_isOpaque(isOpaque)
    , m_transparentColor(transparentColor)
{
    setColorSimilarity(colorSimilarity);
}

//---------------------------------------------------------------------

bool kpImageSelectionTransparency::operator==(const kpImageSelectionTransparency &rhs) const
{
#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
    qCDebug(kpLogLayers) << "kpImageSelectionTransparency::operator==()";
#endif

    if (m_isOpaque != rhs.m_isOpaque) {
#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
        qCDebug(kpLogLayers) << "\tdifferent opacity: lhs=" << m_isOpaque << " rhs=" << rhs.m_isOpaque << endl;
#endif
        return false;
    }

    if (m_isOpaque) {
#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
        qCDebug(kpLogLayers) << "\tboth opaque - eq";
#endif
        return true;
    }

#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
    qCDebug(kpLogLayers) << "\tcolors: lhs=" << (int *)m_transparentColor.toQRgb() << " rhs=" << (int *)rhs.m_transparentColor.toQRgb() << endl;
    qCDebug(kpLogLayers) << "\tcolor similarity: lhs=" << m_colorSimilarity << " rhs=" << rhs.m_colorSimilarity << endl;
#endif

    return (m_transparentColor == rhs.m_transparentColor && m_colorSimilarity == rhs.m_colorSimilarity);
}

//---------------------------------------------------------------------

bool kpImageSelectionTransparency::operator!=(const kpImageSelectionTransparency &rhs) const
{
    return !(*this == rhs);
}

//---------------------------------------------------------------------

kpImageSelectionTransparency::~kpImageSelectionTransparency() = default;

//---------------------------------------------------------------------

// public
bool kpImageSelectionTransparency::isOpaque() const
{
    return m_isOpaque;
}

//---------------------------------------------------------------------

// public
bool kpImageSelectionTransparency::isTransparent() const
{
    return !isOpaque();
}

//---------------------------------------------------------------------

// public
void kpImageSelectionTransparency::setOpaque(bool yes)
{
    m_isOpaque = yes;
}

//---------------------------------------------------------------------

// public
void kpImageSelectionTransparency::setTransparent(bool yes)
{
    setOpaque(!yes);
}

//---------------------------------------------------------------------

// public
kpColor kpImageSelectionTransparency::transparentColor() const
{
    if (m_isOpaque) {
        // There are legitimate uses for this so no qCCritical(kpLogLayers)
        qCDebug(kpLogLayers) << "kpImageSelectionTransparency::transparentColor() "
                                "getting transparent color even though opaque";
    }

    return m_transparentColor;
}

//---------------------------------------------------------------------

// public
void kpImageSelectionTransparency::setTransparentColor(const kpColor &transparentColor)
{
    m_transparentColor = transparentColor;
}

//---------------------------------------------------------------------

// public
double kpImageSelectionTransparency::colorSimilarity() const
{
    if (m_colorSimilarity < 0 || m_colorSimilarity > kpColorSimilarityHolder::MaxColorSimilarity) {
        qCCritical(kpLogLayers) << "kpImageSelectionTransparency::colorSimilarity() invalid colorSimilarity";
        return 0;
    }

    return m_colorSimilarity;
}

//---------------------------------------------------------------------

// pubulic
void kpImageSelectionTransparency::setColorSimilarity(double colorSimilarity)
{
    m_colorSimilarity = colorSimilarity;
    m_processedColorSimilarity = kpColor::processSimilarity(colorSimilarity);
}

//---------------------------------------------------------------------

// public
int kpImageSelectionTransparency::processedColorSimilarity() const
{
    return m_processedColorSimilarity;
}

//---------------------------------------------------------------------
