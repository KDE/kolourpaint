
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


#define DEBUG_KP_SELECTION 0


#include <kpselection.h>

#include <qfont.h>
#include <qimage.h>
#include <qpainter.h>
#include <qwmatrix.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolorsimilaritydialog.h>
#include <kpdefs.h>
#include <kppixmapfx.h>
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

kpSelection::kpSelection (const QRect &rect,
                          const QValueVector <QString> &textLines_,
                          const kpTextStyle &textStyle_)
    : QObject (),
      m_type (Text),
      m_rect (rect),
      m_pixmap (0),
      m_textStyle (textStyle_)
{
    calculatePoints ();

    setTextLines (textLines_);
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
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "kpSelection::operator<<(sel: rect=" << selection.boundingRect ()
               << " pixmap rect=" << (selection.pixmap () ? selection.pixmap ()->rect () : QRect ())
               << endl;
#endif
    stream << int (selection.m_type);
    stream << selection.m_rect;
    stream << selection.m_points;
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\twrote type=" << int (selection.m_type) << " rect=" << selection.m_rect
               << " and points" << endl;
#endif

    // TODO: need for text?
    //       For now we just use QTextDrag for Text Selections so this point is mute.
    if (selection.m_pixmap)
    {
        const QImage image = kpPixmapFX::convertToImage (*selection.m_pixmap);
    #if DEBUG_KP_SELECTION && 1
        kdDebug () << "\twrote image rect=" << image.rect () << endl;
    #endif
        stream << image;
    }
    else
    {
    #if DEBUG_KP_SELECTION && 1
        kdDebug () << "\twrote no image because no pixmap" << endl;
    #endif
        stream << QImage ();
    }

    //stream << selection.m_textLines;
    //stream << selection.m_textStyle;

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
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "kpSelection::readFromStream()" << endl;
#endif
    int typeAsInt;
    stream >> typeAsInt;
    m_type = kpSelection::Type (typeAsInt);
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\ttype=" << typeAsInt << endl;
#endif

    stream >> m_rect;
    stream >> m_points;
    m_points.detach ();
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\trect=" << m_rect << endl;
    //kdDebug () << "\tpoints=" << m_points << endl;
#endif

    QImage image;
    stream >> image;
    delete m_pixmap;
    if (!image.isNull ())
        m_pixmap = new QPixmap (kpPixmapFX::convertToPixmap (image, false/*no dither*/, wali));
    else
        m_pixmap = 0;
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\timage: w=" << image.width () << " h=" << image.height ()
               << " depth=" << image.depth () << endl;
    if (m_pixmap)
    {
        kdDebug () << "\tpixmap: w=" << m_pixmap->width () << " h=" << m_pixmap->height ()
                   << endl;
    }
    else
    {
        kdDebug () << "\tpixmap: none" << endl;
    }
#endif

    //stream >> m_textLines;
    //stream >> m_textStyle;
}

kpSelection::~kpSelection ()
{
    delete m_pixmap; m_pixmap = 0;
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

    if (m_type == kpSelection::Rectangle || m_type == kpSelection::Text)
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
bool kpSelection::isRectangular () const
{
    return (m_type == Rectangle || m_type == Text);
}

// public
bool kpSelection::isText () const
{
    return (m_type == Text);
}

// public
QString kpSelection::name () const
{
    if (m_type == Text)
        return i18n ("Text");

    return i18n ("Selection");
}


// public
int kpSelection::size () const
{
    return kpPixmapFX::pointArraySize (m_points) +
           kpPixmapFX::pixmapSize (m_pixmap) +
           kpPixmapFX::stringSize (text ()) +
           kpPixmapFX::pixmapSize (m_transparencyMask);
}


// public
QBitmap kpSelection::maskForOwnType (bool nullForRectangular) const
{
    if (!m_rect.isValid ())
    {
        kdError () << "kpSelection::maskForOwnType() boundingRect invalid" << endl;
        return QBitmap ();
    }


    if (isRectangular ())
    {
        if (nullForRectangular)
            return QBitmap ();

        QBitmap maskBitmap (m_rect.width (), m_rect.height ());
        maskBitmap.fill (Qt::color1/*opaque*/);
        return maskBitmap;
    }


    QBitmap maskBitmap (m_rect.width (), m_rect.height ());
    maskBitmap.fill (Qt::color0/*transparent*/);

    QPainter painter;
    painter.begin (&maskBitmap);
    painter.setPen (Qt::color1)/*opaque*/;
    painter.setBrush (Qt::color1/*opaque*/);

    if (m_type == kpSelection::Ellipse)
        painter.drawEllipse (0, 0, m_rect.width (), m_rect.height ());
    else if (m_type == kpSelection::Points)
    {
        QPointArray points = m_points;
        points.detach ();
        points.translate (-m_rect.x (), -m_rect.y ());

        painter.drawPolygon (points, false/*even-odd algo*/);
    }

    painter.end ();


    return maskBitmap;
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
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "kpSelection::moveBy(" << dx << "," << dy << ")" << endl;
#endif

    if (dx == 0 && dy == 0)
        return;

    QRect oldRect = boundingRect ();

#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\toldRect=" << oldRect << endl;
#endif

    m_rect.moveBy (dx, dy);
    m_points.translate (dx, dy);
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\tnewRect=" << m_rect << endl;
#endif

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
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "kpSelection::moveTo(" << topLeftPoint << ")" << endl;
#endif
    QRect oldBoundingRect = boundingRect ();
#if DEBUG_KP_SELECTION && 1
    kdDebug () << "\toldBoundingRect=" << oldBoundingRect << endl;
#endif
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
    case kpSelection::Text:
        return true;
    case kpSelection::Ellipse:
        return QRegion (m_rect, QRegion::Ellipse).contains (point);
    case kpSelection::Points:
        // TODO: make this always include the border
        //       (draw up a rect sel in this mode to see what I mean)
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
QPixmap *kpSelection::pixmap (bool evenIfText) const
{
    if (m_type != kpSelection::Text ||
        (m_type == kpSelection::Text && evenIfText))
    {
        return m_pixmap;
    }
    else
    {
        return 0;
    }
}

// public
void kpSelection::setPixmap (const QPixmap &pixmap)
{
    delete m_pixmap;
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);

    QRect oldRect = boundingRect ();
    emit changed (oldRect);

    const bool changedSize = (m_pixmap &&
                              (m_pixmap->width () != oldRect.width () ||
                               m_pixmap->height () != oldRect.height ()));
    const bool changedFromText = (m_type == Text);
    if (changedSize || changedFromText)
    {
        if (changedSize)
        {
            kdError () << "kpSelection::setPixmap() changes the size of the selection!"
                       << "   old:"
                       << " w=" << oldRect.width ()
                       << " h=" << oldRect.height ()
                       << "   new:"
                       << " w=" << m_pixmap->width ()
                       << " h=" << m_pixmap->height ()
                       << endl;
        }

    #if DEBUG_KP_SELECTION && 1
        if (changedFromText)
        {
            kdDebug () << "kpSelection::setPixmap() changed from text" << endl;
        }
    #endif

        m_type = kpSelection::Rectangle;
        m_rect = QRect (m_rect.x (), m_rect.y (),
                        m_pixmap->width (), m_pixmap->height ());
        calculatePoints ();

        m_textLines = QValueVector <QString> ();

        emit changed (boundingRect ());
    }

    calculateTransparencyMask ();
}

// private
void kpSelection::calculateTextPixmap ()
{
    if (!isText ())
    {
        kdError () << "kpSelection::calculateTextPixmap() not a text selection" << endl;
        return;
    }

#if DEBUG_KP_SELECTION
    kdDebug () << "kpSelection::calculateTextPixmap() textStyle: fcol="
               << (int *) m_textStyle.foregroundColor ().toQRgb ()
               << " bcol="
               << (int *) m_textStyle.effectiveBackgroundColor ().toQRgb ()
               << endl;
#endif

    delete m_pixmap;
    m_pixmap = new QPixmap (m_rect.width (), m_rect.height ());
    QBitmap pixmapMask;


    // Iron out stupid case first
    if (m_textStyle.foregroundColor ().isTransparent () &&
        m_textStyle.effectiveBackgroundColor ().isTransparent ())
    {
        pixmapMask.resize (m_pixmap->width (), m_pixmap->height ());
        pixmapMask.fill (Qt::color0/*transparent*/);
        m_pixmap->setMask (pixmapMask);
        return;
    }


    QFont font = m_textStyle.font ();
    // TODO: why doesn't "font.setStyleStrategy (QFont::NoAntialias);"
    //       let us avoid the hack below?


    QPainter pixmapPainter, pixmapMaskPainter;

    if (m_textStyle.foregroundColor ().isOpaque () ||
        m_textStyle.effectiveBackgroundColor ().isOpaque ())
    {
        if (m_textStyle.effectiveBackgroundColor ().isOpaque ())
            m_pixmap->fill (m_textStyle.effectiveBackgroundColor ().toQColor ());
        else
            m_pixmap->fill (Qt::black);  // see hack below

        pixmapPainter.begin (m_pixmap);
        if (m_textStyle.foregroundColor ().isOpaque ())
        {
            pixmapPainter.setPen (m_textStyle.foregroundColor ().toQColor ());
        }
        else
        {
            // HACK: Transparent foreground colour + antialiased fonts don't
            // work - they don't seem to be able to draw in
            // Qt::color0/*transparent*/ (but Qt::color1 seems Ok).
            // So we draw in a contrasting color to the background so that
            // we can identify the transparent pixels for manually creating
            // the mask.
            if (m_textStyle.effectiveBackgroundColor ().isTransparent ())
                pixmapPainter.setPen (Qt::white);
            else
                pixmapPainter.setPen (
                    QColor ((m_textStyle.effectiveBackgroundColor ().toQRgb () & RGB_MASK) ^ 0xFFFFFF));
        }
        pixmapPainter.setFont (font);
    }

    if (m_textStyle.foregroundColor ().isTransparent () ||
        m_textStyle.effectiveBackgroundColor ().isTransparent ())
    {
        pixmapMask.resize (m_rect.width (), m_rect.height ());
        pixmapMask.fill (m_textStyle.effectiveBackgroundColor ().maskColor ());

        pixmapMaskPainter.begin (&pixmapMask);
    #if DEBUG_KP_SELECTION
        kdDebug () << "\tfcol.maskColor="
                   << (int *) m_textStyle.foregroundColor ().maskColor ().rgb ()
                   << endl;
    #endif
        pixmapMaskPainter.setPen (m_textStyle.foregroundColor ().maskColor ());
        pixmapMaskPainter.setFont (font);
    }


#define PAINTER_CALL(cmd)               \
{                                       \
    if (pixmapPainter.isActive ())      \
        pixmapPainter . cmd ;           \
                                        \
    if (pixmapMaskPainter.isActive ())  \
        pixmapMaskPainter . cmd ;       \
}
    QRect rect (textAreaRect ());
    rect.moveBy (-m_rect.x (), -m_rect.y ());
    PAINTER_CALL (drawText (rect, 0/*flags*/, text ()));
#undef PAINTER_CAL


    if (pixmapPainter.isActive ())
        pixmapPainter.end ();

    if (pixmapMaskPainter.isActive ())
        pixmapMaskPainter.end ();


    if (m_textStyle.foregroundColor ().isTransparent ())
    {
    #if DEBUG_KP_SELECTION
        kdDebug () << "\tinvoking foreground transparency hack" << endl;
    #endif
        QImage image = kpPixmapFX::convertToImage (*m_pixmap);
        QRgb backgroundRGB = image.pixel (0, 0);  // on textBorderSize()

        pixmapMaskPainter.begin (&pixmapMask);
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                if (image.pixel (x, y) == backgroundRGB)
                    pixmapMaskPainter.setPen (Qt::color1/*opaque*/);
                else
                    pixmapMaskPainter.setPen (Qt::color0/*transparent*/);

                pixmapMaskPainter.drawPoint (x, y);
            }
        }
        pixmapMaskPainter.end ();
    }


    if (!pixmapMask.isNull ())
        m_pixmap->setMask (pixmapMask);
}

// public static
QString kpSelection::textForTextLines (const QValueVector <QString> &textLines_)
{
    if (textLines_.isEmpty ())
        return QString::null;

    QString bigString = textLines_ [0];

    for (QValueVector <QString>::const_iterator it = textLines_.begin () + 1;
         it != textLines_.end ();
         it++)
    {
        bigString += QString::fromLatin1 ("\n");
        bigString += (*it);
    }

    return bigString;
}

// public
QString kpSelection::text () const
{
    if (!isText ())
    {
        return QString::null;
    }

    return textForTextLines (m_textLines);
}

// public
QValueVector <QString> kpSelection::textLines () const
{
    if (!isText ())
    {
        kdError () << "kpSelection::textLines() not a text selection" << endl;
        return QValueVector <QString> ();
    }

    return m_textLines;
}

// public
void kpSelection::setTextLines (const QValueVector <QString> &textLines_)
{
    if (!isText ())
    {
        kdError () << "kpSelection::setTextLines() not a text selection" << endl;
        return;
    }

    m_textLines = textLines_;
    if (m_textLines.isEmpty ())
    {
        kdError () << "kpSelection::setTextLines() passed no lines" << endl;
        m_textLines.push_back (QString::null);
    }
    calculateTextPixmap ();
    emit changed (boundingRect ());
}

// public static
int kpSelection::textBorderSize ()
{
    return 4;
}

// public
QRect kpSelection::textAreaRect () const
{
    if (!isText ())
    {
        kdError () << "kpSelection::textAreaRect() not a text selection" << endl;
        return QRect ();
    }

    return QRect (m_rect.x () + textBorderSize (),
                  m_rect.y () + textBorderSize (),
                  m_rect.width () - textBorderSize () * 2,
                  m_rect.height () - textBorderSize () * 2);
}

// public
bool kpSelection::pointIsInTextBorderArea (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kdError () << "kpSelection::pointIsInTextBorderArea() not a text selection" << endl;
        return false;
    }

    return (m_rect.contains (globalPoint) && !pointIsInTextArea (globalPoint));
}

// public
bool kpSelection::pointIsInTextArea (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kdError () << "kpSelection::pointIsInTextArea() not a text selection" << endl;
        return false;
    }

    return textAreaRect ().contains (globalPoint);
}

// public
int kpSelection::pointOnResizeHandle (const QPoint &globalPoint) const
{
    // (even if this is not a text selection - be consistent after all)
    const int atomicLength = textBorderSize ();

    // Heuristic:
    //
    // For text selections:
    // Non-corner handles are of size <atomicLength>.
    // Corner handles have height and width both <atomicLength>.
    //
    // For other selections,


    //if (m_rect.width () > atomicLength
    // TODO: wrong for small selections - probably want to move, not resize/scale
    // TODO: give more than a 1 pixel leeway

    if (globalPoint == boundingRect ().topLeft ())
    {
        return Top | Left;
    }
    else if (globalPoint == boundingRect ().topRight ())
    {
        return Top | Right;
    }
    else if (globalPoint == boundingRect ().bottomLeft ())
    {
        return Bottom | Left;
    }
    else if (globalPoint == boundingRect ().bottomRight ())
    {
        return Bottom | Right;
    }
    else if (globalPoint.x () == boundingRect ().left () &&
             globalPoint.y () == (boundingRect ().top () + boundingRect ().bottom ()) / 2)
    {
        return Left;
    }
    else if (globalPoint.x () == boundingRect ().right () &&
             globalPoint.y () == (boundingRect ().top () + boundingRect ().bottom ()) / 2)
    {
        return Right;
    }
    else if (globalPoint.y () == boundingRect ().top () &&
             globalPoint.x () == (boundingRect ().left () + boundingRect ().right ()) / 2)
    {
        return Top;
    }
    else if (globalPoint.y () == boundingRect ().bottom () &&
             globalPoint.x () == (boundingRect ().left () + boundingRect ().right ()) / 2)
    {
        return Bottom;
    }
    else
    {
        return 0;
    }
}

// public
void kpSelection::textResize (int width, int height)
{
    if (!isText ())
    {
        kdError () << "kpSelection::textResize() not a text selection" << endl;
        return;
    }

    QRect oldRect = m_rect;

    m_rect = QRect (oldRect.x (), oldRect.y (), width, height);

    calculatePoints ();
    calculateTextPixmap ();

    emit changed (m_rect.unite (oldRect));
}


// public static
int kpSelection::minimumWidthForTextStyle (const kpTextStyle &)
{
    return (kpSelection::textBorderSize () * 2 + 1);
}

// public static
int kpSelection::minimumHeightForTextStyle (const kpTextStyle &)
{
    return (kpSelection::textBorderSize () * 2 + 1);
}

// public static
QSize kpSelection::minimumSizeForTextStyle (const kpTextStyle &textStyle)
{
    return QSize (minimumWidthForTextStyle (textStyle),
                  minimumHeightForTextStyle (textStyle));
}


// public static
int kpSelection::preferredMinimumWidthForTextStyle (const kpTextStyle &textStyle)
{
    const int about15CharsWidth =
        textStyle.fontMetrics ().width (
            QString::fromLatin1 ("1234567890abcde"));

    const int preferredMinWidth =
        QMAX (150,
              textBorderSize () * 2 + about15CharsWidth);

    return QMAX (minimumWidthForTextStyle (textStyle),
                 QMIN (400, preferredMinWidth));
}

// public static
int kpSelection::preferredMinimumHeightForTextStyle (const kpTextStyle &textStyle)
{
    const int preferredMinHeight =
        textBorderSize () * 2 + textStyle.fontMetrics ().height ();

    return QMAX (minimumHeightForTextStyle (textStyle),
                 QMIN (150, preferredMinHeight));
}

// public static
QSize kpSelection::preferredMinimumSizeForTextStyle (const kpTextStyle &textStyle)
{
    return QSize (preferredMinimumWidthForTextStyle (textStyle),
                  preferredMinimumHeightForTextStyle (textStyle));
}


// public
int kpSelection::textRowForPoint (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kdError () << "kpSelection::textRowForPoint() not a text selection" << endl;
        return -1;
    }

    if (!pointIsInTextArea (globalPoint))
        return -1;

    const QFontMetrics fontMetrics (m_textStyle.fontMetrics ());

    int row = (globalPoint.y () - textAreaRect ().y ()) /
               (fontMetrics.height () + fontMetrics.leading ());
    if (row >= (int) m_textLines.size ())
        row = m_textLines.size () - 1;

    return row;
}

// public
int kpSelection::textColForPoint (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kdError () << "kpSelection::textColForPoint() not a text selection" << endl;
        return -1;
    }

    int row = textRowForPoint (globalPoint);
    if (row < 0 || row >= (int) m_textLines.size ())
        return -1;

    const int localX = globalPoint.x () - textAreaRect ().x ();

    const QFontMetrics fontMetrics (m_textStyle.fontMetrics ());

    // OPT: binary search or guess location then move
    for (int col = (int) m_textLines [row].length (); col >= 0; col--)
    {
        if (localX >= fontMetrics.width (m_textLines [row], col))
            return col;
    }

    return 0;
}

// public
QPoint kpSelection::pointForTextRowCol (int row, int col)
{
    if (!isText ())
    {
        kdError () << "kpSelection::pointForTextRowCol() not a text selection" << endl;
        return KP_INVALID_POINT;
    }

    if (row < 0 || row >= (int) m_textLines.size () ||
        col < 0 || col > (int) m_textLines [row].length ())
    {
    #if DEBUG_KP_SELECTION && 1
        kdDebug () << "kpSelection::pointForTextRowCol("
                   << row << ","
                   << col << ") out of range"
                   << " textLines='"
                   << text ()
                   << "'"
                   << endl;
    #endif
        return KP_INVALID_POINT;
    }

    const QFontMetrics fontMetrics (m_textStyle.fontMetrics ());

    const int x = fontMetrics.width (m_textLines [row], col);
    const int y = row * fontMetrics.height () +
                  (row >= 1 ? row * fontMetrics.leading () : 0);

    return textAreaRect ().topLeft () + QPoint (x, y);
}

// public
kpTextStyle kpSelection::textStyle () const
{
    if (!isText ())
    {
        kdError () << "kpSelection::textStyle() not a text selection" << endl;
    }

    return m_textStyle;
}

// public
void kpSelection::setTextStyle (const kpTextStyle &textStyle_)
{
    if (!isText ())
    {
        kdError () << "kpSelection::setTextStyle() not a text selection" << endl;
        return;
    }

    m_textStyle = textStyle_;
    calculateTextPixmap ();
    emit changed (boundingRect ());
}

// public
QPixmap kpSelection::opaquePixmap (bool evenIfText) const
{
    QPixmap *p = pixmap (evenIfText);
    if (p)
    {
        return *p;
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

    if (m_type == Text)
    {
    #if DEBUG_KP_SELECTION
        kdDebug () << "\ttext - no need for transparency mask" << endl;
    #endif
        m_transparencyMask.resize (0, 0);
        return;
    }

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
QPixmap kpSelection::transparentPixmap (bool evenIfText) const
{
    QPixmap pixmap = opaquePixmap (evenIfText);

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

