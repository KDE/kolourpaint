
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_SELECTION 1

#include <qimage.h>
#include <qpainter.h>
#include <qwmatrix.h>

#include <kdebug.h>

#include <kpcolorsimilaritydialog.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>


kpSelection::kpSelection (const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (kpSelection::Rectangle),
      m_pixmap (0)
{
    setTransparency (transparency);
}

kpSelection::kpSelection (Type type, const QRect &rect, const QPixmap &pixmap,
                          const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (type),
      m_rect (rect)
{
    calculatePoints ();
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);

    setTransparency (transparency);
}

kpSelection::kpSelection (Type type, const QRect &rect, const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (type),
      m_rect (rect),
      m_pixmap (0)
{
    calculatePoints ();

    setTransparency (transparency);
}

kpSelection::kpSelection (const QPointArray &points, const QPixmap &pixmap,
                          const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (Points),
      m_rect (points.boundingRect ()),
      m_points (points)
{
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);
    m_points.detach ();

    setTransparency (transparency);
}

kpSelection::kpSelection (const QPointArray &points, const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (Points),
      m_rect (points.boundingRect ()),
      m_points (points),
      m_pixmap (0)
{
    m_points.detach ();

    setTransparency (transparency);
}

kpSelection::kpSelection (const kpSelection &rhs)
    : QObject (),
      m_type (rhs.m_type),
      m_rect (rhs.m_rect),
      m_points (rhs.m_points),
      m_pixmap (rhs.m_pixmap ? new QPixmap (*rhs.m_pixmap) : 0),
      m_textLines (rhs.m_textLines),
      m_textStyle (rhs.m_textStyle),
      m_transparency (rhs.m_transparency),
      m_transparencyMask (rhs.m_transparencyMask)
{
    m_points.detach ();
}

kpSelection &kpSelection::operator= (const kpSelection &rhs)
{
    if (this == &rhs)
        return *this;

    m_type = rhs.m_type;
    m_rect = rhs.m_rect;
    m_points = rhs.m_points;
    m_points.detach ();

    delete m_pixmap;
    m_pixmap = rhs.m_pixmap ? new QPixmap (*rhs.m_pixmap) : 0;

    m_textLines = rhs.m_textLines;
    m_textStyle = rhs.m_textStyle;

    m_transparency = rhs.m_transparency;
    m_transparencyMask = rhs.m_transparencyMask;

    return *this;
}


// friend
QDataStream &operator<< (QDataStream &stream, const kpSelection &selection)
{
    stream << int (selection.m_type);
    stream << selection.m_rect;
    stream << selection.m_points;
    if (selection.m_pixmap)
        stream << kpPixmapFX::convertToImage (*selection.m_pixmap);
    else
        stream << QImage ();

    return stream;
}

// friend
QDataStream &operator>> (QDataStream &stream, kpSelection &selection)
{
    selection.readFromStream (stream);
    return stream;
}

// public
void kpSelection::readFromStream (QDataStream &stream,
                                  const kpPixmapFX::WarnAboutLossInfo &wali)
{
    int typeAsInt;
    stream >> typeAsInt;
    m_type = kpSelection::Type (typeAsInt);

    stream >> m_rect;
    stream >> m_points;
    m_points.detach ();

    QImage image;
    stream >> image;
    delete m_pixmap;
    if (!image.isNull ())
        m_pixmap = new QPixmap (kpPixmapFX::convertToPixmap (image, false/*no dither*/, wali));
    else
        m_pixmap = 0;
}

kpSelection::~kpSelection ()
{
}


// private
void kpSelection::calculatePoints ()
{
    if (m_type == kpSelection::Points)
        return;

    if (m_type == kpSelection::Ellipse)
    {
        m_points.makeEllipse (m_rect.x (), m_rect.y (),
                              m_rect.width (), m_rect.height ());
        return;
    }

    if (m_type == kpSelection::Rectangle)
    {
        // OPT: not space optimal - redoes corners
        m_points.resize (m_rect.width () * 2 + m_rect.height () * 2);

        int pointsUpto = 0;

        // top
        for (int x = 0; x < m_rect.width (); x++)
            m_points [pointsUpto++] = QPoint (m_rect.x () + x, m_rect.top ());

        // right
        for (int y = 0; y < m_rect.height (); y++)
            m_points [pointsUpto++] = QPoint (m_rect.right (), m_rect.y () + y);

        // bottom
        for (int x = m_rect.width () - 1; x >= 0; x--)
            m_points [pointsUpto++] = QPoint (m_rect.x () + x, m_rect.bottom ());

        // left
        for (int y = m_rect.height () - 1; y >= 0; y--)
            m_points [pointsUpto++] = QPoint (m_rect.left (), m_rect.y () + y);

        return;
    }

    kdError () << "kpSelection::calculatePoints() with unknown type" << endl;
    return;
}


// public
kpSelection::Type kpSelection::type () const
{
    return m_type;
}

// public
QPoint kpSelection::topLeft () const
{
    return m_rect.topLeft ();
}

// public
QPoint kpSelection::point () const
{
    return m_rect.topLeft ();
}


// public
int kpSelection::x () const
{
    return m_rect.x ();
}

// public
int kpSelection::y () const
{
    return m_rect.y ();
}


// public
void kpSelection::moveBy (int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;

    QRect oldRect = boundingRect ();

    m_rect.moveBy (dx, dy);
    m_points.translate (dx, dy);

    emit changed (oldRect);
    emit changed (boundingRect ());
}

// public
void kpSelection::moveTo (int dx, int dy)
{
    moveTo (QPoint (dx, dy));
}

// public
void kpSelection::moveTo (const QPoint &topLeftPoint)
{
    QRect oldBoundingRect = boundingRect ();
    if (topLeftPoint == oldBoundingRect.topLeft ())
        return;

    QPoint delta (topLeftPoint - oldBoundingRect.topLeft ());
    moveBy (delta.x (), delta.y ());
}


// public
QPointArray kpSelection::points () const
{
    return m_points;
}

// public
QPointArray kpSelection::pointArray () const
{
    return m_points;
}

// public
QRect kpSelection::boundingRect () const
{
    return m_rect;
}

// public
int kpSelection::width () const
{
    return boundingRect ().width ();
}

// public
int kpSelection::height () const
{
    return boundingRect ().height ();
}

// public
bool kpSelection::contains (const QPoint &point) const
{
    QRect rect = boundingRect ();

#if DEBUG_KP_SELECTION && 0
    kdDebug () << "kpSelection::contains(" << point
               << ") rect==" << rect
               << endl;
#endif

    if (!rect.contains (point))
        return false;

    // OPT: QRegion is probably incredibly slow - cache
    // We can't use the m_pixmap (if avail) and get the transparency of
    // the pixel at that point as it may be transparent but still within the
    // border
    switch (m_type)
    {
    case kpSelection::Rectangle:
        return true;
    case kpSelection::Ellipse:
        return QRegion (m_rect, QRegion::Ellipse).contains (point);
    case kpSelection::Points:
        return QRegion (m_points, false/*even-odd algo*/).contains (point);
    default:
        return false;
    }
}

// public
bool kpSelection::contains (int x, int y)
{
    return contains (QPoint (x, y));
}


// public
QPixmap *kpSelection::pixmap () const
{
    return m_pixmap;
}

// public
void kpSelection::setPixmap (const QPixmap &pixmap)
{
    delete m_pixmap;
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);

    QRect oldRect = boundingRect ();
    emit changed (oldRect);

    if (m_pixmap &&
        (m_pixmap->width () != oldRect.width () ||
         m_pixmap->height () != oldRect.height ()))
    {
        kdError () << "kpSelection::setPixmap() changes the size of the selection!"
                   << "   old:"
                   << " w=" << oldRect.width ()
                   << " h=" << oldRect.height ()
                   << "   new:"
                   << " w=" << m_pixmap->width ()
                   << " h=" << m_pixmap->height ()
                   << endl;

        m_type = kpSelection::Rectangle;
        m_rect = QRect (m_rect.x (), m_rect.y (),
                        m_pixmap->width (), m_pixmap->height ());
        calculatePoints ();

        emit changed (boundingRect ());
    }

    calculateTransparencyMask ();
}


// public
QPixmap kpSelection::opaquePixmap () const
{
    if (pixmap ())
    {
        return *pixmap ();
    }
    else
    {
        return QPixmap ();
    }
}

// private
void kpSelection::calculateTransparencyMask ()
{
#if DEBUG_KP_SELECTION
    kdDebug () << "kpSelection::calculateTransparencyMask()" << endl;
#endif


    if (!m_pixmap)
    {
    #if DEBUG_KP_SELECTION
        kdDebug () << "\tno pixmap - no need for transparency mask" << endl;
    #endif
        m_transparencyMask.resize (0, 0);
        return;
    }

    if (m_transparency.isOpaque ())
    {
    #if DEBUG_KP_SELECTION
        kdDebug () << "\topaque - no need for transparency mask" << endl;
    #endif
        m_transparencyMask.resize (0, 0);
        return;
    }

    m_transparencyMask.resize (m_pixmap->width (), m_pixmap->height ());

    QImage image = kpPixmapFX::convertToImage (*m_pixmap);
    QPainter transparencyMaskPainter (&m_transparencyMask);

    bool hasTransparent = false;
    for (int y = 0; y < m_pixmap->height (); y++)
    {
        for (int x = 0; x < m_pixmap->width (); x++)
        {
            if (kpPixmapFX::getColorAtPixel (image, x, y).isSimilarTo (m_transparency.transparentColor (),
                                                                       m_transparency.processedColorSimilarity ()))
            {
                transparencyMaskPainter.setPen (Qt::color1/*make it transparent*/);
                hasTransparent = true;
            }
            else
            {
                transparencyMaskPainter.setPen (Qt::color0/*keep pixel as is*/);
            }

            transparencyMaskPainter.drawPoint (x, y);
        }
    }

    transparencyMaskPainter.end ();

    if (!hasTransparent)
    {
    #if DEBUG_KP_SELECTION
        kdDebug () << "\tcolour useless - completely opaque" << endl;
    #endif
        m_transparencyMask.resize (0, 0);
        return;
    }
}

// public
QPixmap kpSelection::transparentPixmap () const
{
    QPixmap pixmap = opaquePixmap ();

    if (!m_transparencyMask.isNull ())
    {
        kpPixmapFX::paintMaskTransparentWithBrush (&pixmap, QPoint (0, 0),
                                                   m_transparencyMask);
    }

    return pixmap;
}

// public
kpSelectionTransparency kpSelection::transparency () const
{
    return m_transparency;
}

// public
bool kpSelection::setTransparency (const kpSelectionTransparency &transparency,
                                   bool checkTransparentPixmapChanged)
{
    if (m_transparency == transparency)
        return false;

    m_transparency = transparency;

    bool haveChanged = true;

    QBitmap oldTransparencyMask = m_transparencyMask;
    calculateTransparencyMask ();


    if (oldTransparencyMask.width () == m_transparencyMask.width () &&
        oldTransparencyMask.height () == m_transparencyMask.height ())
    {
        if (m_transparencyMask.isNull ())
        {
        #if DEBUG_KP_SELECTION
            kdDebug () << "\tboth old and new pixmaps are null - nothing changed" << endl;
        #endif
            haveChanged = false;
        }
        else if (checkTransparentPixmapChanged)
        {
            QImage oldTransparencyMaskImage = kpPixmapFX::convertToImage (oldTransparencyMask);
            QImage newTransparencyMaskImage = kpPixmapFX::convertToImage (m_transparencyMask);

            bool changed = false;
            for (int y = 0; y < oldTransparencyMaskImage.height () && !changed; y++)
            {
                for (int x = 0; x < oldTransparencyMaskImage.width () && !changed; x++)
                {
                    if (kpPixmapFX::getColorAtPixel (oldTransparencyMaskImage, x, y) !=
                        kpPixmapFX::getColorAtPixel (newTransparencyMaskImage, x, y))
                    {
                    #if DEBUG_KP_SELECTION
                        kdDebug () << "\tdiffer at " << QPoint (x, y)
                                   << " old=" << (int *) kpPixmapFX::getColorAtPixel (oldTransparencyMaskImage, x, y).toQRgb ()
                                   << " new=" << (int *) kpPixmapFX::getColorAtPixel (newTransparencyMaskImage, x, y).toQRgb ()
                                   << endl;
                    #endif
                        changed = true;
                        break;
                    }
                }
            }

            if (!changed)
                haveChanged = false;
        }
    }


    if (haveChanged)
        emit changed (boundingRect ());

    return haveChanged;
}


// private
void kpSelection::flipPoints (bool horiz, bool vert)
{
    QRect oldRect = boundingRect ();

    m_points.translate (-oldRect.x (), -oldRect.y ());

    const QWMatrix matrix = kpPixmapFX::flipMatrix (oldRect.width (), oldRect.height (),
                                                    horiz, vert);
    m_points = matrix.map (m_points);

    m_points.translate (oldRect.x (), oldRect.y ());
}


// public
void kpSelection::flip (bool horiz, bool vert)
{
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "kpSelection::flip(horiz=" << horiz
               << ",vert=" << vert << ")" << endl;
#endif

    flipPoints (horiz, vert);


    if (m_pixmap)
    {
    #if DEBUG_KP_SELECTION && 1
        kdDebug () << "\thave pixmap - flipping that" << endl;
    #endif
        kpPixmapFX::flip (m_pixmap, horiz, vert);
    }

    if (!m_transparencyMask.isNull ())
    {
    #if DEBUG_KP_SELECTION && 1
        kdDebug () << "\thave transparency mask - flipping that" << endl;
    #endif
        kpPixmapFX::flip (&m_transparencyMask, horiz, vert);
    }


    emit changed (boundingRect ());
}

#include <kpselection.moc>

