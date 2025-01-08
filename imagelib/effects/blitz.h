#ifndef BLITZ_H
#define BLITZ_H

//**************************************************************************
/*
   SPDX-FileCopyrightText: 2016 Martin Koller kollix@aon.at

   SPDX-License-Identifier: BSD-2-Clause
*/
//**************************************************************************

#include <QImage>

namespace Blitz
{
QImage blur(QImage &img, int radius);
QImage gaussianSharpen(QImage &img, float radius, float sigma);
QImage emboss(QImage &img, float radius, float sigma);
QImage &flatten(QImage &img, const QColor &ca, const QColor &cb);
};

#endif
