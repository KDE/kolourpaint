
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectInvert_H
#define kpEffectInvert_H

class QImage;

class kpEffectInvert
{
public:
    enum Channel {
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        RGB = Red | Green | Blue
    };

    //
    // Inverts the colors of each pixel in the given image.
    // These functions differ from QImage::invertPixels() in the following ways:
    //
    // 1. for 8-bit images, it inverts the colors of the Colour Table
    //    (this means that you would get visually similar results to inversion
    //     at higher bit depths - rather than a "random-looking" inversion
    //     depending on the contents of the Colour Table)
    // 2. never inverts the Alpha Buffer
    //

    static void applyEffect(QImage *destImagePtr, int channels = RGB);
    static QImage applyEffect(const QImage &img, int channels = RGB);
};

#endif // kpEffectInvert_H
