
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpFreeFormImageSelection_H
#define kpFreeFormImageSelection_H

#include "layers/selections/image/kpAbstractImageSelection.h"

class kpFreeFormImageSelection : public kpAbstractImageSelection
{
    Q_OBJECT

public:
    kpFreeFormImageSelection(const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpFreeFormImageSelection(const QPolygon &points,
                             const kpImage &baseImage = kpImage(),
                             const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpFreeFormImageSelection(const QPolygon &points, const kpImageSelectionTransparency &transparency = kpImageSelectionTransparency());

    kpFreeFormImageSelection(const kpFreeFormImageSelection &rhs);

    kpFreeFormImageSelection &operator=(const kpFreeFormImageSelection &rhs);

    kpFreeFormImageSelection *clone() const override;

    ~kpFreeFormImageSelection() override;

    //
    // Marshalling
    //

public:
    static const int SerialID = 2;
    int serialID() const override;

    bool readFromStream(QDataStream &stream) override;

    void writeToStream(QDataStream &stream) const override;

    //
    // General Queries
    //

public:
    kpCommandSize::SizeType size() const override;

    bool isRectangular() const override;

    // (as passed to the constructor)
    QPolygon originalPoints() const;

    //
    // Cardinally Adjacent Points
    //

protected:
    void recalculateCardinallyAdjacentPoints();

public:
    // Returns the originalPoints() interpolated to be cardinally adjacent.
    QPolygon cardinallyAdjacentPoints() const;

    // Returns cardinallyAdjacentPoints() but with extra points interpolated
    // from the last point to the first point (the original points are
    // thought of as a polygon where the first and last points are connected,
    // rather than as a string of points).
    //
    // As used by the shape mask methods.
    QPolygon cardinallyAdjacentPointsLoop() const;

    //
    // Position & Dimensions
    //

public:
    // Implements kpAbstractSelection interface - same as
    // cardinallyAdjacentPointsLoop ().
    // This implementation is fast.
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
    // Mutation
    //

public:
    void moveBy(int dx, int dy) override;

    void flip(bool horiz, bool vert) override;

    //
    // Rendering
    //

public:
    void paintBorder(QImage *destPixmap, const QRect &docRect, bool selectionFinished) const override;

private:
    struct kpFreeFormImageSelectionPrivate *const d;
};

#endif // kpFreeFormImageSelection_H
