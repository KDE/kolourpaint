
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_FLOOD_FILL_H
#define KP_FLOOD_FILL_H

#include "commands/kpCommandSize.h"
#include "kpImage.h"

class kpColor;
class kpFillLine;

struct kpFloodFillPrivate;

class kpFloodFill
{
public:
    kpFloodFill(kpImage *image, int x, int y, const kpColor &color, int processedColorSimilarity);
    ~kpFloodFill();

    kpFloodFill(const kpFloodFill &) = delete;
    kpFloodFill &operator=(const kpFloodFill &) = delete;

    //
    // Spits back constructor arguments.
    //

public:
    kpColor color() const;
    int processedColorSimilarity() const;

public:
    // Used for calculating the size of a command in the command history.
    kpCommandSize::SizeType size() const;

    //
    // Step 1: Determines the color that will be changed to color().
    //
    //         Very fast.
    //

public:
    void prepareColorToChange();

    // (may invoke prepareColorToChange()).
    kpColor colorToChange();

    //
    // Step 2: Determines the scanlines / pixels that will be changed to color().
    //
    //         The slowest part of the whole fill operation.
    //
    //         Before calling a Step 2 function, you don't have to (but you can)
    //         call any of the functions in Step 1.
    //

private:
    kpColor pixelColor(int x, int y, bool *beenHere = nullptr) const;
    bool shouldGoTo(int x, int y) const;

    // Finds the minimum x value at a certain line to be filled.
    int findMinX(int y, int x) const;

    // Finds the maximum x value at a certain line to be filled.
    int findMaxX(int y, int x) const;

    void addLine(int y, int x1, int x2);
    void findAndAddLines(const kpFillLine &fillLine, int dy);

public:
    // (may invoke Step 1's prepareColorToChange())
    void prepare();

    // (may invoke prepare())
    QRect boundingRect();

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
    void fill();

private:
    kpFloodFillPrivate *const d;
};

#endif // KP_FLOOD_FILL_H
