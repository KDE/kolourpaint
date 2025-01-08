
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_WIDGET_MAPPER_H
#define KP_WIDGET_MAPPER_H

class QWidget;
class QPoint;
class QRect;

namespace kpWidgetMapper
{
QPoint fromGlobal(const QWidget *widget, const QPoint &point);
QRect fromGlobal(const QWidget *widget, const QRect &rect);

QPoint toGlobal(const QWidget *widget, const QPoint &point);
QRect toGlobal(const QWidget *widget, const QRect &rect);
}

#endif // KP_WIDGET_MAPPER_H
