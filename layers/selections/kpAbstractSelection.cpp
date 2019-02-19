
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


#include "layers/selections/kpAbstractSelection.h"

#include <QPolygon>
#include <QRect>

#include "kpLogCategories.h"

struct kpAbstractSelectionPrivate
{
    QRect rect;
};


// protected
kpAbstractSelection::kpAbstractSelection ()
    : QObject (),
      d (new kpAbstractSelectionPrivate ())
{
    d->rect = QRect ();
}

// protected
kpAbstractSelection::kpAbstractSelection (const QRect &rect)
    : QObject (),
      d (new kpAbstractSelectionPrivate ())
{
    d->rect = rect;
}

// protected
kpAbstractSelection &kpAbstractSelection::operator= (const kpAbstractSelection &rhs)
{
    if (this == &rhs) {
        return *this;
    }

    d->rect = rhs.d->rect;

    return *this;
}

// protected
kpAbstractSelection::~kpAbstractSelection ()
{
    delete d;
}


// public virtual
bool kpAbstractSelection::readFromStream (QDataStream &stream)
{
    stream >> d->rect;

    return true;
}

// public virtual
void kpAbstractSelection::writeToStream (QDataStream &stream) const
{
    stream << d->rect;
}

// friend
QDataStream &operator<< (QDataStream &stream, const kpAbstractSelection &selection)
{
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpAbstractSelection::operator<<(sel: rect=" <<
                 selection.boundingRect ();
#endif
    stream << selection.serialID ();
    selection.writeToStream (stream);
    return stream;
}


// public virtual
kpCommandSize::SizeType kpAbstractSelection::size () const
{
    return 0/*constant size*/;
}


// public
QSize kpAbstractSelection::minimumSize () const
{
    return  {minimumWidth (), minimumHeight ()};
}


// public
int kpAbstractSelection::x () const
{
    return d->rect.x ();
}

// public
int kpAbstractSelection::y () const
{
    return d->rect.y ();
}

// public
QPoint kpAbstractSelection::topLeft () const
{
    return d->rect.topLeft ();
}


// public
int kpAbstractSelection::width () const
{
    return boundingRect ().width ();
}

// public
int kpAbstractSelection::height () const
{
    return boundingRect ().height ();
}

// public
QRect kpAbstractSelection::boundingRect () const
{
    return d->rect;
}

// public static
QPolygon kpAbstractSelection::CalculatePointsForRectangle (const QRect &rect)
{
    QPolygon points;

    // OPT: not space optimal - current code adds duplicate corner points.

    // top
    for (int x = 0; x < rect.width (); x++) {
        points.append (QPoint (rect.x () + x, rect.top ()));
    }

    // right
    for (int y = 0; y < rect.height (); y++) {
        points.append (QPoint (rect.right (), rect.y () + y));
    }

    // bottom
    for (int x = rect.width () - 1; x >= 0; x--) {
        points.append (QPoint (rect.x () + x, rect.bottom ()));
    }

    // left
    for (int y = rect.height () - 1; y >= 0; y--) {
        points.append (QPoint (rect.left (), rect.y () + y));
    }

    return points;
}


// public
bool kpAbstractSelection::contains (int x, int y) const
{
    return contains (QPoint (x, y));
}


// public virtual
void kpAbstractSelection::moveBy (int dx, int dy)
{
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpAbstractSelection::moveBy(" << dx << "," << dy << ")";
#endif

    if (dx == 0 && dy == 0) {
        return;
    }

    QRect oldRect = boundingRect ();

#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "\toldRect=" << oldRect;
#endif

    d->rect.translate (dx, dy);
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "\tnewRect=" << d->rect;
#endif

    emit changed (oldRect);
    emit changed (boundingRect ());
}

// public
void kpAbstractSelection::moveTo (int dx, int dy)
{
    moveTo (QPoint (dx, dy));
}

// public
void kpAbstractSelection::moveTo (const QPoint &topLeftPoint)
{
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpAbstractSelection::moveTo(" << topLeftPoint << ")";
#endif
    QRect oldBoundingRect = boundingRect ();
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "\toldBoundingRect=" << oldBoundingRect;
#endif
    if (topLeftPoint == oldBoundingRect.topLeft ()) {
        return;
    }

    QPoint delta (topLeftPoint - oldBoundingRect.topLeft ());
    moveBy (delta.x (), delta.y ());
}

//---------------------------------------------------------------------

// protected
void kpAbstractSelection::paintRectangularBorder (QImage *destPixmap,
        const QRect &docRect,
        bool selectionFinished) const
{
    (void) selectionFinished;

#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpAbstractSelection::paintRectangularBorder() boundingRect="
              << boundingRect ();
#endif

#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "\tselection border = rectangle";
    qCDebug(kpLogLayers) << "\t\tx=" << boundingRect ().x () - docRect.x ()
              << " y=" << boundingRect ().y () - docRect.y ()
              << " w=" << boundingRect ().width ()
              << " h=" << boundingRect ().height ();
#endif
    kpPixmapFX::drawStippleRect(destPixmap,
        boundingRect ().x () - docRect.x (),
        boundingRect ().y () - docRect.y (),
        boundingRect ().width (),
        boundingRect ().height (),
        kpColor::Blue,
        kpColor::Yellow);
}

//---------------------------------------------------------------------

// protected
void kpAbstractSelection::paintPolygonalBorder (const QPolygon &points,
        QImage *destPixmap,
        const QRect &docRect,
        bool selectionFinished) const
{
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpAbstractSelection::paintPolygonalBorder() boundingRect="
              << boundingRect ();
#endif

    QPolygon pointsTranslated = points;
    pointsTranslated.translate (-docRect.x (), -docRect.y ());

    if ( !selectionFinished )
    {
      kpPixmapFX::drawPolyline(destPixmap,
        pointsTranslated, kpColor::Blue, 1/*pen width*/, kpColor::Yellow);
    }
    else
    {
      kpPixmapFX::drawPolygon(destPixmap,
          pointsTranslated,
          kpColor::Blue, 1/*pen width*/,
          kpColor::Invalid/*no background*/,
          true/*is final*/,
          kpColor::Yellow);

      kpPixmapFX::drawStippleRect(destPixmap,
          boundingRect ().x () - docRect.x (),
          boundingRect ().y () - docRect.y (),
          boundingRect ().width (),
          boundingRect ().height (),
          kpColor::LightGray,
          kpColor::DarkGray);
    }
}

