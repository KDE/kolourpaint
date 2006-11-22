
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#ifndef kpToolPolygonalBase_H
#define kpToolPolygonalBase_H


#include <qbrush.h>
#include <qpen.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>

#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpimage.h>
#include <kptool.h>
#include <kptoolwidgetfillstyle.h>


class QMouseEvent;
class QPen;
class QPoint;
class QPolygon;
class QRect;
class QString;

class kpView;
class kpDocument;
class kpMainWindow;

class kpToolWidgetFillStyle;
class kpToolWidgetLineWidth;
class kpViewManager;


struct kpToolPolygonalBasePrivate;

//
// This tool base class is for shapes that contain at least an initial line that
// is dragged out (i.e. at least 2 control points).
//
// The tool can also choose to allow an additional point to be added for each
// additional drag or click.
//
// To specify whichever behavior, subclasses must implement endDraw() from
// kpTool and use points():
//
// 1. If the shape is incomplete, call setUserMessage() with a message
//    telling the user what can be done next.
// 2. If the shape is complete, call endShape().  See also MaxPoints.
//
// If additional points are supported by the user's implementation of endDraw(),
// beginDraw() will enforce the following behavior:
//
//     Clicking the mouse button not used for the initial line drag will
//     end the shape.
//
// This behavior cannot be altered by a subclass.
//
class kpToolPolygonalBase : public kpTool
{
Q_OBJECT

public:
    // TODO: awful - needs inheritance, all subclasses and I need to derive from
    //       a class kpToolPolygonalBase.
    enum Mode
    {
        Polygon, Polyline, Line, Curve
    };

    kpToolPolygonalBase (Mode mode, const QString &text, const QString &description,
                   int key,
                   kpMainWindow *mainWindow, const QString &name);
    kpToolPolygonalBase (kpMainWindow *mainWindow);
    virtual ~kpToolPolygonalBase ();

    void setMode (Mode mode);

    virtual bool careAboutModifierState () const { return true; }

protected:
    // The maximum number of points() we should allow (mainly, to ensure
    // good performance).  Enforced by implementors of endShape().
    static const int kpToolPolygonalBase::MaxPoints = 50;

    virtual QString haventBegunShapeUserMessage () const = 0;

public:
    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
private:
    void applyModifiers ();
protected:
    QPolygon *points () const;
public:
    virtual void draw (const QPoint &, const QPoint &, const QRect &);
private:
    kpColor drawingForegroundColor () const;
protected:
    virtual kpColor drawingBackgroundColor () const;
protected slots:
    void updateShape ();
public:
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endShape (const QPoint & = QPoint (), const QRect & = QRect ());

    virtual bool hasBegunShape () const;

protected slots:
    virtual void slotForegroundColorChanged (const kpColor &);
    virtual void slotBackgroundColorChanged (const kpColor &);

private:
    kpToolPolygonalBasePrivate * const d;

};


kpImage kpToolPolygonalBaseImage (const QPixmap &oldImage,
        const QPolygon &points, const QRect &rect,
        const kpColor &foregroundColor, int penWidth,
        kpColor backgroundColor,
        enum kpToolPolygonalBase::Mode mode, bool final = true);


#endif  // kpToolPolygonalBase_H
