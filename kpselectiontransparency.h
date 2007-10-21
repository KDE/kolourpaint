
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

#ifndef __kp_selection_transparency_h__
#define __kp_selection_transparency_h__

#include <kpcolor.h>


// This does not apply to the Text Tool.  Use kpTextStyle for that.
class kpSelectionTransparency
{
public:
    // Opaque selection
    kpSelectionTransparency ();
    // Selection that's transparent at pixels with <color>
    kpSelectionTransparency (const kpColor &transparentColor, double colorSimilarity);
    // If <isOpaque>, <transparentColor> is allowed to be anything
    // (including invalid) as the color would have no effect.
    // However, you are encouraged to set it as you would if !<isOpaque>,
    // because setTransparent(true) might be called later, after which
    // the <transparentColor> would suddenly become important.
    //
    // It is a similar case with <colorSimilarity>, although <colorSimilarity>
    // must be in-range (see kpColorSimilarityDialog).
    kpSelectionTransparency (bool isOpaque, const kpColor &transparentColor, double colorSimilarity);
    // Returns whether they are visually equivalent.
    // This is the same as a memcmp() except that if they are both opaque,
    // this function will return true regardless of the transparentColor's.
    bool operator== (const kpSelectionTransparency &rhs) const;
    bool operator!= (const kpSelectionTransparency &rhs) const;
    ~kpSelectionTransparency ();

    bool isOpaque () const;
    bool isTransparent () const;
    void setOpaque (bool yes = true);
    void setTransparent (bool yes = true);

    // If isOpaque(), transparentColor() is generally not called because
    // the transparent color would have no effect.
    kpColor transparentColor () const;
    void setTransparentColor (const kpColor &transparentColor);

    double colorSimilarity () const;
    void setColorSimilarity (double colorSimilarity);
    int processedColorSimilarity () const;

private:
    bool m_isOpaque;
    kpColor m_transparentColor;
    double m_colorSimilarity;
    int m_processedColorSimilarity;
};


#endif  // __kp_selection_transparency_h__
