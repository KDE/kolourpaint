
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

#include <qpixmap.h>
#include <qwmatrix.h>

#include <kdebug.h>

#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>


kpSelection::kpSelection ()
    : QObject (),
      m_type (kpSelection::Rectangle),
      m_pixmap (0)
{
}

kpSelection::kpSelection (Type type, const QRect &rect, const QPixmap &pixmap)
    : QObject (),
      m_type (type),
      m_rect (rect)
{
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);
}

kpSelection::kpSelection (const QPointArray &points, const QPixmap &pixmap)
    : QObject (),
      m_type (Points),
      m_rect (points.boundingRect ()),
      m_points (points)
{
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);
}

kpSelection::kpSelection (const kpSelection &rhs)
    : QObject (),
      m_type (rhs.m_type),
      m_rect (rhs.m_rect),
      m_points (rhs.m_points),
      m_pixmap (rhs.m_pixmap ? new QPixmap (*rhs.m_pixmap) : 0)
{
    m_points.detach ();
}

kpSelection &kpSelection::operator= (const kpSelection &rhs)
{
    m_type = rhs.m_type;
    m_rect = rhs.m_rect;
    m_points = rhs.m_points;
    m_points.detach ();
    m_pixmap = rhs.m_pixmap ? new QPixmap (*rhs.m_pixmap) : 0;

    return *this;
}

kpSelection::~kpSelection ()
{
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
void kpSelection::moveBy (int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;

    QRect oldRect = boundingRect ();

    if (m_type == Points)
    {
        m_points.translate (dx, dy);
        m_rect = m_points.boundingRect ();
    }
    else
        m_rect.moveBy (dx, dy);

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
    if (m_pixmap &&
        (m_pixmap->width () != oldRect.width () ||
         m_pixmap->height () != oldRect.height ()))
    {
        if (m_type == kpSelection::Points)
        {
            QWMatrix matrix;
            matrix.scale (double (m_pixmap->width ()) / double (oldRect.width ()),
                          double (m_pixmap->height ()) / double (oldRect.height ()));

            m_points.translate (-oldRect.x (), -oldRect.y ());
            m_points = matrix.map (m_points);
            m_points.translate (oldRect.x (), oldRect.y ());
            m_rect = m_points.boundingRect ();
        }
        else
        {
            m_rect = QRect (m_rect.x (), m_rect.y (),
                            m_pixmap->width (), m_pixmap->height ());
        }
    }

    emit changed (oldRect);
    emit changed (boundingRect ());
}

#include <kpselection.moc>

