
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

#include <qobject.h>
#include <qpointarray.h>
#include <qrect.h>


class QPixmap;


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

    kpSelection ();
    kpSelection (Type type, const QRect &rect, const QPixmap &pixmap = QPixmap ());
    kpSelection (const QPointArray &points, const QPixmap &pixmap = QPixmap ());
    kpSelection (const kpSelection &rhs);
    kpSelection &operator= (const kpSelection &rhs);
    ~kpSelection ();

    Type type () const;

    // synonyms
    QPoint topLeft () const;
    QPoint point () const;

    void moveBy (int dx, int dy);
    void moveTo (int dx, int dy);
    void moveTo (const QPoint &topLeftPoint);

    // synonyms
    // (only valid if type == Points)
    QPointArray points () const;
    QPointArray pointArray () const;

    QRect boundingRect () const;
    int width () const;
    int height () const;

    bool contains (const QPoint &point) const;
    bool contains (int x, int y);

    QPixmap *pixmap () const;
    void setPixmap (const QPixmap &pixmap);

signals:
    void changed (const QRect &docRect);

private:
    Type m_type;
    QRect m_rect;
    QPointArray m_points;
    QPixmap *m_pixmap;
};


#endif  // __kpselection_h__
