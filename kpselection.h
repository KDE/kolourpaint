
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


#ifndef __kpselection_h__
#define __kpselection_h__

#include <qbitmap.h>
#include <qdatastream.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qvaluevector.h>
#include <qrect.h>

#include <kpcolor.h>
#include <kppixmapfx.h>
#include <kpselectiontransparency.h>
#include <kptooltext.h>


/*
 * Holds a selection - will also be used for the clipboard
 * so that we can retain the border.
 */
class kpSelection : public QObject
{
Q_OBJECT

public:
    enum Type
    {
        Rectangle,
        Ellipse,
        Points
    };

    kpSelection (const kpSelectionTransparency &transparency = kpSelectionTransparency ());
    kpSelection (Type type, const QRect &rect, const QPixmap &pixmap = QPixmap (),
                 const kpSelectionTransparency &transparency = kpSelectionTransparency ());
    kpSelection (Type type, const QRect &rect, const kpSelectionTransparency &transparency);
    kpSelection (const QPointArray &points, const QPixmap &pixmap = QPixmap (),
                 const kpSelectionTransparency &transparency = kpSelectionTransparency ());
    kpSelection (const QPointArray &points, const kpSelectionTransparency &transparency);
    kpSelection (const kpSelection &rhs);
    kpSelection &operator= (const kpSelection &rhs);
    friend QDataStream &operator<< (QDataStream &stream, const kpSelection &selection);
    friend QDataStream &operator>> (QDataStream &stream, kpSelection &selection);
    void readFromStream (QDataStream &stream,
                         const kpPixmapFX::WarnAboutLossInfo &wali =
                             kpPixmapFX::WarnAboutLossInfo ());
    ~kpSelection ();

private:
    void calculatePoints ();

public:

    Type type () const;

    // synonyms
    QPoint topLeft () const;
    QPoint point () const;

    int x () const;
    int y () const;

    void moveBy (int dx, int dy);
    void moveTo (int dx, int dy);
    void moveTo (const QPoint &topLeftPoint);

    // synonyms
    QPointArray points () const;
    QPointArray pointArray () const;

    QRect boundingRect () const;
    int width () const;
    int height () const;

    bool contains (const QPoint &point) const;
    bool contains (int x, int y);

    QPixmap *pixmap () const;
    void setPixmap (const QPixmap &pixmap);

    // TODO: ret val inconstent with pixmap()
    //       - fix when merge with kpTempPixmap
    QPixmap opaquePixmap () const;  // same as pixmap()

private:
    void calculateTransparencyMask ();

public:
    // Returns opaquePixmap() after applying kpSelectionTransparency
    QPixmap transparentPixmap () const;

    kpSelectionTransparency transparency () const;
    // Returns whether or not the selection changed due to setting the
    // transparency info.  If <checkTransparentPixmapChanged> is set,
    // it will try harder to return false (although the check is
    // expensive).
    bool setTransparency (const kpSelectionTransparency &transparency,
                          bool checkTransparentPixmapChanged = false);

private:
    void flipPoints (bool horiz, bool vert);

public:
    void flip (bool horiz, bool vert);

signals:
    void changed (const QRect &docRect);

private:
    // OPT: use implicit sharing

    Type m_type;
    QRect m_rect;
    QPointArray m_points;
    QPixmap *m_pixmap;

    // TODO: (unused / reserved)
    QValueVector <QString> m_textLines;
    kpTextStyle m_textStyle;

    kpSelectionTransparency m_transparency;
    bool BIC_HACK;
    QBitmap m_transparencyMask;

private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpSelectionPrivate *d;
};

#endif  // __kpselection_h__
