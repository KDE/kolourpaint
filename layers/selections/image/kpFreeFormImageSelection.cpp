
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DEBUG_KP_SELECTION 0


#include "layers/selections/image/kpFreeFormImageSelection.h"

#include "kpLogCategories.h"

#include "imagelib/kpPainter.h"


struct kpFreeFormImageSelectionPrivate
{
    QPolygon orgPoints;

    // Various Qt methods that take a QPolygon interpolate points differently
    // (e.g. QPainter::drawPolygon() vs QRegion(QPolygon)) when given consecutive
    // points that are not cardinally adjacent e.g. these 2 points:
    //
    //          #
    //           #
    //
    // are diagonally, but not cardinally, adjacent.  They are rendered
    // inconsistently.  Also, points which are not adjacent at all definitely
    // require interpolation and are inconsistently rendered:
    //
    //          #
    //                         #
    //
    // So, we only pass cardinally interpolated points to those methods to
    // avoid this issue:
    //
    //           ##
    //            #
    //
    // These interpolated points are stored in <cardPointsCache>.  Regarding
    // <cardPointsLoopCache>, see the APIDoc for cardinallyAdjacentPointsLoop().
    QPolygon cardPointsCache, cardPointsLoopCache;
};


kpFreeFormImageSelection::kpFreeFormImageSelection (
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (transparency),
      d (new kpFreeFormImageSelectionPrivate ())
{
}

kpFreeFormImageSelection::kpFreeFormImageSelection (const QPolygon &points,
        const kpImage &baseImage,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (points.boundingRect (), baseImage, transparency),
      d (new kpFreeFormImageSelectionPrivate ())
{
    d->orgPoints = points;
    recalculateCardinallyAdjacentPoints ();
}

kpFreeFormImageSelection::kpFreeFormImageSelection (const QPolygon &points,
        const kpImageSelectionTransparency &transparency)
    : kpAbstractImageSelection (points.boundingRect (), transparency),
      d (new kpFreeFormImageSelectionPrivate ())
{
    d->orgPoints = points;
    recalculateCardinallyAdjacentPoints ();
}

kpFreeFormImageSelection::kpFreeFormImageSelection (const kpFreeFormImageSelection &rhs)
    : kpAbstractImageSelection (),
      d (new kpFreeFormImageSelectionPrivate ())
{
    *this = rhs;
}

kpFreeFormImageSelection &kpFreeFormImageSelection::operator= (const kpFreeFormImageSelection &rhs)
{
    kpAbstractImageSelection::operator= (rhs);

    d->orgPoints = rhs.d->orgPoints;
    d->cardPointsCache = rhs.d->cardPointsCache;
    d->cardPointsLoopCache = rhs.d->cardPointsLoopCache;

    return *this;
}

// public virtual [kpAbstractSelection]
kpFreeFormImageSelection *kpFreeFormImageSelection::clone () const
{
    kpFreeFormImageSelection *sel = new kpFreeFormImageSelection ();
    *sel = *this;
    return sel;
}

kpFreeFormImageSelection::~kpFreeFormImageSelection ()
{
    delete d;
}


// public virtual [kpAbstractSelection]
int kpFreeFormImageSelection::serialID () const
{
    return SerialID;
}

// public virtual [base kpAbstractImageSelection]
bool kpFreeFormImageSelection::readFromStream (QDataStream &stream)
{
    if (!kpAbstractImageSelection::readFromStream (stream)) {
        return false;
    }

    stream >> d->orgPoints;
    recalculateCardinallyAdjacentPoints ();

    return true;
}

// public virtual [base kpAbstractImageSelection]
void kpFreeFormImageSelection::writeToStream (QDataStream &stream) const
{
    kpAbstractImageSelection::writeToStream (stream);

    stream << d->orgPoints;
}


// public virtual [base kpAbstractImageSelection]
kpCommandSize::SizeType kpFreeFormImageSelection::size () const
{
    return kpAbstractImageSelection::size () +
        (kpCommandSize::PolygonSize (d->orgPoints) +
         kpCommandSize::PolygonSize (d->cardPointsCache) +
         kpCommandSize::PolygonSize (d->cardPointsLoopCache));
}

// public virtual [kpAbstractSelection]
bool kpFreeFormImageSelection::isRectangular () const
{
    return false;
}

// public
QPolygon kpFreeFormImageSelection::originalPoints () const
{
    return d->orgPoints;
}


static QPolygon RecalculateCardinallyAdjacentPoints (const QPolygon &points)
{
#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "kpFreeFormImageSelection.cpp:RecalculateCardinallyAdjacentPoints()";
    qCDebug(kpLogLayers) << "\tpoints=" << points;
#endif

    // Filter out duplicates.
    QPolygon noDups;
    for (const auto &p : points)
    {
        if (!noDups.isEmpty () && p == noDups.last ()) {
            continue;
        }

        noDups.append (p);
    }
#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "\twithout dups=" << noDups;
#endif

    // Interpolate to ensure cardinal adjacency.
    QPolygon cardPoints;
    for (const auto &p : noDups)
    {
        if (!cardPoints.isEmpty () &&
            !kpPainter::pointsAreCardinallyAdjacent (p, cardPoints.last ()))
        {
            const QPoint lastPoint = cardPoints.last ();

            QList <QPoint> interpPoints = kpPainter::interpolatePoints (
                lastPoint,
                p,
                true/*cardinal adjacency*/);

            Q_ASSERT (interpPoints.size () >= 2);
            Q_ASSERT (interpPoints [0] == lastPoint);
            Q_ASSERT (interpPoints.last () == p);

            for (int i = 1/*skip already existing point*/;
                 i < interpPoints.size ();
                 i++)
            {
                cardPoints.append (interpPoints [i]);
            }
        }
        else {
            cardPoints.append (p);
        }
    }
#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "\tcardinally adjacent=" << cardPoints;
#endif

    return cardPoints;
}

// protected
void kpFreeFormImageSelection::recalculateCardinallyAdjacentPoints ()
{
    d->cardPointsCache = ::RecalculateCardinallyAdjacentPoints (d->orgPoints);


    QPolygon pointsLoop = d->cardPointsCache;
    if (!pointsLoop.isEmpty ()) {
        pointsLoop.append (pointsLoop.first ());
    }

    // OPT: We know this method only needs to act on the last 2 points of
    //      "pointLoop", since the previous points are definitely cardinally
    //      adjacent.
    d->cardPointsLoopCache = ::RecalculateCardinallyAdjacentPoints (pointsLoop);
}

// public
QPolygon kpFreeFormImageSelection::cardinallyAdjacentPoints () const
{
    return d->cardPointsCache;
}

// public
QPolygon kpFreeFormImageSelection::cardinallyAdjacentPointsLoop () const
{
    return d->cardPointsLoopCache;
}


// public virtual [kpAbstractSelection]
QPolygon kpFreeFormImageSelection::calculatePoints () const
{
    return d->cardPointsLoopCache;
}


// protected virtual [kpAbstractSelection]
QRegion kpFreeFormImageSelection::shapeRegion () const
{
    const QRegion region = QRegion (d->cardPointsLoopCache, Qt::OddEvenFill);

    // In Qt4, while QPainter::drawRect() gives you rectangles 1 pixel
    // wider and higher, QRegion(QPolygon) gives you regions 1 pixel
    // narrower and shorter!  Compensate for this by merging shifted
    // versions of the region.  This seems to be consistent with shapeBitmap()
    // but I am a bit worried.
    //
    // Regarding alternative solutions:
    // 1. Instead of doing this region shifting and merging, if we were to
    //    construct a QRegion simply from a point array with 4 points for
    //    every point in "d->cardPointsLoopCache" (4 points = original point + 3
    //    translations below), it probably wouldn't work because the order of
    //    the points in any point array matter for the odd-even fill
    //    algorithm.  This would probably manifest as problems with
    //    self-intersecting borders.
    // 2. Constructing a QRegion from QBitmap (from shapeBitmap()) is probably
    //    very slow since it would have to read each pixel of the QBitmap.
    //    Having said that, this is probably the safest option as region shifting
    //    is dodgy.  Also, this would guarantee that shapeBitmap() and shapeRegion()
    //    are consistent and we wouldn't need cardinally adjacent points either
    //    (d->cardPointsCache and d->cardPointsLoopCache).
    const QRegion regionX = region.translated (1, 0);
    const QRegion regionY = region.translated (0, 1);
    const QRegion regionXY = region.translated (1, 1);

    return region.united (regionX).united (regionY).united (regionXY);
}


// public virtual [kpAbstractSelection]
bool kpFreeFormImageSelection::contains (const QPoint &point) const
{
    if (!boundingRect ().contains (point)) {
        return false;
    }

    // We can't use the baseImage() (when non-null) and get the transparency of
    // the pixel at <point>, instead of this region test, as the pixel may be
    // transparent but still within the border.
    return shapeRegion ().contains (point);
}


// public virtual [base kpAbstractSelection]
void kpFreeFormImageSelection::moveBy (int dx, int dy)
{
    d->orgPoints.translate (dx, dy);

    d->cardPointsCache.translate (dx, dy);
    d->cardPointsLoopCache.translate (dx, dy);

    // Call base last since it fires the changed() signal and we only
    // want that to fire at the very end of this method, after all
    // the selection state has been changed.
    kpAbstractImageSelection::moveBy (dx, dy);
}

//---------------------------------------------------------------------

static void FlipPoints (QPolygon *points,
        bool horiz, bool vert,
        const QRect &oldRect)
{
    points->translate (-oldRect.x (), -oldRect.y ());

    const QTransform matrix (horiz ? -1 : +1,  // m11
                          0,  // m12
                          0,  // m21
                          vert  ? -1 : +1,  // m22
                          horiz ? (oldRect.width() - 1) : 0,  // dx
                          vert  ? (oldRect.height() - 1) : 0);  // dy

#if !defined (QT_NO_DEBUG) && !defined (NDEBUG)
    QPolygon oldPoints = *points;
#endif

    *points = matrix.map (*points);

#if !defined (QT_NO_DEBUG) && !defined (NDEBUG)
    // Sanity check: flipping the points twice gives us the original points.
    Q_ASSERT (oldPoints == matrix.map (*points));
#endif

    points->translate (oldRect.x (), oldRect.y ());
}

//---------------------------------------------------------------------

// public virtual [base kpAbstractImageSelection]
void kpFreeFormImageSelection::flip (bool horiz, bool vert)
{
    ::FlipPoints (&d->orgPoints, horiz, vert, boundingRect ());

    ::FlipPoints (&d->cardPointsCache, horiz, vert, boundingRect ());
    ::FlipPoints (&d->cardPointsLoopCache, horiz, vert, boundingRect ());


    // Call base last since it fires the changed() signal and we only
    // want that to fire at the very end of this method, after all
    // the selection state has been changed.
    kpAbstractImageSelection::flip (horiz, vert);
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
void kpFreeFormImageSelection::paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const
{
    if (selectionFinished) {
        paintPolygonalBorder (cardinallyAdjacentPointsLoop (),
            destPixmap, docRect, selectionFinished);
    }
    else {
        paintPolygonalBorder (cardinallyAdjacentPoints (),
            destPixmap, docRect, selectionFinished);
    }
}


