
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


#include <kpSelection.h>

#include <qfont.h>
#include <qimage.h>
#include <qmatrix.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qpolygon.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpColorSimilarityDialog.h>
#include <kpDefs.h>
#include <kpPixmapFX.h>
#include <kpTool.h>


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
                          const QList <QString> &textLines_,
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

kpSelection::kpSelection (const QPolygon &points, const QPixmap &pixmap,
                          const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (Points),
      m_rect (points.boundingRect ()),
      m_points (points)
{
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);

    setTransparency (transparency);
}

kpSelection::kpSelection (const QPolygon &points, const kpSelectionTransparency &transparency)
    : QObject (),
      m_type (Points),
      m_rect (points.boundingRect ()),
      m_points (points),
      m_pixmap (0)
{
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
}

kpSelection &kpSelection::operator= (const kpSelection &rhs)
{
    if (this == &rhs)
        return *this;

    m_type = rhs.m_type;
    m_rect = rhs.m_rect;
    m_points = rhs.m_points;

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
    kDebug () << "kpSelection::operator<<(sel: rect=" << selection.boundingRect ()
               << " pixmap rect=" << (selection.pixmap () ? selection.pixmap ()->rect () : QRect ())
               << endl;
#endif
    stream << int (selection.m_type);
    stream << selection.m_rect;
    stream << selection.m_points;
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\twrote type=" << int (selection.m_type) << " rect=" << selection.m_rect
               << " and points" << endl;
#endif

    // TODO: need for text?
    //       For now we just use QTextDrag for Text Selections so this point is mute.
    if (selection.m_pixmap)
    {
        const QImage image = kpPixmapFX::convertToImage (*selection.m_pixmap);
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\twrote image rect=" << image.rect () << endl;
    #endif
        stream << image;
    }
    else
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\twrote no image because no pixmap" << endl;
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
    kDebug () << "kpSelection::readFromStream()" << endl;
#endif
    int typeAsInt;
    stream >> typeAsInt;
    m_type = kpSelection::Type (typeAsInt);
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\ttype=" << typeAsInt << endl;
#endif

    stream >> m_rect;
    stream >> m_points;
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\trect=" << m_rect << endl;
    //kDebug () << "\tpoints=" << m_points << endl;
#endif

    QImage image;
    stream >> image;
    delete m_pixmap;
    if (!image.isNull ())
        m_pixmap = new QPixmap (kpPixmapFX::convertToPixmap (image, false/*no dither*/, wali));
    else
        m_pixmap = 0;
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\timage: w=" << image.width () << " h=" << image.height ()
               << " depth=" << image.depth () << endl;
    if (m_pixmap)
    {
        kDebug () << "\tpixmap: w=" << m_pixmap->width () << " h=" << m_pixmap->height ()
                   << endl;
    }
    else
    {
        kDebug () << "\tpixmap: none" << endl;
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
        QPainterPath path;
        path.addEllipse (m_rect);

        const QList <QPolygonF> polygons = path.toSubpathPolygons ();
        Q_ASSERT (polygons.size () == 1);
        
        const QPolygonF firstPolygonF = polygons.first ();
        m_points = firstPolygonF.toPolygon ();

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

    kError () << "kpSelection::calculatePoints() with unknown type" << endl;
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
        kError () << "kpSelection::maskForOwnType() boundingRect invalid" << endl;
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
        QPolygon points = m_points;
        points.translate (-m_rect.x (), -m_rect.y ());

        painter.drawPolygon (points, Qt::OddEvenFill);
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
    kDebug () << "kpSelection::moveBy(" << dx << "," << dy << ")" << endl;
#endif

    if (dx == 0 && dy == 0)
        return;

    QRect oldRect = boundingRect ();

#if DEBUG_KP_SELECTION && 1
    kDebug () << "\toldRect=" << oldRect << endl;
#endif

    m_rect.translate (dx, dy);
    m_points.translate (dx, dy);
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\tnewRect=" << m_rect << endl;
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
    kDebug () << "kpSelection::moveTo(" << topLeftPoint << ")" << endl;
#endif
    QRect oldBoundingRect = boundingRect ();
#if DEBUG_KP_SELECTION && 1
    kDebug () << "\toldBoundingRect=" << oldBoundingRect << endl;
#endif
    if (topLeftPoint == oldBoundingRect.topLeft ())
        return;

    QPoint delta (topLeftPoint - oldBoundingRect.topLeft ());
    moveBy (delta.x (), delta.y ());
}


// public
QPolygon kpSelection::points () const
{
    return m_points;
}

// public
QPolygon kpSelection::pointArray () const
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

// protected
QRegion kpSelection::maskRegion () const
{
    // OPT: QRegion is probably incredibly slow - cache
    // We can't use the m_pixmap (if avail) and get the transparency of
    // the pixel at that point as it may be transparent but still within the
    // border
    switch (m_type)
    {
    case kpSelection::Rectangle:
    case kpSelection::Text:
        return QRegion (m_rect, QRegion::Rectangle);
        
    case kpSelection::Ellipse:
        return QRegion (m_rect, QRegion::Ellipse);
        
    case kpSelection::Points:
        // TODO: make this always include the border
        //       (draw up a rect sel in this mode to see what I mean)
        return QRegion (m_points, Qt::OddEvenFill);
        
    default:
        Q_ASSERT (!"unknown type");
        return QRegion ();
    }
}

// public
bool kpSelection::contains (const QPoint &point) const
{
    QRect rect = boundingRect ();

#if DEBUG_KP_SELECTION && 1
    kDebug () << "kpSelection::contains(" << point
               << ") rect==" << rect
               << " #points=" << m_points.size ()
               << endl;
#endif

    if (!rect.contains (point))
        return false;

    switch (m_type)
    {
    case kpSelection::Rectangle:
    case kpSelection::Text:
        return true;
    case kpSelection::Ellipse:
    case kpSelection::Points:
        return maskRegion ().contains (point);
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
kpImage kpSelection::givenImageMaskedByShape (const kpImage &image) const
{
#if DEBUG_KP_SELECTION || 1
    kDebug () << "kpSelection::givenImageMaskedByShape() boundingRect="
              << boundingRect () << endl;
#endif
    Q_ASSERT (image.width () == width () && image.height () == height ());

    if (isRectangular ())
        return image;
        

    const QRegion mRegion = maskRegion ().translated (-topLeft ());

    kpImage retImage (width (), height ());
    kpPixmapFX::ensureTransparentAt (&retImage, retImage.rect ());

    // OPT: Hopelessly inefficent due to function call overhead.
    //      kpPixmapFX should have a function that does this.
    foreach (QRect r, mRegion.rects ())
    {
    #if DEBUG_KP_SELECTION || 1
        kDebug () << "\tcopy rect=" << r << endl;
    #endif
        // OPT: Hopeless inefficient.  If kpPixmapFX::setPixmapAt() was
        //      more flexible, we wouldn't need to call getPixmapAt().
        const kpImage srcPixmap = kpPixmapFX::getPixmapAt (image, r);
        
        kpPixmapFX::setPixmapAt (&retImage, r.topLeft (), srcPixmap);
    }

    return retImage;
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
    // TODO: If isText(), setting pixmap() to 0 is unexpected (implies
    //       it's a border, not a text box) but saves memory when using
    //       that kpSelection::setPixmap (QPixmap ()) hack.
    m_pixmap = pixmap.isNull () ? 0 : new QPixmap (pixmap);

    QRect oldRect = boundingRect ();
    emit changed (oldRect);

    if (m_pixmap)
    {
        const bool changedSize = (m_pixmap->width () != oldRect.width () ||
                                  m_pixmap->height () != oldRect.height ());
        const bool changedFromText = (m_type == Text);
        if (changedSize || changedFromText)
        {
            if (changedSize)
            {
                kError () << "kpSelection::setPixmap() changes the size of the selection!"
                        << "   old:"
                        << " w=" << oldRect.width ()
                        << " h=" << oldRect.height ()
                        << "   new:"
                        << " w=" << m_pixmap->width ()
                        << " h=" << m_pixmap->height ()
                        << endl;
            }

            if (changedFromText)
            {
                kError () << "kpSelection::setPixmap() changed from text" << endl;
            }

            m_type = kpSelection::Rectangle;
            m_rect = QRect (m_rect.x (), m_rect.y (),
                            m_pixmap->width (), m_pixmap->height ());
            calculatePoints ();

            m_textLines = QList <QString> ();

            emit changed (boundingRect ());
        }
    }

    calculateTransparencyMask ();
}

void kpSelection::fill (const kpColor &color)
{
    QPixmap newPixmap (width (), height ());
    kpPixmapFX::fill (&newPixmap, color);

    // TODO: maybe disable Image/Clear menu item if transparent colour
    if (color.isOpaque ())
        newPixmap.setMask (maskForOwnType ());

    setPixmap (newPixmap);
}

// public
bool kpSelection::usesBackgroundPixmapToPaint () const
{
    // Opaque text with transparent background needs to antialias with
    // doc pixmap below it.
    return (isText () &&
            m_textStyle.foregroundColor ().isOpaque () &&
            m_textStyle.effectiveBackgroundColor ().isTransparent ());
}

static int mostContrastingValue (int val)
{
    if (val <= 127)
        return 255;
    else
        return 0;
}

static QRgb mostContrastingRGB (QRgb val)
{
    return qRgba (mostContrastingValue (qRed (val)),
                  mostContrastingValue (qGreen (val)),
                  mostContrastingValue (qBlue (val)),
                  mostContrastingValue (qAlpha (val)));
}

// private
static void drawTextLines (QPainter *painter, QPainter *maskPainter,
                           const QRect &rect,
                           const QList <QString> &textLines)
{
    if (!painter->clipRegion ().isEmpty () || !maskPainter->clipRegion ().isEmpty ())
    {
        // TODO: fix esp. before making method public
        kError () << "kpselection.cpp:drawTextLines() can't deal with existing painter clip regions" << endl;
        return;
    }


#define PAINTER_CALL(cmd)          \
{                                  \
    if (painter->isActive ())      \
        painter->cmd;              \
                                   \
    if (maskPainter->isActive ())  \
        maskPainter->cmd;          \
}


    // Can't do this because the line heights become
    // >QFontMetrics::height() if you type Chinese characters (!) and then
    // the cursor gets out of sync.
    // PAINTER_CALL (drawText (rect, 0/*flags*/, text ()));

    
#if 0
    const QFontMetrics fontMetrics (painter->fontMetrics ());

    kDebug () << "height=" << fontMetrics.height ()
               << " leading=" << fontMetrics.leading ()
               << " ascent=" << fontMetrics.ascent ()
               << " descent=" << fontMetrics.descent ()
               << " lineSpacing=" << fontMetrics.lineSpacing ()
               << endl;
#endif


 // COMPAT: CoordPainter   PAINTER_CALL (setClipRect (rect, QPainter::CoordPainter/*transform*/));

    int baseLine = rect.y () + painter->fontMetrics ().ascent ();
    for (QList <QString>::const_iterator it = textLines.begin ();
         it != textLines.end ();
         it++)
    {
        PAINTER_CALL (drawText (rect.x (), baseLine, *it));
        baseLine += painter->fontMetrics ().lineSpacing ();
    }


#undef PAINTER_CALL
}

// private
void kpSelection::paintOpaqueText (QPixmap *destPixmap, const QRect &docRect) const
{
    if (!isText () || !m_textStyle.foregroundColor ().isOpaque ())
        return;


    const QRect modifyingRect = docRect.intersect (boundingRect ());
    if (modifyingRect.isEmpty ())
        return;


    QBitmap destPixmapMask;
    QPainter destPixmapPainter, destPixmapMaskPainter;

    if (!destPixmap->mask ().isNull ())
    {
        if (m_textStyle.effectiveBackgroundColor ().isTransparent ())
        {
            QRect modifyingRectRelPixmap = modifyingRect;
            modifyingRectRelPixmap.translate (-docRect.x (), -docRect.y ());

            // Set the RGB of transparent pixels to foreground colour to avoid
            // anti-aliasing the foreground coloured text with undefined RGBs.
            kpPixmapFX::setPixmapAt (destPixmap,
                modifyingRectRelPixmap,
                kpPixmapFX::pixmapWithDefinedTransparentPixels (
                    kpPixmapFX::getPixmapAt (*destPixmap, modifyingRectRelPixmap),
                    m_textStyle.foregroundColor ().toQColor ()));
        }

        destPixmapMask = destPixmap->mask ();
        destPixmapMaskPainter.begin (&destPixmapMask);
        destPixmapMaskPainter.translate (-docRect.x (), -docRect.y ());
        destPixmapMaskPainter.setPen (Qt::color1/*opaque*/);
        destPixmapMaskPainter.setFont (m_textStyle.font ());
    }

    destPixmapPainter.begin (destPixmap);
    destPixmapPainter.translate (-docRect.x (), -docRect.y ());
    destPixmapPainter.setPen (m_textStyle.foregroundColor ().toQColor ());
    destPixmapPainter.setFont (m_textStyle.font ());


    if (m_textStyle.effectiveBackgroundColor ().isOpaque ())
    {
        destPixmapPainter.fillRect (
            boundingRect (),
            m_textStyle.effectiveBackgroundColor ().toQColor ());

        if (destPixmapMaskPainter.isActive ())
        {
            destPixmapMaskPainter.fillRect (
                boundingRect (),
                Qt::color1/*opaque*/);
        }
    }


    ::drawTextLines (&destPixmapPainter, &destPixmapMaskPainter,
                      textAreaRect (),
                      textLines ());


    if (destPixmapPainter.isActive ())
        destPixmapPainter.end ();

    if (destPixmapMaskPainter.isActive ())
        destPixmapMaskPainter.end ();


    if (!destPixmapMask.isNull ())
        destPixmap->setMask (destPixmapMask);
}

// private
QPixmap kpSelection::transparentForegroundTextPixmap () const
{
    if (!isText () || !m_textStyle.foregroundColor ().isTransparent ())
        return QPixmap ();


    QPixmap pixmap (m_rect.width (), m_rect.height ());
    QBitmap pixmapMask (m_rect.width (), m_rect.height ());


    // Iron out stupid case first
    if (m_textStyle.effectiveBackgroundColor ().isTransparent ())
    {
        pixmapMask.fill (Qt::color0/*transparent*/);
        pixmap.setMask (pixmapMask);
        return pixmap;
    }


    // -- Foreground transparent, background opaque --


    QFont font = m_textStyle.font ();
    // TODO: why doesn't "font.setStyleStrategy (QFont::NoAntialias);"
    //       let us avoid the hack below?


    QPainter pixmapPainter, pixmapMaskPainter;

    pixmap.fill (m_textStyle.effectiveBackgroundColor ().toQColor ());
    pixmapPainter.begin (&pixmap);
    // HACK: Transparent foreground colour + antialiased fonts don't
    // work - they don't seem to be able to draw in
    // Qt::color0/*transparent*/ (but Qt::color1 seems Ok).
    // So we draw in a contrasting color to the background so that
    // we can identify the transparent pixels for manually creating
    // the mask.
    pixmapPainter.setPen (
        QColor (mostContrastingRGB (m_textStyle.effectiveBackgroundColor ().toQRgb () & RGB_MASK)));
    pixmapPainter.setFont (font);


    pixmapMask.fill (Qt::color1/*opaque*/);
    pixmapMaskPainter.begin (&pixmapMask);
    pixmapMaskPainter.setPen (Qt::color0/*transparent*/);
    pixmapMaskPainter.setFont (font);


    QRect rect (textAreaRect ());
    rect.translate (-m_rect.x (), -m_rect.y ());
    ::drawTextLines (&pixmapPainter, &pixmapMaskPainter,
                     rect,
                     textLines ());


    if (pixmapPainter.isActive ())
        pixmapPainter.end ();

    if (pixmapMaskPainter.isActive ())
        pixmapMaskPainter.end ();


#if DEBUG_KP_SELECTION
    kDebug () << "\tinvoking foreground transparency hack" << endl;
#endif
    QImage image = kpPixmapFX::convertToImage (pixmap);
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


    if (!pixmapMask.isNull ())
        pixmap.setMask (pixmapMask);


    return pixmap;
}

// public
void kpSelection::paint (QPixmap *destPixmap, const QRect &docRect) const
{
    if (!isText ())
    {
        if (pixmap () && !pixmap ()->isNull ())
        {
            kpPixmapFX::paintPixmapAt (destPixmap,
                                       topLeft () - docRect.topLeft (),
                                       transparentPixmap ());
        }
    }
    else
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "kpSelection::paint() textStyle: fcol="
                << (int *) m_textStyle.foregroundColor ().toQRgb ()
                << " bcol="
                << (int *) m_textStyle.effectiveBackgroundColor ().toQRgb ()
                << endl;
    #endif

        if (m_textStyle.foregroundColor ().isOpaque ())
        {
            // (may have to antialias with background so don't use m_pixmap)
            paintOpaqueText (destPixmap, docRect);
        }
        else
        {
            if (!m_pixmap)
            {
                kError () << "kpSelection::paint() without m_pixmap?" << endl;
                return;
            }

            // (transparent foreground slow to render, no antialiasing
            //  so use m_pixmap cache)
            kpPixmapFX::paintPixmapAt (destPixmap,
                                       topLeft () - docRect.topLeft (),
                                       *m_pixmap);
        }
    }
}

// public
void kpSelection::paintBorder (QPixmap *destPixmap, const QRect &docRect,
        bool selectionFinished) const
{
#if DEBUG_KP_SELECTION && 1
    kDebug () << "kpSelection::paintBorder() boundingRect=" << boundingRect () << endl;
#endif

#if 0
    QPainter destPixmapPainter (destPixmap);
    // COMPAT: destPixmapPainter.setRasterOp (Qt::XorROP);
    destPixmapPainter.setPen (QPen (Qt::white, 1, Qt::DotLine));

    destPixmapPainter.setBackgroundMode (Qt::OpaqueMode);
    destPixmapPainter.setBackground (Qt::blue);

    QBitmap maskBitmap;
    QPainter maskBitmapPainter;
    if (!destPixmap->mask ().isNull ())
    {
        maskBitmap = destPixmap->mask ();
        maskBitmapPainter.begin (&maskBitmap);
        maskBitmapPainter.setPen (Qt::color1/*opaque*/);
    }
#endif


#define PAINTER_CMD(cmd)                 \
{                                        \
    destPixmapPainter . cmd;             \
    if (maskBitmapPainter.isActive ())   \
        maskBitmapPainter . cmd;         \
}

#if DEBUG_KP_SELECTION && 1
    kDebug () << "\tsel boundingRect="
            << boundingRect
            << endl;
#endif

#if 0
    if (boundingRect ().topLeft () != boundingRect ().bottomRight ())
    {
#endif
        switch (type ())
        {
        case kpSelection::Rectangle:
        case kpSelection::Text:
        #if DEBUG_KP_SELECTION && 1
            kDebug () << "\tselection border = rectangle" << endl;
            kDebug () << "\t\tx=" << boundingRect ().x () - docRect.x ()
                    << " y=" << boundingRect ().y () - docRect.y ()
                    << " w=" << boundingRect ().width ()
                    << " h=" << boundingRect ().height ()
                    << endl;
        #endif
            kpPixmapFX::drawStippledXORRect (destPixmap,
                boundingRect ().x () - docRect.x (),
                boundingRect ().y () - docRect.y (),
                boundingRect ().width (),
                boundingRect ().height (),
                kpColor::White, kpColor::Blue,  // Stippled XOR colors
                kpColor::Blue, kpColor::Yellow);  // Hint colors if XOR not supported
            break;

        case kpSelection::Ellipse:
        #if 0
        #if DEBUG_KP_SELECTION && 1
            kDebug () << "\tselection border = ellipse" << endl;
        #endif
            PAINTER_CMD (drawEllipse (boundingRect ().x () - docRect.x (),
                                    boundingRect ().y () - docRect.y (),
                                    boundingRect ().width (),
                                    boundingRect ().height ()));
        #endif
            break;

        case kpSelection::Points:
        {
        #if 0
        #if DEBUG_KP_SELECTION
            kDebug () << "\tselection border = freeForm" << endl;
        #endif
            QPolygon pointsTranslated = points ();
            pointsTranslated.translate (-docRect.x (), -docRect.y ());
            if (selectionFinished)
            {
                PAINTER_CMD (drawPolygon (pointsTranslated));
            }
            else
            {
                PAINTER_CMD (drawPolyline (pointsTranslated));
            }
        #endif

            break;
        }

        default:
            kError () << "kpView::paintEventDrawSelection() unknown sel border type" << endl;
            break;
        }


        if (selectionFinished &&
            (type () == kpSelection::Ellipse ||
            type () == kpSelection::Points))
        {
            kpPixmapFX::drawNOTRect (destPixmap,
                boundingRect ().x () - docRect.x (),
                boundingRect ().y () - docRect.y (),
                boundingRect ().width (),
                boundingRect ().height (),
                kpColor::White/*"Raster NOT" color*/,
                kpColor::LightGray/*hint color if "Raster NOT" not supported*/);
        }
#if 0
    }
    else
    {
        // SYNC: Work around Qt bug: can't draw 1x1 rectangle
        PAINTER_CMD (drawPoint (boundingRect ().topLeft () - docRect.topLeft ()));
    }
#endif

#undef PAINTER_CMD

#if 0
    destPixmapPainter.end ();
    if (maskBitmapPainter.isActive ())
        maskBitmapPainter.end ();

    destPixmap->setMask (maskBitmap);
#endif
}

// private
void kpSelection::calculateTextPixmap ()
{
    if (!isText ())
    {
        kError () << "kpSelection::calculateTextPixmap() not text sel"
                   << endl;
        return;
    }

    delete m_pixmap;

    if (m_textStyle.foregroundColor().isOpaque ())
    {
        m_pixmap = new QPixmap (m_rect.width (), m_rect.height ());

        if (usesBackgroundPixmapToPaint ())
            kpPixmapFX::fill (m_pixmap, kpColor::Transparent);

        paintOpaqueText (m_pixmap, m_rect);
    }
    else
    {
        m_pixmap = new QPixmap (transparentForegroundTextPixmap ());
    }
}


// public static
QString kpSelection::textForTextLines (const QList <QString> &textLines_)
{
    if (textLines_.isEmpty ())
        return QString::null;

    QString bigString = textLines_ [0];

    for (QList <QString>::const_iterator it = textLines_.begin () + 1;
         it != textLines_.end ();
         it++)
    {
        bigString += QLatin1String ("\n");
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
QList <QString> kpSelection::textLines () const
{
    if (!isText ())
    {
        kError () << "kpSelection::textLines() not a text selection" << endl;
        return QList <QString> ();
    }

    return m_textLines;
}

// public
void kpSelection::setTextLines (const QList <QString> &textLines_)
{
    if (!isText ())
    {
        kError () << "kpSelection::setTextLines() not a text selection" << endl;
        return;
    }

    m_textLines = textLines_;
    if (m_textLines.isEmpty ())
    {
        kError () << "kpSelection::setTextLines() passed no lines" << endl;
        m_textLines.push_back (QString::null);
    }
    calculateTextPixmap ();
    emit changed (boundingRect ());
}

// public static
int kpSelection::textBorderSize ()
{
    return 1;
}

// public
QRect kpSelection::textAreaRect () const
{
    if (!isText ())
    {
        kError () << "kpSelection::textAreaRect() not a text selection" << endl;
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
        kError () << "kpSelection::pointIsInTextBorderArea() not a text selection" << endl;
        return false;
    }

    return (m_rect.contains (globalPoint) && !pointIsInTextArea (globalPoint));
}

// public
bool kpSelection::pointIsInTextArea (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kError () << "kpSelection::pointIsInTextArea() not a text selection" << endl;
        return false;
    }

    return textAreaRect ().contains (globalPoint);
}


// public
void kpSelection::textResize (int width, int height)
{
    if (!isText ())
    {
        kError () << "kpSelection::textResize() not a text selection" << endl;
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
    return (kpSelection::textBorderSize () * 2 + 5);
}

// public static
int kpSelection::minimumHeightForTextStyle (const kpTextStyle &)
{
    return (kpSelection::textBorderSize () * 2 + 5);
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
            QLatin1String ("1234567890abcde"));

    const int preferredMinWidth =
        qMax (150,
              textBorderSize () * 2 + about15CharsWidth);

    return qMax (minimumWidthForTextStyle (textStyle),
                 qMin (250, preferredMinWidth));
}

// public static
int kpSelection::preferredMinimumHeightForTextStyle (const kpTextStyle &textStyle)
{
    const int preferredMinHeight =
        textBorderSize () * 2 + textStyle.fontMetrics ().height ();

    return qMax (minimumHeightForTextStyle (textStyle),
                 qMin (150, preferredMinHeight));
}

// public static
QSize kpSelection::preferredMinimumSizeForTextStyle (const kpTextStyle &textStyle)
{
    return QSize (preferredMinimumWidthForTextStyle (textStyle),
                  preferredMinimumHeightForTextStyle (textStyle));
}


// public
int kpSelection::minimumWidth () const
{
    if (isText ())
        return minimumWidthForTextStyle (textStyle ());
    else
        return 1;
}

// public
int kpSelection::minimumHeight () const
{
    if (isText ())
        return minimumHeightForTextStyle (textStyle ());
    else
        return 1;
}

// public
QSize kpSelection::minimumSize () const
{
    return QSize (minimumWidth (), minimumHeight ());
}


// public
int kpSelection::textRowForPoint (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kError () << "kpSelection::textRowForPoint() not a text selection" << endl;
        return -1;
    }

    if (!pointIsInTextArea (globalPoint))
        return -1;

    const QFontMetrics fontMetrics (m_textStyle.fontMetrics ());

    int row = (globalPoint.y () - textAreaRect ().y ()) /
               fontMetrics.lineSpacing ();
    if (row >= (int) m_textLines.size ())
        row = m_textLines.size () - 1;

    return row;
}

// public
int kpSelection::textColForPoint (const QPoint &globalPoint) const
{
    if (!isText ())
    {
        kError () << "kpSelection::textColForPoint() not a text selection" << endl;
        return -1;
    }

    int row = textRowForPoint (globalPoint);
    if (row < 0 || row >= (int) m_textLines.size ())
        return -1;

    const int localX = globalPoint.x () - textAreaRect ().x ();

    const QFontMetrics fontMetrics (m_textStyle.fontMetrics ());

    // (should be 0 but call just in case)
    int charLocalLeft = fontMetrics.width (m_textLines [row], 0);
    
    // OPT: binary search or guess location then move
    for (int col = 0; col < (int) m_textLines [row].length (); col++)
    {
        // OPT: fontMetrics::charWidth() might be faster
        const int nextCharLocalLeft = fontMetrics.width (m_textLines [row], col + 1);
        if (localX <= (charLocalLeft + nextCharLocalLeft) / 2)
            return col;
        
        charLocalLeft = nextCharLocalLeft;
    }

    return m_textLines [row].length ()/*past end of line*/;
}

// public
QPoint kpSelection::pointForTextRowCol (int row, int col)
{
    if (!isText ())
    {
        kError () << "kpSelection::pointForTextRowCol() not a text selection" << endl;
        return KP_INVALID_POINT;
    }

    if (row < 0 || row >= (int) m_textLines.size () ||
        col < 0 || col > (int) m_textLines [row].length ())
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "kpSelection::pointForTextRowCol("
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
        kError () << "kpSelection::textStyle() not a text selection" << endl;
    }

    return m_textStyle;
}

// public
void kpSelection::setTextStyle (const kpTextStyle &textStyle_)
{
    if (!isText ())
    {
        kError () << "kpSelection::setTextStyle() not a text selection" << endl;
        return;
    }

    m_textStyle = textStyle_;
    calculateTextPixmap ();
    emit changed (boundingRect ());
}

// public
QPixmap kpSelection::opaquePixmap () const
{
    QPixmap *p = pixmap ();
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
    kDebug () << "kpSelection::calculateTransparencyMask()" << endl;
#endif

    if (isText ())
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\ttext - no need for transparency mask" << endl;
    #endif
        m_transparencyMask = QBitmap ();
        return;
    }

    if (!m_pixmap)
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\tno pixmap - no need for transparency mask" << endl;
    #endif
        m_transparencyMask = QBitmap ();
        return;
    }

    if (m_transparency.isOpaque ())
    {
    #if DEBUG_KP_SELECTION
        kDebug () << "\topaque - no need for transparency mask" << endl;
    #endif
        m_transparencyMask = QBitmap ();
        return;
    }

    m_transparencyMask = QBitmap (m_pixmap->width (), m_pixmap->height ());

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
        kDebug () << "\tcolour useless - completely opaque" << endl;
    #endif
        m_transparencyMask = QBitmap ();
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
            kDebug () << "\tboth old and new pixmaps are null - nothing changed" << endl;
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
                        kDebug () << "\tdiffer at " << QPoint (x, y)
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

    const QMatrix matrix = kpPixmapFX::flipMatrix (oldRect.width (), oldRect.height (),
                                                    horiz, vert);
    m_points = matrix.map (m_points);

    m_points.translate (oldRect.x (), oldRect.y ());
}


// public
void kpSelection::flip (bool horiz, bool vert)
{
#if DEBUG_KP_SELECTION && 1
    kDebug () << "kpSelection::flip(horiz=" << horiz
               << ",vert=" << vert << ")" << endl;
#endif

    flipPoints (horiz, vert);


    if (m_pixmap)
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\thave pixmap - flipping that" << endl;
    #endif
        kpPixmapFX::flip (m_pixmap, horiz, vert);
    }

    if (!m_transparencyMask.isNull ())
    {
    #if DEBUG_KP_SELECTION && 1
        kDebug () << "\thave transparency mask - flipping that" << endl;
    #endif
        kpPixmapFX::flip (&m_transparencyMask, horiz, vert);
    }


    emit changed (boundingRect ());
}

#include <kpSelection.moc>
