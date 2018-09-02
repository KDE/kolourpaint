
// OPT: The selection classes should use copy-on-write.

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


#ifndef KPABSTRACTSELECTION_H
#define KPABSTRACTSELECTION_H


#include <QObject>

#include "commands/kpCommandSize.h"
#include "pixmapfx/kpPixmapFX.h"


class QImage;
class QPolygon;
class QPoint;
class QRect;
class QSize;


//
// Abstract base class for selections.
//
// Selections consist of:
//
// 1. Bounding rectangle (provided by this base class) relative to the
//    source document
// 2. Border (must be on, or inside, the bounding rectangle and does not
//    have to be rectangular)
// 3. Optional content (e.g. image or text).
//
// A selection without content is a selection border.
//
// Any content outside the border should not be rendered i.e. the selection
// is transparent in all areas outside of the border.  Pixels on, or inside,
// the border are considered to be renderable content.  Parts, or all, of
// this content can be transparent.
//
class kpAbstractSelection : public QObject
{
Q_OBJECT

//
// Initialization
//

protected:
    // (Call these in subclass constructors)
    kpAbstractSelection ();
    kpAbstractSelection (const QRect &rect);

    // (Call this in subclass implementations of operator=)
    kpAbstractSelection &operator= (const kpAbstractSelection &rhs);

public:
    // To implement, create an instance of your type and then call your
    // implementation of operator=().
    virtual kpAbstractSelection *clone () const = 0;

    ~kpAbstractSelection () override;


//
// Marshalling
//

public:
    // Returns a unique ID for this type of selection, use for marshalling.
    virtual int serialID () const = 0;

    // This is called after your object has been created with the default
    // constructor.
    //
    // Reads the object marshalled in the <stream> and returns whether it
    // succeeded.  This is called by kpSelectionFactory so the serialID()
    // has already been read and removed from the <stream>.
    //
    // You must override this.  Remember to call this base implementation
    // before your code.
    virtual bool readFromStream (QDataStream &stream);

    // Marshalls the object into the <stream>.  This is called by
    // operator<<() so the serialID() has already been written into the
    // <stream>.
    //
    // You must override this.  Remember to call this base implementation
    // before your code.
    virtual void writeToStream (QDataStream &stream) const;

    // Writes the serialID() of the <selection> to the <stream> and then
    // calls writeToStream() to do the remaining marshalling.
    //
    // (kpSelectionFactory::FromStream() is the ">>" replacement)
    friend QDataStream &operator<< (QDataStream &stream,
        const kpAbstractSelection &selection);


//
// General Queries
//

public:
    // Returns e.g. i18n ("Selection") or i18n ("Text").
    virtual QString name () const = 0;

    // Returns the memory usage of the selection (like kpCommand's),
    // _not_ its dimensions.
    //
    // You must override this and add the size returned by this implementation.
    virtual kpCommandSize::SizeType size () const;

public:
    // e.g. return false for an elliptical selection.
    virtual bool isRectangular () const = 0;


//
// Position & Dimensions
//

public:
    // Returns the minimum allowed dimensions of your selection type.
    // Usually this is 1x1 pixels by pixels.
    virtual int minimumWidth () const = 0;
    virtual int minimumHeight () const = 0;
    QSize minimumSize () const;

public:
    // (in document coordinates)
    int x () const;
    int y () const;
    QPoint topLeft () const;

public:
    // (in document coordinates)

    // Returns the width of the bounding rectangle.
    int width () const;

    // Returns the height of the bounding rectangle.
    int height () const;

    // Returns the bounding rectangle.
    QRect boundingRect () const;

public:
    // Use this to implement calculatePoints() for rectangular selections.
    static QPolygon CalculatePointsForRectangle (const QRect &rect);

    // Returns the border.  This may be recalculated for every call so
    // may be slow.
    virtual QPolygon calculatePoints () const = 0;


//
// Point Testing
//

public:
    // Returns whether the given <point> is on or inside the -- possibly,
    // non-rectangular -- border of the selection.
    //
    // (for non-rectangular selections, may return false even if
    //  kpView::onSelectionResizeHandle())
    virtual bool contains (const QPoint &point) const = 0;
    bool contains (int x, int y) const;


//
// Content
//

public:
    // i.e. Has an image or text - not just a border.
    virtual bool hasContent () const = 0;

    // Deletes the content, changing the selection back into a border.
    // If the selection has no content, it does nothing.
    virtual void deleteContent () = 0;


//
// Mutation - Movement
//

public:
    // (You only need to override this if you store your own border
    //  coordinates)
    virtual void moveBy (int dx, int dy);

    // (These call moveBy() so if you only reimplement moveBy(), that should
    //  be sufficient)
    void moveTo (int dx, int dy);
    void moveTo (const QPoint &topLeftPoint);


//
// Rendering
//

public:
    // Renders the selection on top of <*destPixmap>.  This does not render
    // the border.
    //
    // <docRect> is the document rectangle that <*destPixmap> represents.
    //
    // You need to clip to boundingRect() or if you are a non-rectangular
    // selection, an even smaller region,
    //
    // However, there is no need to do any explicit clipping to <docRect>,
    // since any drawing outside the bounds of <destPixmap> is discarded.
    // However, you may choose to clip for whatever reason e.g. performance.
    virtual void paint (QImage *destPixmap, const QRect &docRect) const = 0;


protected:
    // Use this to implement paintBorder() for rectangular selections.
    void paintRectangularBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const;

    // Use this to implement paintBorder() for non-rectangular selections
    // (this calls calculatePoints()).
    //
    // If <selectionFinished>, this also draws a bounding rectangular box.
    void paintPolygonalBorder (const QPolygon &points,
        QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const;

public:
    // Renders the selection border on top of <*destPixmap>.
    //
    // <docRect> is the same as for paint().
    //
    // If <selectionFinished> is false, the user is still dragging out the
    // selection border so it may be drawn differently.
    virtual void paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const = 0;


signals:
    // Signals that a view update is required in the document region <docRect>,
    // due to the selection changing.
    void changed (const QRect &docRect);


private:
    struct kpAbstractSelectionPrivate * const d;
};


#endif  // KPABSTRACTSELECTION_H
