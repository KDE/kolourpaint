
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpWidgetMapper.h"

#include <QPoint>
#include <QRect>
#include <QWidget>

namespace kpWidgetMapper
{

QPoint fromGlobal(const QWidget *widget, const QPoint &point)
{
    if (!widget) {
        return point;
    }

    return widget->mapFromGlobal(point);
}

QRect fromGlobal(const QWidget *widget, const QRect &rect)
{
    if (!widget || !rect.isValid()) {
        return rect;
    }

    auto topLeft = fromGlobal(widget, rect.topLeft());
    return {topLeft.x(), topLeft.y(), rect.width(), rect.height()};
}

QPoint toGlobal(const QWidget *widget, const QPoint &point)
{
    if (!widget) {
        return point;
    }

    return widget->mapToGlobal(point);
}

QRect toGlobal(const QWidget *widget, const QRect &rect)
{
    if (!widget || !rect.isValid()) {
        return rect;
    }

    auto topLeft = toGlobal(widget, rect.topLeft());
    return {topLeft.x(), topLeft.y(), rect.width(), rect.height()};
}

} // namespace kpWidgetMapper
