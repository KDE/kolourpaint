
/*
   Copyright (c) 2007 Mike Gashler <gashlerm@yahoo.com>
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

// TODO: Clarence's code review

#include <kpEffectHSV.h>

#include <math.h>

#include <qbitmap.h>
#include <qimage.h>

#include <kdebug.h>

#include <kpPixmapFX.h>


static void ColorToHSV(unsigned int c, float* pHue, float* pSaturation, float* pValue)
{
    int r = qRed(c);
    int g = qGreen(c);
    int b = qBlue(c);
    int min;
    if(b >= g && b >= r)
    {
        // Blue
        min = qMin(r, g);
        if(b != min)
        {
            *pHue = (float)(r - g) / ((b - min) * 6) + (float)2 / 3;
            *pSaturation = (float)1 - (float)min / (float)b;
        }
        else
        {
            *pHue = 0;
            *pSaturation = 0;
        }
        *pValue = (float)b / 255;
    }
    else if(g >= r)
    {
        // Green
        min = qMin(b, r);
        if(g != min)
        {
            *pHue = (float)(b - r) / ((g - min) * 6) + (float)1 / 3;
            *pSaturation = (float)1 - (float)min / (float)g;
        }
        else
        {
            *pHue = 0;
            *pSaturation = 0;
        }
        *pValue = (float)g / 255;
    }
    else
    {
        // Red
        min = qMin(g, b);
        if(r != min)
        {
            *pHue = (float)(g - b) / ((r - min) * 6);
            if(*pHue < 0)
                (*pHue) += (float)1;
            *pSaturation = (float)1 - (float)min / (float)r;
        }
        else
        {
            *pHue = 0;
            *pSaturation = 0;
        }
        *pValue = (float)r / 255;
    }
}

static unsigned int HSVToColor(int alpha, float hue, float saturation, float value)
{
    //Q_ASSERT (hue >= 0 && hue <= 1 && saturation >= 0 && saturation <= 1 && value >= 0 && value <= 1);

    hue *= (float)5.999999;
    int h = (int)hue;
    float f = hue - h;
    float p = value * ((float)1 - saturation);
    float q = value * ((float)1 - ((h & 1) == 0 ? (float)1 - f : f) * saturation);
    switch(h)
    {
        case 0: return qRgba((int)(value * 255.999999), (int)(q * 255.999999), (int)(p * 255.999999), alpha);
        case 1: return qRgba((int)(q * 255.999999), (int)(value * 255.999999), (int)(p * 255.999999), alpha);
        case 2: return qRgba((int)(p * 255.999999), (int)(value * 255.999999), (int)(q * 255.999999), alpha);
        case 3: return qRgba((int)(p * 255.999999), (int)(q * 255.999999), (int)(value * 255.999999), alpha);
        case 4: return qRgba((int)(q * 255.999999), (int)(p * 255.999999), (int)(value * 255.999999), alpha);
        case 5: return qRgba((int)(value * 255.999999), (int)(p * 255.999999), (int)(q * 255.999999), alpha);
    }
    return qRgba(0, 0, 0, alpha);
}

static QRgb AdjustHSVInternal (QRgb pix, double hueDiv360, double saturation, double value)
{
    float h, s, v;
    ::ColorToHSV(pix, &h, &s, &v);
    
    const int alpha = qAlpha(pix);

    h += (float)hueDiv360;
    h -= floor(h);

    s = qMax((float)0, qMin((float)1, s + (float)saturation));

    v = qMax((float)0, qMin((float)1, v + (float)value));

    return ::HSVToColor(alpha, h, s, v);
}

static void AdjustHSV (QImage* pImage, double hue, double saturation, double value)
{
    hue /= 360;

    if (pImage->depth () > 8)
    {
        for (int y = 0; y < pImage->height (); y++)
        {
            for (int x = 0; x < pImage->width (); x++)
            {
                QRgb pix = pImage->pixel (x, y);
                pix = ::AdjustHSVInternal (pix, hue, saturation, value);
                pImage->setPixel (x, y, pix);
            }
        }
    }
    else
    {
        for (int i = 0; i < pImage->numColors (); i++)
        {
            QRgb pix = pImage->color (i);
            pix = ::AdjustHSVInternal (pix, hue, saturation, value);
            pImage->setColor (i, pix);
        }
    }
}

// public static
kpImage kpEffectHSV::applyEffect (const kpImage &image,
                                  double hue, double saturation, double value)
{
    QImage qimage(image);
    ::AdjustHSV (&qimage, hue, saturation, value);
    return qimage;
}

