/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2017 Martin Koller <kollix@aon.at>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpToolRoundedRectangle.h"

#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpColor.h"

#include <KLocalizedString>

#include <QBrush>
#include <QPainter>
#include <QPen>

//---------------------------------------------------------------------

kpToolRoundedRectangle::kpToolRoundedRectangle(kpToolEnvironment *environ, QObject *parent)
    : kpToolRectangularBase(i18n("Rounded Rectangle"),
                            i18n("Draws rectangles and squares with rounded corners"),
                            &kpToolRoundedRectangle::drawRoundedRect,
                            Qt::Key_U,
                            environ,
                            parent,
                            QStringLiteral("tool_rounded_rectangle"))
{
}

//---------------------------------------------------------------------

void kpToolRoundedRectangle::drawRoundedRect(kpImage *image, int x, int y, int width, int height, const kpColor &fcolor, int penWidth, const kpColor &bcolor)
{
    if ((width == 0) || (height == 0)) {
        return;
    }

    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

    if (((2 * penWidth) > width) || ((2 * penWidth) > height)) {
        penWidth = qMin(width, height) / 2;
    }

    painter.setPen(QPen(fcolor.toQColor(), penWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));

    if (bcolor.isValid()) {
        painter.setBrush(QBrush(bcolor.toQColor()));
    } else {
        painter.setBrush(Qt::NoBrush);
    }

    int offset = painter.testRenderHint(QPainter::Antialiasing) ? 1 : 0;

    int radius = qMin(width, height) / 4;

    painter.drawRoundedRect(x + penWidth / 2 + offset,
                            y + penWidth / 2 + offset,
                            qMax(1, width - penWidth - offset),
                            qMax(1, height - penWidth - offset),
                            radius,
                            radius);
}

//---------------------------------------------------------------------

#include "moc_kpToolRoundedRectangle.cpp"
