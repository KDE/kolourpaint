
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


#define DEBUG_KP_COLOR 0


#include <kpcolor.h>

#include <qdatastream.h>

#include <kdebug.h>


// public static
const int kpColor::Exact = 0;

// public static
const kpColor kpColor::invalid;  // TODO: what's wrong with explicitly specifying () constructor?
const kpColor kpColor::transparent (0, 0, 0, true/*isTransparent*/);


kpColor::kpColor ()
    : m_rgbaIsValid (false),
      m_colorCacheIsValid (false)
{
}

kpColor::kpColor (int red, int green, int blue, bool isTransparent)
    : m_colorCacheIsValid (false)
{
    if (red < 0 || red > 255 ||
        green < 0 || green > 255 ||
        blue < 0 || blue > 255)
    {
        kdError () << "kpColor::<ctor>(r=" << red
                   << ",g=" << green
                   << ",b=" << blue
                   << ",t=" << isTransparent
                   << ") passed out of range values" << endl;
        m_rgbaIsValid = false;
        return;
    }

    m_rgba = qRgba (red, green, blue, isTransparent ? 0 : 255/*opaque*/);
    m_rgbaIsValid = true;
}

kpColor::kpColor (const QRgb &rgba)
    : m_colorCacheIsValid (false)
{
    if (qAlpha (rgba) > 0 && qAlpha (rgba) < 255)
    {
        kdError () << "kpColor::<ctor>(QRgb) passed translucent alpha "
                   << qAlpha (rgba)
                   << " - trying to recover"
                   << endl;

        // Forget the alpha channel - make it opaque
        m_rgba = qRgb (qRed (m_rgba), qGreen (m_rgba), qBlue (m_rgba));
        m_rgbaIsValid = true;
    }
    else
    {
        m_rgba = rgba;
        m_rgbaIsValid = true;
    }
}

kpColor::kpColor (const kpColor &rhs)
    :  m_rgbaIsValid (rhs.m_rgbaIsValid),
       m_rgba (rhs.m_rgba),
       m_colorCacheIsValid (rhs.m_colorCacheIsValid),
       m_colorCache (rhs.m_colorCache)
{
}

// friend
QDataStream &operator<< (QDataStream &stream, const kpColor &color)
{
    stream << int (color.m_rgbaIsValid) << int (color.m_rgba);

    return stream;
}

// friend
QDataStream &operator>> (QDataStream &stream, kpColor &color)
{
    int a, b;
    stream >> a >> b;
    color.m_rgbaIsValid = a;
    color.m_rgba = b;

    color.m_colorCacheIsValid = false;

    return stream;
}

kpColor &kpColor::operator= (const kpColor &rhs)
{
    // (as soon as you add a ptr, you won't be complaining to me that this
    //  method was unnecessary :))

    if (this == &rhs)
        return *this;

    m_rgbaIsValid = rhs.m_rgbaIsValid;
    m_rgba = rhs.m_rgba;
    m_colorCacheIsValid = rhs.m_colorCacheIsValid;
    m_colorCache = rhs.m_colorCache;

    return *this;
}

bool kpColor::operator== (const kpColor &rhs) const
{
    return isSimilarTo (rhs, kpColor::Exact);
}

bool kpColor::operator!= (const kpColor &rhs) const
{
    return !(*this == rhs);
}


template <class dtype>
inline dtype square (dtype val)
{
    return val * val;
}

// public static
int kpColor::processSimilarity (double colorSimilarity)
{
    // sqrt (dr ^ 2 + dg ^ 2 + db ^ 2) <= colorSimilarity       * sqrt (255 ^ 2 * 3)
    //       dr ^ 2 + dg ^ 2 + db ^ 2  <= (colorSimilarity ^ 2) * (255 ^ 2 * 3)

    return int (square (colorSimilarity) * (square (255) * 3));
}

bool kpColor::isSimilarTo (const kpColor &rhs, int processedSimilarity) const
{
    // Are we the same?
    if (this == &rhs)
        return true;


    // Do we dither in terms of validity?
    if (isValid () != rhs.isValid ())
        return false;

    // Are both of us invalid?
    if (!isValid ())
        return true;

    // --- both are now valid ---


    if (isTransparent () != rhs.isTransparent ())
        return false;

    // Are both of us transparent?
    if (isTransparent ())
        return true;

    // --- both are now valid and opaque ---


    if (m_rgba == rhs.m_rgba)
        return true;


    if (processedSimilarity == kpColor::Exact)
        return false;
    else
    {
        return (square (qRed (m_rgba) - qRed (rhs.m_rgba)) +
                square (qGreen (m_rgba) - qGreen (rhs.m_rgba)) +
                square (qBlue (m_rgba) - qBlue (rhs.m_rgba))
                <= processedSimilarity);
    }
}

kpColor::~kpColor ()
{
}


// public
bool kpColor::isValid () const
{
    return m_rgbaIsValid;
}


// public
int kpColor::red () const
{
    if (!m_rgbaIsValid)
    {
        kdError () << "kpColor::red() called with invalid kpColor" << endl;
        return 0;
    }

    if (isTransparent ())
    {
        kdError () << "kpColor::red() called with transparent kpColor" << endl;
        return 0;
    }

    return qRed (m_rgba);
}

// public
int kpColor::green () const
{
    if (!m_rgbaIsValid)
    {
        kdError () << "kpColor::green() called with invalid kpColor" << endl;
        return 0;
    }

    if (isTransparent ())
    {
        kdError () << "kpColor::green() called with transparent kpColor" << endl;
        return 0;
    }

    return qGreen (m_rgba);
}

// public
int kpColor::blue () const
{
    if (!m_rgbaIsValid)
    {
        kdError () << "kpColor::blue() called with invalid kpColor" << endl;
        return 0;
    }

    if (isTransparent ())
    {
        kdError () << "kpColor::blue() called with transparent kpColor" << endl;
        return 0;
    }

    return qBlue (m_rgba);
}

// public
int kpColor::alpha () const
{
    if (!m_rgbaIsValid)
    {
        kdError () << "kpColor::alpha() called with invalid kpColor" << endl;
        return 0;
    }

    const int alpha = qAlpha (m_rgba);

    if (alpha > 0 && alpha < 255)
    {
        kdError () << "kpColor::alpha() called with translucent kpColor alpha=" << alpha << endl;

        // no translucency
        return alpha ? 255 : 0;
    }
    else
    {
        return alpha;
    }
}

// public
bool kpColor::isTransparent () const
{
    return (alpha () == 0);
}

// public
bool kpColor::isOpaque () const
{
    return (alpha () == 255);
}


// public
QRgb kpColor::toQRgb () const
{
    if (!m_rgbaIsValid)
    {
        kdError () << "kpColor::toQRgb() called with invalid kpColor" << endl;
        return 0;
    }

    return m_rgba;
}

// public
const QColor &kpColor::toQColor () const
{
    if (!m_rgbaIsValid)
    {
        kdError () << "kpColor::toQColor() called with invalid kpColor" << endl;
        return Qt::black;
    }

    if (m_colorCacheIsValid)
        return m_colorCache;

    if (qAlpha (m_rgba) < 255)
    {
        kdError () << "kpColor::toQColor() called with not fully opaque kpColor alpha="
                   << qAlpha (m_rgba)
                   << endl;
        return Qt::black;
    }

    m_colorCache = QColor (m_rgba);
    if (!m_colorCache.isValid ())
    {
        kdError () << "kpColor::toQColor () internal error - could not return valid QColor"
                   << endl;
        return Qt::black;
    }

    m_colorCacheIsValid = true;

    return m_colorCache;
}

// public
QColor kpColor::maskColor () const
{
    return isTransparent () ? Qt::color0 : Qt::color1;
}
