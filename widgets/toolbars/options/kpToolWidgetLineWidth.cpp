
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"

#include "imagelib/kpColor.h"
#include "pixmapfx/kpPixmapFX.h"

#include <KLocalizedString>

#include <QPixmap>

static int lineWidths[] = {1, 2, 3, 5, 8};

kpToolWidgetLineWidth::kpToolWidgetLineWidth(QWidget *parent, const QString &name)
    : kpToolWidgetBase(parent, name)
{
    int numLineWidths = sizeof(lineWidths) / sizeof(lineWidths[0]);

    int w = (width() - 2 /*margin*/) * 3 / 4;
    int h = (height() - 2 /*margin*/ - (numLineWidths - 1) /*spacing*/) * 3 / (numLineWidths * 4);

    for (int i = 0; i < numLineWidths; i++) {
        QImage image((w <= 0 ? width() : w), (h <= 0 ? height() : h), QImage::Format_ARGB32_Premultiplied);
        image.fill(QColor(Qt::transparent).rgba());

        kpPixmapFX::fillRect(&image, 0, (image.height() - lineWidths[i]) / 2, image.width(), lineWidths[i], kpColor::Black);

        addOption(QPixmap::fromImage(std::move(image)), QString::number(lineWidths[i]));
        startNewOptionRow();
    }

    finishConstruction(0, 0);
}

kpToolWidgetLineWidth::~kpToolWidgetLineWidth() = default;

int kpToolWidgetLineWidth::lineWidth() const
{
    return lineWidths[selectedRow()];
}

// virtual protected slot [base kpToolWidgetBase]
bool kpToolWidgetLineWidth::setSelected(int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected(row, col, saveAsDefault);
    if (ret) {
        Q_EMIT lineWidthChanged(lineWidth());
    }
    return ret;
}

#include "moc_kpToolWidgetLineWidth.cpp"
