
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


#ifndef kpAbstractScrollAreaUtils_H
#define kpAbstractScrollAreaUtils_H


class Q3ScrollView;
class QAbstractScrollArea;
class QSize;


// TODO: Account for scroll areas' viewport margins (but they're 0 by default anyway).
namespace kpAbstractScrollAreaUtils
{
    // Guesses the width of the vertical scrollbar of <scrollArea>.
    // If it's not visible, this method guesses its width if it were visible.
    int EstimateVerticalScrollBarWidth (QAbstractScrollArea *scrollArea);
    int EstimateVerticalScrollBarWidth (Q3ScrollView *scrollArea);

    // Guesses the height of the horizontal scrollbar of <scrollArea>.
    // If it's not visible, this method guesses its height if it were visible.
    int EstimateHorizontalScrollBarHeight (QAbstractScrollArea *scrollArea);
    int EstimateHorizontalScrollBarHeight (Q3ScrollView *scrollArea);

    // Guesses the size of the <scrollArea> minus the space the scrollbars
    // take (if they are visible) or would take (if they are not visible).
    //
    // The is useful for implementing zoom-to-rectangle so that the
    // automatic introduction of scrollbars doesn't cover any content that
    // should be visible.  Of course, if the scrollbars aren't automatically
    // introduced, then <scrollArea> will display a little too much content,
    // which is OK.
    QSize EstimateUsableArea (QAbstractScrollArea *scrollArea);
    QSize EstimateUsableArea (Q3ScrollView *scrollArea);
}


#endif  // kpAbstractScrollAreaUtils_H

