
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#include "kpImage.h"
#include "commands/kpCommandSize.h"


class kpColor;
class kpFillLine;


struct kpFloodFillPrivate;

class kpFloodFill
{
public:
    kpFloodFill (kpImage *image, int x, int y,
                 const kpColor &color,
                 int processedColorSimilarity);
    ~kpFloodFill ();


    //
    // Spits back constructor arguments.
    //

public:
    kpColor color () const;
    int processedColorSimilarity () const;


public:
    // Used for calculating the size of a command in the command history.
    kpCommandSize::SizeType size () const;


    //
    // Step 1: Determines the colour that will be changed to color().
    //
    //         Very fast.
    //

public:
    void prepareColorToChange ();

    // (may invoke prepareColorToChange()).
    kpColor colorToChange ();


    //
    // Step 2: Determines the scanlines / pixels that will be changed to color().
    //
    //         The slowest part of the whole fill operation.
    //
    //         Before calling a Step 2 function, you don't have to (but you can)
    //         call any of the functions in Step 1.
    //

private:
    kpColor pixelColor (int x, int y, bool *beenHere = nullptr) const;
    bool shouldGoTo (int x, int y) const;

    // Finds the minimum x value at a certain line to be filled.
    int findMinX (int y, int x) const;

    // Finds the maximum x value at a certain line to be filled.
    int findMaxX (int y, int x) const;

    void addLine (int y, int x1, int x2);
    void findAndAddLines (const kpFillLine &fillLine, int dy);

public:
    // (may invoke Step 1's prepareColorToChange())
    void prepare ();

    // (may invoke prepare())
    QRect boundingRect ();


    //
    // Step 3: Draws the lines identified in Step 2 in color().
    //
    //         Between the speeds of Step 2 and Step 1.
    //
    //         Before calling a Step 3 function, you don't have to (but you can)
    //         call any of the functions in Step 1 or 2.
    //

public:
    // (may invoke Step 2's prepare())
    void fill ();


private:
    kpFloodFillPrivate * const d;
};


#endif  // KP_FLOOD_FILL_H
