
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
// beginDraw() will ensure that points() contains 2 points on the initial line
// drag.  It will add an extra point for each additional point that is dragged.
//
// You may wish to reimplement drawingALine() if your shape does not consist of
// just connected lines e.g. while the Curve tool, on the initial drag (consisting
// of 2 points) creates a line, future drags do not create extra lines - they
// actually modify the Bezier control points.
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
    static const int MaxPoints = 50;

    virtual QString haventBegunShapeUserMessage () const = 0;

public:
    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();

protected:
    // Adjusts the current line (end points given by the last 2 points of points())
    // in response to keyboard modifiers:
    //
    // No modifiers: Does nothing
    // Shift       : Clamps the line to 45 degrees increments
    // Ctrl        : Clamps the line to 30 degrees increments
    // Alt         : [currently disabled] Makes the starting point the center
    //               point of the line.
    //
    // It is possible to depress multiple modifiers for combined effects e.g.
    // Ctrl+Shift clamps the line to 30 and 45 degree increments i.e.
    // 0, 30, 45, 60, 90, 120, 135, 150, 180, 210, ... degrees.
    //
    // This really only makes sense if drawingALine() returns true, but you are
    // free to call even if it returns false.
    void applyModifiers ();

    // Returns the current points in the shape.  It is updated by beginDraw()
    // (see the class description).
    //
    // draw() sets the last point to the currentPoint().  If drawingALine(), it
    // then calls applyModifiers().
    QPolygon *points () const;
    
    // Returns the mouse button for the drag that created the initial line.
    // Use this - instead of mouseButton() - for determining whether you should
    // use the left mouse button's or right mouse button's color.  This is because
    // the user presses the other mouse button to finish the shape (so mouseButton()
    // will return the wrong one, after the initial line).
    //
    // Only valid if kpTool::hasBegunShape() returns true.
    int originatingMouseButton () const;

    // Returns true if the current drag is visually a line from the 2nd last point
    // of points() to the last point of points() e.g. the initial line drag for
    // a Curve and all drags for a Polygon or Polyline.  draw() will call
    // applyModifiers() and update the statusbar with those 2 points.
    //
    // Returns false if the current drag only draws something based on the last
    // point of points() e.g. a control point of a Bezier curve.  draw() will
    // _not_ call applyModifiers().  It will update the statubar with just that
    // point.
    virtual bool drawingALine () const { return true; }
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
