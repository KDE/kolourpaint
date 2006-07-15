
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#ifndef KP_FLOOD_FILL_H
#define KP_FLOOD_FILL_H


#include <qimage.h>
#include <qlinkedlist.h>
#include <qlist.h>

#include <kpcolor.h>


class QPixmap;


class kpFloodFill
{
public:
    kpFloodFill (QPixmap *pixmap, int x, int y,
                 const kpColor &color,
                 int processedColorSimilarity);
    ~kpFloodFill ();

    int size () const;
    
    kpColor color () const;
    int processedColorSimilarity () const;

    
    //
    // Step 1: Determines the colour that will be changed to color().
    //
    
    void prepareColorToChange ();

    // (may invoke prepareColorToChange()).
    kpColor colorToChange ();


    //
    // Step 2: Determines the scanlines / pixels that will be changed to color().
    //
    //         Before calling a Step 2 function, you don't have to (but you can)
    //         call any of the functions in Step 1.
    //

    // (may invoke Step 1's prepare())
    void prepare ();

    // (may invoke prepare())
    QRect boundingRect ();


    //
    // Step 3: Draws the lines identified in Step 2 in color().
    //
    //         Before calling a Step 3 function, you don't have to (but you can)
    //         call any of the functions in Step 1 or 2.
    //

    // (may invoke Step 2's prepare())
    void fill ();

private:
    QPixmap *m_pixmapPtr;
    int m_x, m_y;
    kpColor m_color;
    int m_processedColorSimilarity;

    bool m_prepared;

    QRect m_boundingRect;

    struct FillLine
    {
        FillLine (int y = -1, int x1 = -1, int x2 = -1)
            : m_y (y), m_x1 (x1), m_x2 (x2)
        {
        }

        static int size ()
        {
            return sizeof (FillLine);
        }
        
        int m_y, m_x1, m_x2;
    };

    int fillLinesListSize (const QLinkedList <kpFloodFill::FillLine> &fillLines) const;
    
    void addLine (int y, int x1, int x2);
    kpColor pixelColor (int x, int y, bool *beenHere = 0) const;
    bool shouldGoTo (int x, int y) const;
    void findAndAddLines (const FillLine &fillLine, int dy);
    int findMinX (int y, int x) const;
    int findMaxX (int y, int x) const;

    QLinkedList <FillLine> m_fillLines;

    // Init info
    QImage m_image;
    QList < QLinkedList <FillLine> > m_fillLinesCache;
    kpColor m_colorToChange;
};


#endif  // KP_FLOOD_FILL_H

