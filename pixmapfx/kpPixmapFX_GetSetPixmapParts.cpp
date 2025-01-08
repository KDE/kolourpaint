
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_PIXMAP_FX 0

#include "kpPixmapFX.h"

#include <QImage>
#include <QPainter>
#include <QPoint>
#include <QRect>

#include "kpLogCategories.h"

#include "imagelib/kpColor.h"

//---------------------------------------------------------------------

// public static
QImage kpPixmapFX::getPixmapAt(const QImage &image, const QRect &rect)
{
    return image.copy(rect);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::setPixmapAt(QImage *destPtr, const QRect &destRect, const QImage &src)
{
#if DEBUG_KP_PIXMAP_FX && 1
    qCDebug(kpLogPixmapfx) << "kpPixmapFX::setPixmapAt(destPixmap->rect=" << destPtr->rect() << ",destRect=" << destRect << ",src.rect=" << src.rect() << ")";
#endif

    Q_ASSERT(destPtr);

    // You cannot copy more than what you have.
    Q_ASSERT(destRect.width() <= src.width() && destRect.height() <= src.height());

    QPainter painter(destPtr);
    // destination shall be source only
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(destRect.topLeft(), src, QRect(0, 0, destRect.width(), destRect.height()));
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::setPixmapAt(QImage *destPtr, const QPoint &destAt, const QImage &src)
{
    kpPixmapFX::setPixmapAt(destPtr, QRect(destAt.x(), destAt.y(), src.width(), src.height()), src);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::setPixmapAt(QImage *destPtr, int destX, int destY, const QImage &src)
{
    kpPixmapFX::setPixmapAt(destPtr, QPoint(destX, destY), src);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::paintPixmapAt(QImage *destPtr, const QPoint &destAt, const QImage &src)
{
    // draw image with SourceOver composition mode
    QPainter painter(destPtr);
    painter.drawImage(destAt, src);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::paintPixmapAt(QImage *destPtr, int destX, int destY, const QImage &src)
{
    kpPixmapFX::paintPixmapAt(destPtr, QPoint(destX, destY), src);
}

//---------------------------------------------------------------------

// public static
kpColor kpPixmapFX::getColorAtPixel(const QImage &img, const QPoint &at)
{
    if (!img.valid(at.x(), at.y())) {
        return kpColor::Invalid;
    }

    QRgb rgba = img.pixel(at);
    return kpColor(rgba);
}

//---------------------------------------------------------------------

// public static
kpColor kpPixmapFX::getColorAtPixel(const QImage &img, int x, int y)
{
    return kpPixmapFX::getColorAtPixel(img, QPoint(x, y));
}

//---------------------------------------------------------------------
