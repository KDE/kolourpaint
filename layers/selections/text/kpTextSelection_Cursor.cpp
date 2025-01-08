
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2010 Tasuku Suzuki <stasuku@gmail.com>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_SELECTION 0

#include "kpTextSelection.h"
#include "kpTextSelectionPrivate.h"

#include "kpDefs.h"
#include "kpPreeditText.h"
#include "kpTextStyle.h"

#include "kpLogCategories.h"

#include <QFontMetrics>

// public
int kpTextSelection::closestTextRowForPoint(const QPoint &point) const
{
    if (!pointIsInTextArea(point)) {
        return -1;
    }

    const QFontMetrics fontMetrics(d->textStyle.fontMetrics());

    int row = (point.y() - textAreaRect().y()) / fontMetrics.lineSpacing();
    if (row >= static_cast<int>(d->textLines.size())) {
        row = d->textLines.size() - 1;
    }

    return row;
}

// public
int kpTextSelection::closestTextColForPoint(const QPoint &point) const
{
    int row = closestTextRowForPoint(point);
    if (row < 0 || row >= static_cast<int>(d->textLines.size())) {
        return -1;
    }

    const int localX = point.x() - textAreaRect().x();

    const QFontMetrics fontMetrics(d->textStyle.fontMetrics());

    // (should be 0 but call just in case)
    int charLocalLeft = fontMetrics.horizontalAdvance(d->textLines[row], 0);

    // OPT: binary search or guess location then move
    for (int col = 0; col < static_cast<int>(d->textLines[row].length()); col++) {
        // OPT: fontMetrics::charWidth() might be faster
        const int nextCharLocalLeft = fontMetrics.horizontalAdvance(d->textLines[row], col + 1);
        if (localX <= (charLocalLeft + nextCharLocalLeft) / 2) {
            return col;
        }

        charLocalLeft = nextCharLocalLeft;
    }

    return d->textLines[row].length() /*past end of line*/;
}

//---------------------------------------------------------------------

// public
QPoint kpTextSelection::pointForTextRowCol(int row, int col) const
{
    kpPreeditText preeditText = d->preeditText;
    if ((row < 0 || col < 0)
        || (preeditText.isEmpty() && (row >= static_cast<int>(d->textLines.size()) || col > static_cast<int>(d->textLines[row].length())))) {
#if DEBUG_KP_SELECTION && 1
        qCDebug(kpLogLayers) << "kpTextSelection::pointForTextRowCol(" << row << "," << col << ") out of range"
                             << " textLines='" << text() << "'";
#endif
        return KP_INVALID_POINT;
    }

    const QFontMetrics fontMetrics(d->textStyle.fontMetrics());

    QString line = (d->textLines.count() > row) ? d->textLines[row] : QString();
    if (row == preeditText.position().y()) {
        line.insert(preeditText.position().x(), preeditText.preeditString());
    }
    const int x = fontMetrics.horizontalAdvance(line.left(col));
    const int y = row * fontMetrics.height() + (row >= 1 ? row * fontMetrics.leading() : 0);

    return textAreaRect().topLeft() + QPoint(x, y);
}

//---------------------------------------------------------------------
