
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_SELECTION 0

#include "layers/selections/image/kpRectangularImageSelection.h"

#include <QBitmap>
#include <QRegion>

struct kpRectangularImageSelectionPrivate {
};

kpRectangularImageSelection::kpRectangularImageSelection(const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection(transparency)
    , d(new kpRectangularImageSelectionPrivate())
{
}

kpRectangularImageSelection::kpRectangularImageSelection(const QRect &rect, const kpImage &baseImage, const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection(rect, baseImage, transparency)
    , d(new kpRectangularImageSelectionPrivate())
{
}

kpRectangularImageSelection::kpRectangularImageSelection(const QRect &rect, const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection(rect, transparency)
    , d(new kpRectangularImageSelectionPrivate())
{
}

kpRectangularImageSelection::kpRectangularImageSelection(const kpRectangularImageSelection &rhs)
    : kpAbstractImageSelection()
    , d(new kpRectangularImageSelectionPrivate())
{
    *this = rhs;
}

kpRectangularImageSelection &kpRectangularImageSelection::operator=(const kpRectangularImageSelection &rhs)
{
    kpAbstractImageSelection::operator=(rhs);

    return *this;
}

kpRectangularImageSelection *kpRectangularImageSelection::clone() const
{
    kpRectangularImageSelection *sel = new kpRectangularImageSelection();
    *sel = *this;
    return sel;
}

kpRectangularImageSelection::~kpRectangularImageSelection()
{
    delete d;
}

// public virtual [kpAbstractSelection]
int kpRectangularImageSelection::serialID() const
{
    return SerialID;
}

// public virtual [kpAbstractSelection]
bool kpRectangularImageSelection::isRectangular() const
{
    return true;
}

// public virtual [kpAbstractSelection]
QPolygon kpRectangularImageSelection::calculatePoints() const
{
    return kpAbstractImageSelection::CalculatePointsForRectangle(boundingRect());
}

// public virtual [base kpAbstractImageSelection]
QBitmap kpRectangularImageSelection::shapeBitmap(bool nullForRectangular) const
{
    Q_ASSERT(boundingRect().isValid());

    if (nullForRectangular) {
        return {};
    }

    QBitmap maskBitmap(width(), height());
    maskBitmap.fill(Qt::color1 /*opaque*/);
    return maskBitmap;
}

// public virtual [kpAbstractImageSelection]
QRegion kpRectangularImageSelection::shapeRegion() const
{
    return QRegion(boundingRect(), QRegion::Rectangle);
}

// public virtual [kpAbstractSelection]
bool kpRectangularImageSelection::contains(const QPoint &point) const
{
    return boundingRect().contains(point);
}

// public virtual [kpAbstractSelection]
void kpRectangularImageSelection::paintBorder(QImage *destPixmap, const QRect &docRect, bool selectionFinished) const
{
    paintRectangularBorder(destPixmap, docRect, selectionFinished);
}

#include "moc_kpRectangularImageSelection.cpp"
