// functions taken from qimageblitz (no longer maintained)

/* 
 Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005, 2007
      Daniel M. Duley <daniel.duley@verizon.net>
 (C) 2004 Zack Rusin <zack@kde.org>
 (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
 (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
 (C) 1998, 1999 Dirk Mueller <mueller@kde.org>

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

/*
 Portions of this software were originally based on ImageMagick's
 algorithms. ImageMagick is copyrighted under the following conditions:

Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated to
making software imaging solutions freely available.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files ("ImageMagick"), to deal
in ImageMagick without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense,  and/or sell
copies of ImageMagick, and to permit persons to whom the ImageMagick is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of ImageMagick.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.  In no event shall
ImageMagick Studio be liable for any claim, damages or other liability,
whether in an action of contract, tort or otherwise, arising from, out of or
in connection with ImageMagick or the use or other dealings in ImageMagick.

Except as contained in this notice, the name of the ImageMagick Studio shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in ImageMagick without prior written authorization from the
ImageMagick Studio.
*/

#include "blitz.h"

#include <QColor>
#include <cmath>

#define M_SQ2PI 2.50662827463100024161235523934010416269302368164062
#define M_EPSILON 1.0e-6

#define CONVOLVE_ACC(weight, pixel) \
    r+=((weight))*(qRed((pixel))); g+=((weight))*(qGreen((pixel))); \
    b+=((weight))*(qBlue((pixel)));

//--------------------------------------------------------------------------------

inline QRgb convertFromPremult(QRgb p)
{
    int alpha = qAlpha(p);
    return(!alpha ? 0 : qRgba(255*qRed(p)/alpha,
                              255*qGreen(p)/alpha,
                              255*qBlue(p)/alpha,
                              alpha));
}

//--------------------------------------------------------------------------------

inline QRgb convertToPremult(QRgb p)
{
    unsigned int a = p >> 24;
    unsigned int t = (p & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    p = ((p >> 8) & 0xff) * a;
    p = (p + ((p >> 8) & 0xff) + 0x80);
    p &= 0xff00;
    p |= t | (a << 24);
    return(p);
}

//--------------------------------------------------------------------------------
// These are used as accumulators

typedef struct
{
    quint32 red, green, blue, alpha;
} IntegerPixel;

typedef struct
{
    // Yes, a normal pixel can be used instead but this is easier to read
    // and no shifts to get components.
    quint8 red, green, blue, alpha;
} CharPixel;

typedef struct
{
    quint32 red, green, blue, alpha;
} HistogramListItem;


bool equalize(QImage &img)
{
    if(img.isNull()) {
        return(false);
    }

    HistogramListItem *histogram;
    IntegerPixel *map;
    IntegerPixel intensity, high, low;
    CharPixel *equalize_map;
    int i, count;
    QRgb pixel, *dest;
    unsigned char r, g, b;

    if(img.depth() < 32){
        img = img.convertToFormat(img.hasAlphaChannel() ?
                                  QImage::Format_ARGB32 :
                                  QImage::Format_RGB32);
    }
    count = img.width()*img.height();

    map = new IntegerPixel[256];
    histogram = new HistogramListItem[256];
    equalize_map = new CharPixel[256];

    // form histogram
    memset(histogram, 0, 256*sizeof(HistogramListItem));
    dest = reinterpret_cast<QRgb *>(img.bits());

    if(img.format() == QImage::Format_ARGB32_Premultiplied){
        for(i=0; i < count; ++i, ++dest){
            pixel = convertFromPremult(*dest);
            histogram[qRed(pixel)].red++;
            histogram[qGreen(pixel)].green++;
            histogram[qBlue(pixel)].blue++;
            histogram[qAlpha(pixel)].alpha++;
        }
    }
    else{
        for(i=0; i < count; ++i){
            pixel = *dest++;
            histogram[qRed(pixel)].red++;
            histogram[qGreen(pixel)].green++;
            histogram[qBlue(pixel)].blue++;
            histogram[qAlpha(pixel)].alpha++;
        }
    }

    // integrate the histogram to get the equalization map
    memset(&intensity, 0, sizeof(IntegerPixel));
    for(i=0; i < 256; ++i){
        intensity.red += histogram[i].red;
        intensity.green += histogram[i].green;
        intensity.blue += histogram[i].blue;
        map[i] = intensity;
    }

    low = map[0];
    high = map[255];
    memset(equalize_map, 0, 256*sizeof(CharPixel));
    for(i=0; i < 256; ++i){
        if(high.red != low.red) {
            equalize_map[i].red = static_cast<unsigned char>
                    ((255*(map[i].red-low.red))/(high.red-low.red));
        }
        if(high.green != low.green) {
            equalize_map[i].green = static_cast<unsigned char>
                    ((255*(map[i].green-low.green))/(high.green-low.green));
        }
        if(high.blue != low.blue) {
            equalize_map[i].blue = static_cast<unsigned char>
                    ((255*(map[i].blue-low.blue))/(high.blue-low.blue));
        }
    }

    // stretch the histogram and write
    dest = reinterpret_cast<QRgb *>(img.bits());
    if(img.format() == QImage::Format_ARGB32_Premultiplied){
        for(i=0; i < count; ++i, ++dest){
            pixel = convertFromPremult(*dest);
            r = static_cast<unsigned char> ((low.red != high.red) ?
                                                equalize_map[qRed(pixel)].red : qRed(pixel));

            g = static_cast<unsigned char> ((low.green != high.green) ?
                                                equalize_map[qGreen(pixel)].green : qGreen(pixel));

            b = static_cast<unsigned char> ((low.blue != high.blue) ?
                                                equalize_map[qBlue(pixel)].blue : qBlue(pixel));

            *dest = convertToPremult(qRgba(r, g, b, qAlpha(pixel)));
        }
    }
    else{
        for(i=0; i < count; ++i){
            pixel = *dest;
            r = static_cast<unsigned char> ((low.red != high.red) ?
                                                equalize_map[qRed(pixel)].red : qRed(pixel));

            g = static_cast<unsigned char> ((low.green != high.green) ?
                                                equalize_map[qGreen(pixel)].green : qGreen(pixel));

            b = static_cast<unsigned char> ((low.blue != high.blue) ?
                                                equalize_map[qBlue(pixel)].blue : qBlue(pixel));

            *dest++ = qRgba(r, g, b, qAlpha(pixel));
        }
    }

    delete[] histogram;
    delete[] map;
    delete[] equalize_map;
    return(true);
}

//--------------------------------------------------------------------------------

QImage Blitz::blur(QImage &img, int radius)
{
    if (img.isNull()) {
        return (img);
    }

    if (img.depth() < 8) {
        img = img.convertToFormat(QImage::Format_Indexed8);
    }

    QVector<QRgb> colorTable;
    if (img.format() == QImage::Format_Indexed8) {
        colorTable = img.colorTable();
    }

    auto width = img.width();
    auto height = img.height();

    QImage buffer(width, height, img.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

    const auto img_format = img.format();

    int *as = new int[width];
    int *rs = new int[width];
    int *gs = new int[width];
    int *bs = new int[width];

    QRgb *p1 , *p2;

    for (auto y = 0; y < height; ++y) {
        auto my = y - radius;
        auto mh = (radius << 1) + 1;

        if (my < 0) {
            mh += my;
            my = 0;
        }

        if ((my + mh) > height) {
            mh = height - my;
        }

        p1 = reinterpret_cast<QRgb *>(buffer.scanLine(y));

        memset(as, 0, static_cast<unsigned int>(width) * sizeof(int));
        memset(rs, 0, static_cast<unsigned int>(width) * sizeof(int));
        memset(gs, 0, static_cast<unsigned int>(width) * sizeof(int));
        memset(bs, 0, static_cast<unsigned int>(width) * sizeof(int));


        switch (img_format) {
        case QImage::Format_ARGB32_Premultiplied: {
            QRgb pixel;
            for (auto i = 0; i < mh; i++) {
                p2 = reinterpret_cast<QRgb *>(img.scanLine(i + my));
                for (auto j = 0; j < width; ++j) {
                    p2++;
                    pixel = convertFromPremult(*p2);
                    as[j] += qAlpha(pixel);
                    rs[j] += qRed(pixel) * qRed(pixel);
                    gs[j] += qGreen(pixel) * qGreen(pixel);
                    bs[j] += qBlue(pixel) * qBlue(pixel);
                }
            }
            break;
        }

        case QImage::Format_Indexed8: {
            QRgb pixel;
            unsigned char *ptr;
            for (auto i = 0; i < mh; ++i) {
                ptr = img.scanLine(i + my);
                for (auto j = 0; j < width; ++j) {
                    ptr++;
                    pixel = colorTable[*ptr];
                    as[j] += qAlpha(pixel);
                    rs[j] += qRed(pixel) * qRed(pixel);
                    gs[j] += qGreen(pixel) * qGreen(pixel);
                    bs[j] += qBlue(pixel) * qBlue(pixel);
                }
            }
            break;
        }

        default: {
            for (auto i = 0; i < mh; ++i) {
                p2 = reinterpret_cast<QRgb *>(img.scanLine(i + my));
                for (auto j = 0; j < width; j++) {
                    p2++;
                    as[j] += qAlpha(*p2);
                    rs[j] += qRed(*p2);
                    gs[j] += qGreen(*p2);
                    bs[j] += qBlue(*p2);
                }
            }
            break;
        }
        }

        for (auto i = 0; i < width; ++i) {
            auto a{0};
            auto r{0};
            auto g{0};
            auto b{0};

            auto mx = i - radius;
            auto mw = (radius << 1) + 1;

            if (mx < 0) {
                mw += mx;
                mx = 0;
            }

            if ((mx + mw) > width) {
                mw = width - mx;
            }

            for (auto j = mx; j < (mw + mx); ++j) {
                a += as[j];
                r += rs[j];
                g += gs[j];
                b += bs[j];
            }

            auto mt = mw * mh;

            a = a / mt;
            r = r / mt;
            g = g / mt;
            b = b / mt;

            *p1++ = qRgba(std::sqrt(r), std::sqrt(g), std::sqrt(b), a);
        }
    }

    delete[] as;
    delete[] rs;
    delete[] gs;
    delete[] bs;

    return (buffer);
}

//--------------------------------------------------------------------------------

int defaultConvolveMatrixSize(float radius, float sigma, bool quality)
{
    int i, matrix_size;
    float normalize, value;
    float sigma2 = sigma*sigma*2.0f;
    float sigmaSQ2PI = static_cast<float>(M_SQ2PI) * sigma;
    int max = quality ? 65535 : 255;

    if(sigma == 0.0f){
        qWarning("Blitz::defaultConvolveMatrixSize(): Zero sigma is invalid!");
        return(5);
    }

    if(radius > 0.0f) {
        return(static_cast<int>(2.0f * std::ceil(radius) + 1.0f));
    }

    matrix_size = 5;
    do{
        normalize = 0.0;
        for(i=(-matrix_size/2); i <= (matrix_size/2); ++i) {
            normalize += std::exp(-(static_cast<float> (i*i))/sigma2) / sigmaSQ2PI;
        }
        i = matrix_size/2;
        value = std::exp(-(static_cast<float> (i*i))/sigma2) / sigmaSQ2PI / normalize;
        matrix_size += 2;
    } while(static_cast<int>(max*value) > 0);

    matrix_size-=4;
    return(matrix_size);
}

//--------------------------------------------------------------------------------

QImage convolve(QImage &img, int matrix_size, float *matrix)
{
    int i, x, y, w, h, matrix_x, matrix_y;
    int edge = matrix_size/2;
    QRgb *dest, *src, *s, **scanblock;
    float *m, *normalize_matrix, normalize;

    if(!(matrix_size % 2)){
        qWarning("Blitz::convolve(): kernel width must be an odd number!");
        return(img);
    }

    w = img.width();
    h = img.height();
    if(w < 3 || h < 3){
        qWarning("Blitz::convolve(): Image is too small!");
        return(img);
    }

    if(img.format() == QImage::Format_ARGB32_Premultiplied) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }
    else if(img.depth() < 32){ 
        img = img.convertToFormat(img.hasAlphaChannel() ?
                                  QImage::Format_ARGB32 :
                                  QImage::Format_RGB32);
    }
    QImage buffer(w, h, img.format());

    scanblock = new QRgb* [matrix_size];
    normalize_matrix = new float[matrix_size*matrix_size];

    // create normalized matrix
    normalize = 0.0;
    for(i=0; i < matrix_size*matrix_size; ++i) {
        normalize += matrix[i];
    }
    if(std::abs(normalize) <=  static_cast<float> (M_EPSILON)) {
        normalize = 1.0f;
    }
    normalize = 1.0f/normalize;
    for(i=0; i < matrix_size*matrix_size; ++i){
        normalize_matrix[i] = normalize*matrix[i];
    }

    // apply

    {
        //
        //
        // Non-MMX version
        //
        //

        float r, g, b;
        for(y=0; y < h; ++y){
            src = reinterpret_cast<QRgb *>(img.scanLine(y));
            dest = reinterpret_cast<QRgb *>(buffer.scanLine(y));
            // Read in scanlines to pixel neighborhood. If the scanline is outside
            // the image use the top or bottom edge.
            for(x=y-edge, i=0; x <= y+edge; ++i, ++x){
                scanblock[i] = reinterpret_cast<QRgb *>(
                    img.scanLine((x < 0) ? 0 : (x > h-1) ? h-1 : x));
            }
            // Now we are about to start processing scanlines. First handle the
            // part where the pixel neighborhood extends off the left edge.
            for(x=0; x-edge < 0 ; ++x){
                r = g = b = 0.0;
                m = normalize_matrix;
                for(matrix_y = 0; matrix_y < matrix_size; ++matrix_y){
                    s = scanblock[matrix_y];
                    matrix_x = -edge;
                    while(x+matrix_x < 0){
                        CONVOLVE_ACC(*m, *s);
                        ++matrix_x; ++m;
                    }
                    while(matrix_x <= edge){
                        CONVOLVE_ACC(*m, *s);
                        ++matrix_x; ++m; ++s;
                    }
                }
                r = r < 0.0f ? 0.0f : r > 255.0f ? 255.0f : r + 0.5f;
                g = g < 0.0f ? 0.0f : g > 255.0f ? 255.0f : g + 0.5f;
                b = b < 0.0f ? 0.0f : b > 255.0f ? 255.0f : b + 0.5f;
                *dest++ = qRgba(static_cast<unsigned char> (r), static_cast<unsigned char> (g),
                                static_cast<unsigned char> (b), qAlpha(*src++));
            }
            // Okay, now process the middle part where the entire neighborhood
            // is on the image.
            for(; x+edge < w; ++x){
                m = normalize_matrix;
                r = g = b = 0.0;
                for(matrix_y = 0; matrix_y < matrix_size; ++matrix_y){
                    s = scanblock[matrix_y] + (x-edge);
                    for(matrix_x = -edge; matrix_x <= edge; ++matrix_x, ++m, ++s){
                        CONVOLVE_ACC(*m, *s);
                    }
                }
                r = r < 0.0f ? 0.0f : r > 255.0f ? 255.0f : r + 0.5f;
                g = g < 0.0f ? 0.0f : g > 255.0f ? 255.0f : g + 0.5f;
                b = b < 0.0f ? 0.0f : b > 255.0f ? 255.0f : b + 0.5f;
                *dest++ = qRgba(static_cast<unsigned char> (r), static_cast<unsigned char> (g),
                                static_cast<unsigned char> (b), qAlpha(*src++));
            }
            // Finally process the right part where the neighborhood extends off
            // the right edge of the image
            for(; x < w; ++x){
                r = g = b = 0.0;
                m = normalize_matrix;
                for(matrix_y = 0; matrix_y < matrix_size; ++matrix_y){
                    s = scanblock[matrix_y];
                    s += x-edge;
                    matrix_x = -edge;
                    while(x+matrix_x < w){
                        CONVOLVE_ACC(*m, *s);
                        ++matrix_x;
                        ++m;
                        ++s;
                    }
                    --s;
                    while(matrix_x <= edge){
                        CONVOLVE_ACC(*m, *s);
                        ++matrix_x;
                        ++m;
                    }
                }
                r = r < 0.0f ? 0.0f : r > 255.0f ? 255.0f : r + 0.5f;
                g = g < 0.0f ? 0.0f : g > 255.0f ? 255.0f : g + 0.5f;
                b = b < 0.0f ? 0.0f : b > 255.0f ? 255.0f : b + 0.5f;
                *dest++ = qRgba(static_cast<unsigned char> (r), static_cast<unsigned char> (g),
                                static_cast<unsigned char> (b), qAlpha(*src++));
            }
        }
    }

    delete[] scanblock;
    delete[] normalize_matrix;
    return(buffer);
}

//--------------------------------------------------------------------------------

QImage Blitz::gaussianSharpen(QImage &img, float radius, float sigma)
{
    if(sigma == 0.0f){
        qWarning("Blitz::gaussianSharpen(): Zero sigma is invalid!");
        return(img);
    }

    int matrix_size = defaultConvolveMatrixSize(radius, sigma, true);
    int len = matrix_size*matrix_size;
    float alpha, *matrix = new float[len];
    float sigma2 = sigma*sigma*2.0f;
    float sigmaPI2 = 2.0f*static_cast<float> (M_PI)*sigma*sigma;

    int half = matrix_size/2;
    int x, y, i=0, j=half;
    float normalize=0.0;
    for(y=(-half); y <= half; ++y, --j){
        for(x=(-half); x <= half; ++x, ++i){
            alpha = std::exp(-(static_cast<float> (x*x+y*y))/sigma2);
            matrix[i] = alpha/sigmaPI2;
            normalize += matrix[i];
        }
    }

    matrix[i/2]=(-2.0f)*normalize;
    QImage result(convolve(img, matrix_size, matrix));
    delete[] matrix;
    return(result);
}

//--------------------------------------------------------------------------------

QImage Blitz::emboss(QImage &img, float radius, float sigma)
{
    if(sigma == 0.0f){
        qWarning("Blitz::emboss(): Zero sigma is invalid!");
        return(img);
    }

    int matrix_size = defaultConvolveMatrixSize(radius, sigma, true);
    int len = matrix_size*matrix_size;

    float alpha, *matrix = new float[len];
    float sigma2 = sigma*sigma*2.0f;
    float sigmaPI2 = 2.0f*static_cast<float> (M_PI)*sigma*sigma;

    int half = matrix_size/2;
    int x, y, i=0, j=half;
    for(y=(-half); y <= half; ++y, --j){
        for(x=(-half); x <= half; ++x, ++i){
            alpha = std::exp(-(static_cast<float> (x*x+y*y))/sigma2);
            matrix[i]=((x < 0) || (y < 0) ? -8.0f : 8.0f)*alpha/sigmaPI2;
            if(x == j) {
                matrix[i]=0.0;
            }
        }
    }
    QImage result(convolve(img, matrix_size, matrix));
    delete[] matrix;
    equalize(result);
    return(result);
}

//--------------------------------------------------------------------------------

QImage& Blitz::flatten(QImage &img, const QColor &ca, const QColor &cb)
{
    if(img.isNull()) {
        return(img);
    }

    if(img.depth() == 1) {
        img.setColor(0, ca.rgb());
        img.setColor(1, cb.rgb());
        return(img);
    }

    int r1 = ca.red(); int r2 = cb.red();
    int g1 = ca.green(); int g2 = cb.green();
    int b1 = ca.blue(); int b2 = cb.blue();
    int min = 0, max = 255;

    QRgb *data, *end;
    QVector<QRgb> cTable;
    if(img.format() == QImage::Format_Indexed8){
        cTable = img.colorTable();
        data = static_cast<unsigned int *> (cTable.data());
        end = data + img.colorCount();

    }
    else{
        data = reinterpret_cast<QRgb *>(img.scanLine(0));
        end = data + (img.width()*img.height());
    }

    // get minimum and maximum graylevel
    QRgb *ptr = data;
    int mean;

    if(img.format() != QImage::Format_ARGB32_Premultiplied){
        while(ptr != end){
            mean = (qRed(*ptr) + qGreen(*ptr) + qBlue(*ptr)) / 3;
            min = qMin(min, mean);
            max = qMax(max, mean);
            ++ptr;
        }
    }
    else{
        QRgb pixel;
        while(ptr != end){
            pixel = convertFromPremult(*ptr);
            mean = (qRed(pixel) + qGreen(pixel) + qBlue(pixel)) / 3;
            min = qMin(min, mean);
            max = qMax(max, mean);
            ++ptr;
        }
    }

    // conversion factors
    float sr = (static_cast<float> (r2 - r1) / (max - min));
    float sg = (static_cast<float> (g2 - g1) / (max - min));
    float sb = (static_cast<float> (b2 - b1) / (max - min));

    if(img.format() != QImage::Format_ARGB32_Premultiplied){
        while(data != end){
            mean = (qRed(*data) + qGreen(*data) + qBlue(*data)) / 3;
            *data = qRgba(static_cast<unsigned char> (sr * (mean - min) + r1 + 0.5f),
                          static_cast<unsigned char> (sg * (mean - min) + g1 + 0.5f),
                          static_cast<unsigned char> (sb * (mean - min) + b1 + 0.5f),
                          qAlpha(*data));
            ++data;
        }
    }
    else{
        QRgb pixel;
        while(data != end){
            pixel = convertFromPremult(*data);
            mean = (qRed(pixel) + qGreen(pixel) + qBlue(pixel)) / 3;
            *data =
                convertToPremult(qRgba(static_cast<unsigned char> (sr * (mean - min) + r1 + 0.5f),
                                       static_cast<unsigned char> (sg * (mean - min) + g1 + 0.5f),
                                       static_cast<unsigned char> (sb * (mean - min) + b1 + 0.5f),
                                       qAlpha(*data)));
            ++data;
        }
    }

    if(img.format() == QImage::Format_Indexed8) {
        img.setColorTable(cTable);
    }
    return(img);
}

//--------------------------------------------------------------------------------
