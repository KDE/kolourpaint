
/*
   Copyright (c) 2020 Kirill Proshakov <kproshakov@astralinux.ru>
   Copyright (c) 2024 Andrew Shark <ashark@linuxcomp.ru>
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

#include "kpCursorLightCrossWithIcon.h"
#include "kpLogCategories.h"

#include <QPixmap>
#include <QPainter>
#include <QCursor>
#include <QIcon>

QCursor kpCursorLightCrossWithIconCreate(const QString &iconName)
{
    qCDebug(kpLogTools) << "iconName:" << iconName;
    // Standard cursor sizes: 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72
    // Standard icon sizes:    8, 16, 22, 32, 48, 64

    auto icon = QIcon::fromTheme(iconName);
    constexpr int pixmapsize = 16;
    QPixmap pixmapIcon = icon.pixmap(pixmapsize);

    QPixmap pixmap(pixmapsize * 2, pixmapsize * 2);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    constexpr int hotPointYShift = 6;
    constexpr int crossSide = 24;
    constexpr int crossHalf = crossSide / 2;
    constexpr int crossHalfShifted = crossHalf + hotPointYShift;

    QPen whitePen(Qt::white);
    QPen blackPen(Qt::black);

    for (int i = 0; i < crossHalf; i++) {
        painter.setPen(i % 2 ? blackPen : whitePen);

        painter.drawPoint(crossHalf, hotPointYShift + i);
        painter.drawPoint(crossHalf, crossSide + hotPointYShift - i);

        painter.drawPoint(1 + i, crossHalfShifted);
        painter.drawPoint(crossSide - 1 - i, crossHalfShifted);
    }

    painter.drawPixmap(pixmapsize - 1, 0, pixmapsize, pixmapsize, pixmapIcon);
    return QCursor(pixmap, crossHalf, crossHalf + hotPointYShift);
}

