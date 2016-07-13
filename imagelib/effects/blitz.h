#ifndef BLITZ_H
#define BLITZ_H

//**************************************************************************
//   (c) 2016 Martin Koller, kollix@aon.at
//
//   This file is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, version 2 of the License
//
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
