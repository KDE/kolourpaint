
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __kpfloodfill_h__
#define __kpfloodfill_h__

#include <qcolor.h>
#include <qimage.h>
#include <qvaluelist.h>
#include <qvaluevector.h>

class QPixmap;

class kpFloodFill
{
public:
    kpFloodFill (QPixmap *pixmap, int x, int y, const QColor &color);
    ~kpFloodFill ();

    QColor color () const { return m_color; }

    // you should call [prepareColorToChange(),[prepare(),[fill()]]]
    bool prepareColorToChange ();
    QColor colorToChange () const { return QColor (m_colorToChange); };  // only valid after prepareColorToChange()

    bool prepare ();
    QRect boundingRect () const;  // only valid after prepare()

    bool fill ();

private:
    QPixmap *m_pixmapPtr;
    int m_x, m_y;
    QColor m_color;

    int m_initState;

    QRect m_boundingRect;

    struct FillLine
    {
        FillLine (int y = -1, int x1 = -1, int x2 = -1)
            : m_y (y), m_x1 (x1), m_x2 (x2)
        {
        }

        int m_y, m_x1, m_x2;
    };

    void addLine (int y, int x1, int x2);
    QRgb pixelColor (int x, int y);
    void findAndAddLines (const FillLine &fillLine, int dy);
    int findMinX (int y, int x);
    int findMaxX (int y, int x);

    QValueList <FillLine> m_fillLines;

    // Init info
    QImage m_image;
    QValueVector < QValueList <FillLine> > m_fillLinesCache;
    QRgb m_colorToChange;
};

#endif  // __kpfloodfill_h__
