
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


#ifndef __kp_color_h__
#define __kp_color_h__


#include <qcolor.h>

class QDataStream;


//
// kpColor is an object-oriented abstraction of QRgb, with the additional
// restriction of following the KolourPaint convention of only supporting
// totally transparent and totally opaque colors.  It also provides better
// error handling, reporting (noisy kdError()'s) and recovery.
//
// In general, you should pass around kpColor objects instead of QRgb
// and QColor.  Only convert an opaque kpColor to a QColor (using toQColor())
// if you need to draw something onscreen.  Constructing a kpColor object
// from QColor is probably wrong since onscreen representations of color
// are not guaranteed to be faithful (due to QColor color allocation).
//
class kpColor
{
public:
    kpColor ();
    kpColor (int red, int green, int blue, bool isTransparent = false);
    kpColor (const QRgb &rgba);
    kpColor (const kpColor &rhs);
    friend QDataStream &operator<< (QDataStream &stream, const kpColor &color);
    friend QDataStream &operator>> (QDataStream &stream, kpColor &color);
    kpColor &operator= (const kpColor &rhs);
    bool operator== (const kpColor &rhs) const;
    bool operator!= (const kpColor &rhs) const;

    static int processSimilarity (double colorSimilarity);
    static const int Exact;  // "isSimilarTo (rhs, kpColor::Exact)" == "== rhs"
    // Usage: isSimilarTo (rhs, kpColor::processSimilarity (.1)) checks for
    //        Color Similarity within 10%
    bool isSimilarTo (const kpColor &rhs, int processedSimilarity) const;
    ~kpColor ();

    static const kpColor invalid;
    static const kpColor transparent;

    bool isValid () const;

    int red () const;
    int green () const;
    int blue () const;
    int alpha () const;
    bool isTransparent () const;
    bool isOpaque () const;

    // Cast operators will most likely result in careless conversions so
    // use explicit functions instead:
    QRgb toQRgb () const;

    // (only valid if isOpaque())
    // (const QColor & return results in fewer color reallocations)
    const QColor &toQColor () const;

    QColor maskColor () const;

private:
    bool m_rgbaIsValid;
    QRgb m_rgba;

    mutable bool m_colorCacheIsValid;
    mutable QColor m_colorCache;
};


#endif  // __kp_color_h__
