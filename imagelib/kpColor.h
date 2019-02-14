
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


#ifndef KP_COLOR_H
#define KP_COLOR_H


#include <QColor>


class QDataStream;


//
// kpColor is an object-oriented abstraction of QRgb, for document image data.
// In the future, other color models such as
// 8-bit indexed will be supported.  It also provides better error handling,
// reporting (noisy qCCritical(kpLogImagelib)'s) and recovery compared to Qt.  This abstraction
// will allow us to eventually dump the Qt paint routines.
//
// In general, you should pass around kpColor objects instead of QRgb
// and QColor.  Only convert an opaque kpColor to a QColor (using toQColor())
// if you need to draw something on-screen.
//
// Constructing a kpColor object from QColor is usually wrong since QColor's
// come from on-screen pixels, which may lack the full color resolution of
// kpColor, due to the limited color range on e.g. a 16-bit screen.
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


//
// Constants
//
public:
    // "lhs.isSimilarTo (rhs, kpColor::Exact)" is exactly the same as calling
    // "lhs == rhs".
    static const int Exact;
  
    static const kpColor Invalid;
    static const kpColor Transparent;


    //
    // Primary Colors + B&W
    //
    
    static const kpColor Red, Green, Blue;
    static const kpColor Black, White;


    //
    // Full-brightness Colors
    //

    static const kpColor Yellow, Purple, Aqua;


    //
    // Mixed Colors
    //
    
    static const kpColor Gray, LightGray, Orange;
     

    //
    // Pastel Colors
    //

    static const kpColor Pink, LightGreen, LightBlue, Tan;


    //
    // Dark Colors
    //
    
    static const kpColor DarkRed;

    // (identical)
    static const kpColor DarkOrange, Brown;

    static const kpColor DarkYellow, DarkGreen, DarkAqua, DarkBlue,
        DarkPurple, DarkGray;


public:
    static int processSimilarity (double colorSimilarity);
    // Usage: isSimilarTo (rhs, kpColor::processSimilarity (.1)) checks for
    //        Color Similarity within 10%
    bool isSimilarTo (const kpColor &rhs, int processedSimilarity) const;

    bool isValid () const;

    int red () const;
    int green () const;
    int blue () const;
    int alpha () const;
    bool isTransparent () const;

    // Cast operators will most likely result in careless conversions so
    // use explicit functions instead:
    QRgb toQRgb () const;

    QColor toQColor () const;

private:
    // Catch accidental call to "const QRgb &rgba" (unsigned int) ctor
    // by e.g. "kpColor(Qt::black)" (Qt::black is an enum element that can cast
    // to "unsigned int").
    kpColor (Qt::GlobalColor color);
    
    bool m_rgbaIsValid;
    QRgb m_rgba;

    mutable bool m_colorCacheIsValid;
    mutable QColor m_colorCache;
};


#endif  // KP_COLOR_H
