
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kpselectiontransparency.h>

#include <kdebug.h>

#include <kpcolor.h>
#include <kpcolorsimilaritydialog.h>


kpSelectionTransparency::kpSelectionTransparency ()
    : m_isOpaque (true)
{
    setColorSimilarity (0);
}

kpSelectionTransparency::kpSelectionTransparency (const kpColor &transparentColor, double colorSimilarity)
    : m_isOpaque (false),
      m_transparentColor (transparentColor)
{
    setColorSimilarity (colorSimilarity);
}

kpSelectionTransparency::kpSelectionTransparency (bool isOpaque, const kpColor &transparentColor,
                                                  double colorSimilarity)
    : m_isOpaque (isOpaque),
      m_transparentColor (transparentColor)
{
    setColorSimilarity (colorSimilarity);
}

bool kpSelectionTransparency::operator== (const kpSelectionTransparency &rhs) const
{
#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
    kdDebug () << "kpSelectionTransparency::operator==()" << endl;
#endif
    
    if (m_isOpaque != rhs.m_isOpaque)
    {
    #if DEBUG_KP_SELECTION_TRANSPARENCY && 0
        kdDebug () << "\tdifferent opacity: lhs=" << m_isOpaque
                   << " rhs=" << rhs.m_isOpaque
                   << endl;
    #endif
        return false;
    }

    if (m_isOpaque)
    {
    #if DEBUG_KP_SELECTION_TRANSPARENCY && 0
        kdDebug () << "\tboth opaque - eq" << endl;
    #endif
        return true;
    }

#if DEBUG_KP_SELECTION_TRANSPARENCY && 0
    kdDebug () << "\tcolours: lhs=" << (int *) m_transparentColor.toQRgb ()
               << " rhs=" << (int *) rhs.m_transparentColor.toQRgb ()
               << endl;
    kdDebug () << "\tcolour similarity: lhs=" << m_colorSimilarity
               << " rhs=" << rhs.m_colorSimilarity
               << endl;
#endif
    
    return (m_transparentColor == rhs.m_transparentColor &&
            m_colorSimilarity == rhs.m_colorSimilarity);
}

bool kpSelectionTransparency::operator!= (const kpSelectionTransparency &rhs) const
{
    return !(*this == rhs);
}

kpSelectionTransparency::~kpSelectionTransparency ()
{
}


// public
bool kpSelectionTransparency::isOpaque () const
{
    return m_isOpaque;
}

// public
bool kpSelectionTransparency::isTransparent () const
{
    return !isOpaque ();
}

// public
void kpSelectionTransparency::setOpaque (bool yes)
{
    m_isOpaque = yes;
}

// public
void kpSelectionTransparency::setTransparent (bool yes)
{
    setOpaque (!yes);
}


// public
kpColor kpSelectionTransparency::transparentColor () const
{
    if (m_isOpaque)
    {
        // There are legitimate uses for this so no kdError()
        kdDebug () << "kpSelectionTransparency::transparentColor() "
                      "getting transparent color even though opaque" << endl;
    }

    return m_transparentColor;
}

// public
void kpSelectionTransparency::setTransparentColor (const kpColor &transparentColor)
{
    m_transparentColor = transparentColor;
}


// public
double kpSelectionTransparency::colorSimilarity () const
{
    if (m_colorSimilarity < 0 ||
        m_colorSimilarity > kpColorSimilarityDialog::maximumColorSimilarity)
    {
        kdError () << "kpSelectionTransparency::colorSimilarity() invalid colorSimilarity" << endl;
        return 0;
    }

    return m_colorSimilarity;
}

// pubulic
void kpSelectionTransparency::setColorSimilarity (double colorSimilarity)
{
    m_colorSimilarity = colorSimilarity;
    m_processedColorSimilarity = kpColor::processSimilarity (colorSimilarity);
}

// public
int kpSelectionTransparency::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}

