
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_WIDGET_FILL_STYLE 0

#include "kpToolWidgetFillStyle.h"

#include "imagelib/kpColor.h"
#include "kpDefs.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"

#include "kpLogCategories.h"

#include <QPainter>
#include <QPixmap>

//---------------------------------------------------------------------

kpToolWidgetFillStyle::kpToolWidgetFillStyle(QWidget *parent, const QString &name)
    : kpToolWidgetBase(parent, name)
{
    for (int i = 0; i < FillStyleNum; i++) {
        QPixmap pixmap;

        pixmap = fillStylePixmap(static_cast<FillStyle>(i), (width() - 2 /*margin*/) * 3 / 4, (height() - 2 /*margin*/ - 2 /*spacing*/) * 3 / (3 * 4));
        addOption(pixmap, fillStyleName(static_cast<FillStyle>(i)) /*tooltip*/);

        startNewOptionRow();
    }

    finishConstruction(0, 0);
}

//---------------------------------------------------------------------

kpToolWidgetFillStyle::~kpToolWidgetFillStyle() = default;

//---------------------------------------------------------------------

// private
QPixmap kpToolWidgetFillStyle::fillStylePixmap(FillStyle fs, int w, int h)
{
    QPixmap pixmap((w <= 0 ? width() : w), (h <= 0 ? height() : h));
    pixmap.fill(palette().color(QPalette::Window));

    const int penWidth = 2;

    const QRect rectRect(1, 1, w - 2, h - 2);

    QPainter painter(&pixmap);
    painter.setPen(kpPixmapFX::QPainterDrawRectPen(Qt::black, penWidth));

    switch (fs) {
    case NoFill: {
        painter.setBrush(Qt::NoBrush);
        break;
    }
    case FillWithBackground: {
        painter.setBrush(Qt::gray);
        break;
    }
    case FillWithForeground: {
        painter.setBrush(Qt::black);
        break;
    }
    default:;
    }

    painter.drawRect(rectRect);
    painter.end();

    return pixmap;
}

//---------------------------------------------------------------------

// private
QString kpToolWidgetFillStyle::fillStyleName(FillStyle fs) const
{
    switch (fs) {
    case NoFill:
        return i18n("No Fill");

    case FillWithBackground:
        return i18n("Fill with Background Color");

    case FillWithForeground:
        return i18n("Fill with Foreground Color");

    default:
        return {};
    }
}

//---------------------------------------------------------------------

// public
kpToolWidgetFillStyle::FillStyle kpToolWidgetFillStyle::fillStyle() const
{
#if DEBUG_KP_TOOL_WIDGET_FILL_STYLE
    qCDebug(kpLogWidgets) << "kpToolWidgetFillStyle::fillStyle() selected=" << selectedRow();
#endif
    return static_cast<FillStyle>(selectedRow());
}

//---------------------------------------------------------------------

kpColor kpToolWidgetFillStyle::drawingBackgroundColor(const kpColor &foregroundColor, const kpColor &backgroundColor) const
{
    switch (fillStyle()) {
    default:
    case NoFill:
        return kpColor::Invalid;

    case FillWithBackground:
        return backgroundColor;

    case FillWithForeground:
        return foregroundColor;
    }
}

//---------------------------------------------------------------------

// virtual protected slot [base kpToolWidgetBase]
bool kpToolWidgetFillStyle::setSelected(int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected(row, col, saveAsDefault);
    if (ret) {
        Q_EMIT fillStyleChanged(fillStyle());
    }
    return ret;
}

//---------------------------------------------------------------------

#include "moc_kpToolWidgetFillStyle.cpp"
