
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpRectangularImageSelection_H
#define kpRectangularImageSelection_H

#include "layers/selections/image/kpAbstractImageSelection.h"

class kpRectangularImageSelection : public kpAbstractImageSelection
{
    Q_OBJECT

public:
    kpRectangularImageSelection(const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpRectangularImageSelection(const QRect &rect,
                                const kpImage &baseImage = kpImage(),
                                const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpRectangularImageSelection(const QRect &rect, const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpRectangularImageSelection(const kpRectangularImageSelection &rhs);

    kpRectangularImageSelection &operator=(const kpRectangularImageSelection &rhs);

    kpRectangularImageSelection *clone() const override;

    ~kpRectangularImageSelection() override;

    //
    // Marshalling
    //

public:
    static const int SerialID = 0;
    int serialID() const override;

    //
    // General Queries
    //

public:
    bool isRectangular() const override;

    //
    // Position & Dimensions
    //

public:
    QPolygon calculatePoints() const override;

    //
    // Shape Mask
    //

public:
    QBitmap shapeBitmap(bool nullForRectangular = false) const override;

    QRegion shapeRegion() const override;

    //
    // Point Testing
    //

public:
    bool contains(const QPoint &point) const override;

    //
    // Rendering
    //

public:
    void paintBorder(QImage *destPixmap, const QRect &docRect, bool selectionFinished) const override;

private:
    struct kpRectangularImageSelectionPrivate *const d;
};

#endif // kpRectangularImageSelection_H
