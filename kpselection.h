
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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
#include <qpoint.h>
#include <qpointarray.h>
#include <qvaluevector.h>
#include <qrect.h>
#include <qstring.h>

#include <kpcolor.h>
#include <kppixmapfx.h>
#include <kpselectiontransparency.h>
#include <kptextstyle.h>


class QSize;


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
        Points,
        Text
    };

    // (for any)
    kpSelection (const kpSelectionTransparency &transparency = kpSelectionTransparency ());

    // (for Rectangle & Ellipse)
    kpSelection (Type type, const QRect &rect, const QPixmap &pixmap = QPixmap (),
                 const kpSelectionTransparency &transparency = kpSelectionTransparency ());
    kpSelection (Type type, const QRect &rect, const kpSelectionTransparency &transparency);

    // (for Text)
    kpSelection (const QRect &rect, const QValueVector <QString> &textLines_, const kpTextStyle &textStyle_);

    // (for Points)
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
    bool isRectangular () const;
    bool isText () const;
    // returns either i18n ("Selection") or i18n ("Text")
    QString name () const;

    int size () const;

    QBitmap maskForOwnType (bool nullForRectangular = false) const;

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


    // (for non-rectangular selections, may return false even if
    //  kpView::onSelectionResizeHandle())
    bool contains (const QPoint &point) const;
    bool contains (int x, int y);


    // (Avoid using for text selections since text selection may
    //  require a background for antialiasing purposes - use paint()
    //  instead, else no antialising)
    QPixmap *pixmap () const;
    void setPixmap (const QPixmap &pixmap);

    bool usesBackgroundPixmapToPaint () const;

private:
    void paintOpaqueText (QPixmap *destPixmap, const QRect &docRect) const;
    QPixmap transparentForegroundTextPixmap () const;

public:
    // (<docRect> is the document rectangle that <*destPixmap> represents)
    void paint (QPixmap *destPixmap, const QRect &docRect) const;

private:
    void calculateTextPixmap ();

public:
    static QString textForTextLines (const QValueVector <QString> &textLines_);
    QString text () const;  // textLines() as one long string
    QValueVector <QString> textLines () const;
    void setTextLines (const QValueVector <QString> &textLines_);

    static int textBorderSize ();
    QRect textAreaRect () const;
    bool pointIsInTextBorderArea (const QPoint &globalPoint) const;
    bool pointIsInTextArea (const QPoint &globalPoint) const;


    void textResize (int width, int height);

    // TODO: Enforce in kpSelection, not just in kpToolSelection & when pasting
    //       (in kpMainWindow).
    //       Be more robust when external enforcement fails.
    static int minimumWidthForTextStyle (const kpTextStyle &);
    static int minimumHeightForTextStyle (const kpTextStyle &);
    static QSize minimumSizeForTextStyle (const kpTextStyle &);

    static int preferredMinimumWidthForTextStyle (const kpTextStyle &textStyle);
    static int preferredMinimumHeightForTextStyle (const kpTextStyle &textStyle);
    static QSize preferredMinimumSizeForTextStyle (const kpTextStyle &textStyle);

    int minimumWidth () const;
    int minimumHeight () const;
    QSize minimumSize () const;

    int textRowForPoint (const QPoint &globalPoint) const;
    int textColForPoint (const QPoint &globalPoint) const;
    QPoint pointForTextRowCol (int row, int col);

    kpTextStyle textStyle () const;
    void setTextStyle (const kpTextStyle &textStyle);

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

    QValueVector <QString> m_textLines;
    kpTextStyle m_textStyle;

    kpSelectionTransparency m_transparency;
    QBitmap m_transparencyMask;

private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpSelectionPrivate *d;
};

#endif  // __kpselection_h__
