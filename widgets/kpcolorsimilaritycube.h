
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


#ifndef __kp_color_similarity_cube_h__
#define __kp_color_similarity_cube_h__

#include <qframe.h>

class kpColor;
class kpMainWindow;

class kpColorSimilarityCube : public QFrame
{
public:
    enum Look
    {
        Plain = 0,
        Depressed = 1,
        DoubleClickInstructions = 2
    };

    kpColorSimilarityCube (int look,
                           kpMainWindow *mainWindow,
                           QWidget *parent,
                           const char *name = 0);
    virtual ~kpColorSimilarityCube ();

    static const double colorCubeDiagonalDistance;

    double colorSimilarity () const;
    void setColorSimilarity (double similarity);

    virtual QSize sizeHint () const;

protected:
    QColor color (int redOrGreenOrBlue, int baseBrightness, int similarityDirection) const;
    void drawFace (QPainter *p,
                   int redOrGreenOrBlue,
                   const QPoint &tl, const QPoint &tr,
                   const QPoint &bl, const QPoint &br);
    virtual void drawContents (QPainter *p);

    kpMainWindow *m_mainWindow;
    double m_colorSimilarity;
};

#endif  // __kp_color_similarity_cube_h__
