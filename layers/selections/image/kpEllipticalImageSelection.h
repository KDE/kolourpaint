
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEllipticalImageSelection_H
#define kpEllipticalImageSelection_H

#include "layers/selections/image/kpAbstractImageSelection.h"

class kpEllipticalImageSelection : public kpAbstractImageSelection
{
    Q_OBJECT

public:
    kpEllipticalImageSelection(const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpEllipticalImageSelection(const QRect &rect,
                               const kpImage &baseImage = kpImage(),
                               const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpEllipticalImageSelection(const QRect &rect, const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpEllipticalImageSelection(const kpEllipticalImageSelection &rhs);

    kpEllipticalImageSelection &operator=(const kpEllipticalImageSelection &rhs);

    kpEllipticalImageSelection *clone() const override;

    ~kpEllipticalImageSelection() override;

    //
    // Marshalling
    //

public:
    static const int SerialID = 1;
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
    struct kpEllipticalImageSelectionPrivate *const d;
};

#endif // kpEllipticalImageSelection_H
