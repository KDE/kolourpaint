
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2010 Tasuku Suzuki <stasuku@gmail.com>
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


#define DEBUG_KP_SELECTION 0


#include "kpTextSelection.h"
#include "kpTextSelectionPrivate.h"

#include "kpDefs.h"
#include "kpTextStyle.h"
#include "kpPreeditText.h"

#include "kpLogCategories.h"

#include <QFontMetrics>
#include <QList>


// public
int kpTextSelection::closestTextRowForPoint (const QPoint &point) const
{
    if (!pointIsInTextArea (point)) {
        return -1;
    }

    const QFontMetrics fontMetrics (d->textStyle.fontMetrics ());

    int row = (point.y () - textAreaRect ().y ()) /
               fontMetrics.lineSpacing ();
    if (row >= static_cast<int> (d->textLines.size ())) {
        row = d->textLines.size () - 1;
    }

    return row;
}

// public
int kpTextSelection::closestTextColForPoint (const QPoint &point) const
{
    int row = closestTextRowForPoint (point);
    if (row < 0 || row >= static_cast<int> (d->textLines.size ())) {
        return -1;
    }

    const int localX = point.x () - textAreaRect ().x ();

    const QFontMetrics fontMetrics (d->textStyle.fontMetrics ());

    // (should be 0 but call just in case)
    int charLocalLeft = fontMetrics.horizontalAdvance(d->textLines [row], 0);

    // OPT: binary search or guess location then move
    for (int col = 0; col < static_cast<int> (d->textLines [row].length ()); col++)
    {
        // OPT: fontMetrics::charWidth() might be faster
        const int nextCharLocalLeft = fontMetrics.horizontalAdvance(d->textLines [row], col + 1);
        if (localX <= (charLocalLeft + nextCharLocalLeft) / 2) {
            return col;
        }

        charLocalLeft = nextCharLocalLeft;
    }

    return d->textLines [row].length ()/*past end of line*/;
}

//---------------------------------------------------------------------

// public
QPoint kpTextSelection::pointForTextRowCol (int row, int col) const
{
    kpPreeditText preeditText = d->preeditText;
    if ((row < 0 || col < 0) ||
        (preeditText.isEmpty () &&
            (row >= static_cast<int> (d->textLines.size ()) || col > static_cast<int> (d->textLines [row].length ()))))
    {
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpTextSelection::pointForTextRowCol("
               << row << ","
               << col << ") out of range"
               << " textLines='"
               << text ()
               << "'";
#endif
        return KP_INVALID_POINT;
    }

    const QFontMetrics fontMetrics (d->textStyle.fontMetrics ());

    QString line = (d->textLines.count () > row) ? d->textLines[row] : QString ();
    if (row == preeditText.position ().y ())
    {
        line.insert (preeditText.position ().x (), preeditText.preeditString ());
    }
    const int x = fontMetrics.horizontalAdvance(line.left(col));
    const int y = row * fontMetrics.height () +
                  (row >= 1 ? row * fontMetrics.leading () : 0);

    return textAreaRect ().topLeft () + QPoint (x, y);
}

//---------------------------------------------------------------------
