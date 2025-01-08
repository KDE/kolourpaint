
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectFlatten_H
#define kpEffectFlatten_H

class QColor;
class QImage;

class kpEffectFlatten
{
public:
    static void applyEffect(QImage *destImagePtr, const QColor &color1, const QColor &color2);
    static QImage applyEffect(const QImage &img, const QColor &color1, const QColor &color2);
};

#endif // kpEffectFlatten_H
