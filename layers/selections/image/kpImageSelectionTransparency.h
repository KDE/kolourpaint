
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_IMAGE_SELECTION_TRANSPARENCY_H
#define KP_IMAGE_SELECTION_TRANSPARENCY_H

#include "imagelib/kpColor.h"

// This does not apply to the Text Tool.  Use kpTextStyle for that.
class kpImageSelectionTransparency
{
public:
    // Opaque selection
    kpImageSelectionTransparency();
    // Selection that's transparent at pixels with <color>
    kpImageSelectionTransparency(const kpColor &transparentColor, double colorSimilarity);
    // If <isOpaque>, <transparentColor> is allowed to be anything
    // (including invalid) as the color would have no effect.
    // However, you are encouraged to set it as you would if !<isOpaque>,
    // because setTransparent(true) might be called later, after which
    // the <transparentColor> would suddenly become important.
    //
    // It is a similar case with <colorSimilarity>, although <colorSimilarity>
    // must be in-range (see kpColorSimilarityHolder).
    kpImageSelectionTransparency(bool isOpaque, const kpColor &transparentColor, double colorSimilarity);
    // Returns whether they are visually equivalent.
    // This is the same as a memcmp() except that if they are both opaque,
    // this function will return true regardless of the transparentColor's.
    bool operator==(const kpImageSelectionTransparency &rhs) const;
    bool operator!=(const kpImageSelectionTransparency &rhs) const;
    ~kpImageSelectionTransparency();

    bool isOpaque() const;
    bool isTransparent() const;
    void setOpaque(bool yes = true);
    void setTransparent(bool yes = true);

    // If isOpaque(), transparentColor() is generally not called because
    // the transparent color would have no effect.
    kpColor transparentColor() const;
    void setTransparentColor(const kpColor &transparentColor);

    double colorSimilarity() const;
    void setColorSimilarity(double colorSimilarity);
    int processedColorSimilarity() const;

private:
    bool m_isOpaque;
    kpColor m_transparentColor;
    double m_colorSimilarity;
    int m_processedColorSimilarity;
};

#endif // KP_IMAGE_SELECTION_TRANSPARENCY_H
