
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


#define DEBUG_KP_SELECTION_TRANSPARENCY 0


#include "layers/selections/image/kpImageSelectionTransparency.h"

#include "kpLogCategories.h"

#include "widgets/colorSimilarity/kpColorSimilarityHolder.h"


//---------------------------------------------------------------------

kpImageSelectionTransparency::kpImageSelectionTransparency ()
    : m_isOpaque (true)
{
    setColorSimilarity (0);
}

//---------------------------------------------------------------------

kpImageSelectionTransparency::kpImageSelectionTransparency (const kpColor &transparentColor, double colorSimilarity)
    : m_isOpaque (false),
      m_transparentColor (transparentColor)
{
    setColorSimilarity (colorSimilarity);
}

//---------------------------------------------------------------------

kpImageSelectionTransparency::kpImageSelectionTransparency (bool isOpaque, const kpColor &transparentColor,
                                                  double colorSimilarity)
    : m_isOpaque (isOpaque),
      m_transparentColor (transparentColor)
{
    setColorSimilarity (colorSimilarity);
}

//---------------------------------------------------------------------

bool kpImageSelectionTransparency::operator== (const kpImageSelectionTransparency &rhs) const
{
#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
    qCDebug(kpLogLayers) << "kpImageSelectionTransparency::operator==()";
#endif
    
    if (m_isOpaque != rhs.m_isOpaque)
    {
    #if DEBUG_KP_SELECTION_TRANSPARENCY && 0
        qCDebug(kpLogLayers) << "\tdifferent opacity: lhs=" << m_isOpaque
                   << " rhs=" << rhs.m_isOpaque
                   << endl;
    #endif
        return false;
    }

    if (m_isOpaque)
    {
    #if DEBUG_KP_SELECTION_TRANSPARENCY && 0
        qCDebug(kpLogLayers) << "\tboth opaque - eq";
    #endif
        return true;
    }

#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
    qCDebug(kpLogLayers) << "\tcolours: lhs=" << (int *) m_transparentColor.toQRgb ()
               << " rhs=" << (int *) rhs.m_transparentColor.toQRgb ()
               << endl;
    qCDebug(kpLogLayers) << "\tcolour similarity: lhs=" << m_colorSimilarity
               << " rhs=" << rhs.m_colorSimilarity
               << endl;
#endif
    
    return (m_transparentColor == rhs.m_transparentColor &&
            m_colorSimilarity == rhs.m_colorSimilarity);
}

//---------------------------------------------------------------------

bool kpImageSelectionTransparency::operator!= (const kpImageSelectionTransparency &rhs) const
{
    return !(*this == rhs);
}

//---------------------------------------------------------------------

kpImageSelectionTransparency::~kpImageSelectionTransparency () = default;

//---------------------------------------------------------------------

// public
bool kpImageSelectionTransparency::isOpaque () const
{
    return m_isOpaque;
}

//---------------------------------------------------------------------

// public
bool kpImageSelectionTransparency::isTransparent () const
{
    return !isOpaque ();
}

//---------------------------------------------------------------------

// public
void kpImageSelectionTransparency::setOpaque (bool yes)
{
    m_isOpaque = yes;
}

//---------------------------------------------------------------------

// public
void kpImageSelectionTransparency::setTransparent (bool yes)
{
    setOpaque (!yes);
}

//---------------------------------------------------------------------


// public
kpColor kpImageSelectionTransparency::transparentColor () const
{
    if (m_isOpaque)
    {
        // There are legitimate uses for this so no qCCritical(kpLogLayers)
        qCDebug(kpLogLayers) << "kpImageSelectionTransparency::transparentColor() "
                      "getting transparent color even though opaque";
    }

    return m_transparentColor;
}

//---------------------------------------------------------------------

// public
void kpImageSelectionTransparency::setTransparentColor (const kpColor &transparentColor)
{
    m_transparentColor = transparentColor;
}

//---------------------------------------------------------------------


// public
double kpImageSelectionTransparency::colorSimilarity () const
{
    if (m_colorSimilarity < 0 ||
        m_colorSimilarity > kpColorSimilarityHolder::MaxColorSimilarity)
    {
        qCCritical(kpLogLayers) << "kpImageSelectionTransparency::colorSimilarity() invalid colorSimilarity";
        return 0;
    }

    return m_colorSimilarity;
}

//---------------------------------------------------------------------

// pubulic
void kpImageSelectionTransparency::setColorSimilarity (double colorSimilarity)
{
    m_colorSimilarity = colorSimilarity;
    m_processedColorSimilarity = kpColor::processSimilarity (colorSimilarity);
}

//---------------------------------------------------------------------

// public
int kpImageSelectionTransparency::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}

//---------------------------------------------------------------------
