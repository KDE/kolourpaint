
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectReduceColors_H
#define kpEffectReduceColors_H

#include <QImage>

// The <depth> specified must be supported by QImage.
class kpEffectReduceColors
{
public:
    // TODO: Why isn't applyEffect() for the public API sufficient?
    //       Ans: See TODO in kpDocument_Save.cpp.  Maybe we should rename
    //            this method?
    //
    //            Also, this can increase the image depth while applyEffect()
    //            will not.
    static QImage convertImageDepth(const QImage &image, int depth, bool dither);

    static void applyEffect(QImage *destPixmapPtr, int depth, bool dither);
    static QImage applyEffect(const QImage &pm, int depth, bool dither);
};

#endif // kpEffectReduceColors_H
