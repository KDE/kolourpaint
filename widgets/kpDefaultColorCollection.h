
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpDefaultColorCollection_H
#define kpDefaultColorCollection_H

#include "lgpl/generic/kpColorCollection.h"

//
// The default set of colors offered by KolourPaint to the user.
//
// It contains all of the ordinary colors (black, white, gray, colors of
// the rainbow etc.) and a few others.
//
class kpDefaultColorCollection : public kpColorCollection
{
public:
    kpDefaultColorCollection();
    ~kpDefaultColorCollection();
};

#endif // kpDefaultColorCollection_H
