
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE 0

#include "kpToolWidgetSpraycanSize.h"

#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QPixmap>

static int spraycanSizes[] = {9, 17, 29};

kpToolWidgetSpraycanSize::kpToolWidgetSpraycanSize(QWidget *parent, const QString &name)
    : kpToolWidgetBase(parent, name)
{
#if DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE
    qCDebug(kpLogWidgets) << "kpToolWidgetSpraycanSize::kpToolWidgetSpraycanSize() CALLED!";
#endif

    for (int i = 0; i < int(sizeof(spraycanSizes) / sizeof(spraycanSizes[0])); i++) {
        int s = spraycanSizes[i];
        const QString iconName = QStringLiteral(":/icons/tool_spraycan_%1x%2").arg(s).arg(s);

#if DEBUG_KP_TOOL_WIDGET_SPRAYCAN_SIZE
        qCDebug(kpLogWidgets) << "\ticonName=" << iconName;
#endif

        QPixmap pixmap(s, s);
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.drawPixmap(0, 0, QPixmap(iconName));
        painter.end();

        QImage image = pixmap.toImage();

        QBitmap mask(pixmap.width(), pixmap.height());
        mask.fill(Qt::color0);

        painter.begin(&mask);
        painter.setPen(Qt::color1);

        for (int y = 0; y < image.height(); y++) {
            for (int x = 0; x < image.width(); x++) {
                if ((image.pixel(x, y) & RGB_MASK) == 0 /*black*/) {
                    painter.drawPoint(x, y); // mark as opaque
                }
            }
        }

        painter.end();

        pixmap.setMask(mask);

        addOption(pixmap, i18n("%1x%2", s, s) /*tooltip*/);
        if (i == 1) {
            startNewOptionRow();
        }
    }

    finishConstruction(0, 0);
}

kpToolWidgetSpraycanSize::~kpToolWidgetSpraycanSize() = default;

// public
int kpToolWidgetSpraycanSize::spraycanSize() const
{
    return spraycanSizes[selected() < 0 ? 0 : selected()];
}

// protected slot virtual [base kpToolWidgetBase]
bool kpToolWidgetSpraycanSize::setSelected(int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected(row, col, saveAsDefault);
    if (ret) {
        Q_EMIT spraycanSizeChanged(spraycanSize());
    }
    return ret;
}

#include "moc_kpToolWidgetSpraycanSize.cpp"
