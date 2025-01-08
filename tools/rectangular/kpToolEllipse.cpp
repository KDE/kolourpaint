/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2017      Martin Koller <kollix@aon.at>
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

#include "kpToolEllipse.h"
#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpColor.h"

#include <KLocalizedString>

#include <QBrush>
#include <QPainter>
#include <QPen>

//---------------------------------------------------------------------

kpToolEllipse::kpToolEllipse(kpToolEnvironment *environ, QObject *parent)
    : kpToolRectangularBase(i18n("Ellipse"),
                            i18n("Draws ellipses and circles"),
                            &kpToolEllipse::drawEllipse,
                            Qt::Key_E,
                            environ,
                            parent,
                            QStringLiteral("tool_ellipse"))
{
}

//---------------------------------------------------------------------

void kpToolEllipse::drawEllipse(kpImage *image, int x, int y, int width, int height, const kpColor &fcolor, int penWidth, const kpColor &bcolor)
{
    if ((width == 0) || (height == 0)) {
        return;
    }

    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

    if (((2 * penWidth) > width) || ((2 * penWidth) > height)) {
        penWidth = qMin(width, height) / 2;
    }

    painter.setPen(QPen(fcolor.toQColor(), penWidth));

    if (bcolor.isValid()) {
        painter.setBrush(QBrush(bcolor.toQColor()));
    } else {
        painter.setBrush(Qt::NoBrush);
    }

    int offset = painter.testRenderHint(QPainter::Antialiasing) ? 1 : 0;

    painter.drawEllipse(x + penWidth / 2 + offset, y + penWidth / 2 + offset, qMax(1, width - penWidth - offset), qMax(1, height - penWidth - offset));
}

//---------------------------------------------------------------------

#include "moc_kpToolEllipse.cpp"
