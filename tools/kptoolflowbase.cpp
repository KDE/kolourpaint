
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


#define DEBUG_KP_TOOL_FLOW_BASE 0

#include <kptoolflowbase.h>

#include <cstdlib>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qimage.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpcursorprovider.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpimage.h>
#include <kpmainwindow.h>
#include <kppainter.h>
#include <kppixmapfx.h>
#include <kptemppixmap.h>
#include <kptoolclear.h>
#include <kptoolflowcommand.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetbrush.h>
#include <kptoolwidgeterasersize.h>
#include <kpviewmanager.h>


struct kpToolFlowBasePrivate
{
    kpToolWidgetBrush *toolWidgetBrush;
    kpToolWidgetEraserSize *toolWidgetEraserSize;


    //
    // Cursor and Brush Data
    // (must be zero if unused)
    //
        
        kpTempPixmap::UserFunctionType brushDrawFunc, cursorDrawFunc;

        // Can't use union since package types contain fields requiring
        // constructors.
        kpToolWidgetBrush::DrawPackage brushDrawPackageForMouseButton [2];
        kpToolWidgetEraserSize::DrawPackage eraserDrawPackageForMouseButton [2];
        
        void *drawPackageForMouseButton [2];
    
        int brushWidth, brushHeight;
        int cursorWidth, cursorHeight;
        
        bool brushIsDiagonalLine;


    kpToolFlowCommand *currentCommand;
};


kpToolFlowBase::kpToolFlowBase (const QString &text, const QString &description,
        int key,
        kpMainWindow *mainWindow, const QString &name)
        
    : kpTool (text, description, key, mainWindow, name),
      d (new kpToolFlowBasePrivate ())
{
    d->toolWidgetBrush = 0;
    d->toolWidgetEraserSize = 0;

    clearBrushCursorData ();

    d->currentCommand = 0;
}

kpToolFlowBase::~kpToolFlowBase ()
{
    delete d;
}


// private
void kpToolFlowBase::clearBrushCursorData ()
{
    d->brushDrawFunc = d->cursorDrawFunc = 0;

    memset (&d->brushDrawPackageForMouseButton,
        0,
        sizeof (d->brushDrawPackageForMouseButton));
    memset (&d->eraserDrawPackageForMouseButton,
        0,
        sizeof (d->eraserDrawPackageForMouseButton));

    memset (&d->drawPackageForMouseButton,
        0,
        sizeof (d->drawPackageForMouseButton));

    d->brushWidth = d->brushHeight = 0;
    d->cursorWidth = d->cursorHeight = 0;

    d->brushIsDiagonalLine = false;
}


// virtual
void kpToolFlowBase::begin ()
{
    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

    // TODO: Bad smell.  Mutually exclusive.  Use inheritance.
    if (haveSquareBrushes ())
    {
        d->toolWidgetEraserSize = tb->toolWidgetEraserSize ();
        connect (d->toolWidgetEraserSize, SIGNAL (eraserSizeChanged (int)),
                 this, SLOT (updateBrushAndCursor ()));
        d->toolWidgetEraserSize->show ();

        updateBrushAndCursor ();

        viewManager ()->setCursor (kpCursorProvider::lightCross ());
    }
    else if (haveDiverseBrushes ())
    {
        d->toolWidgetBrush = tb->toolWidgetBrush ();
        connect (d->toolWidgetBrush, SIGNAL (brushChanged ()),
                 this, SLOT (updateBrushAndCursor ()));
        d->toolWidgetBrush->show ();

        updateBrushAndCursor ();

        viewManager ()->setCursor (kpCursorProvider::lightCross ());
    }

    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolFlowBase::end ()
{
    if (d->toolWidgetEraserSize)
    {
        disconnect (d->toolWidgetEraserSize, SIGNAL (eraserSizeChanged (int)),
                    this, SLOT (updateBrushAndCursor ()));
        d->toolWidgetEraserSize = 0;
    }
    else if (d->toolWidgetBrush)
    {
        disconnect (d->toolWidgetBrush, SIGNAL (brushChanged ()),
                    this, SLOT (updateBrushAndCursor ()));
        d->toolWidgetBrush = 0;
    }

    kpViewManager *vm = viewManager ();
    Q_ASSERT (vm);

    if (vm->tempPixmap () && vm->tempPixmap ()->isBrush ())
        vm->invalidateTempPixmap ();

    if (haveAnyBrushes ())
        vm->unsetCursor ();

    clearBrushCursorData ();
}

// virtual
void kpToolFlowBase::beginDraw ()
{
    d->currentCommand = new kpToolFlowCommand (text (), mainWindow ());

    // we normally show the Brush pix in the foreground colour but if the
    // user starts drawing in the background color, we don't want to leave
    // the cursor in the foreground colour -- just hide it in all cases
    // to avoid confusion
    viewManager ()->invalidateTempPixmap ();

    setUserMessage (cancelUserMessage ());
}

// virtual
void kpToolFlowBase::hover (const QPoint &point)
{
#if DEBUG_KP_TOOL_FLOW_BASE && 0
    kDebug () << "kpToolFlowBase::hover(" << point << ")"
               << " hasBegun=" << hasBegun ()
               << " hasBegunDraw=" << hasBegunDraw ()
               << " cursorPixmap.isNull=" << m_cursorPixmap.isNull ()
               << endl;
#endif
    if (point != KP_INVALID_POINT && d->cursorDrawFunc)
    {
        // TODO: why this?
        m_mouseButton = 0;

        viewManager ()->setFastUpdates ();

        viewManager ()->setTempPixmap (
            kpTempPixmap (true/*brush*/,
                hotPoint (),
                d->cursorDrawFunc, d->drawPackageForMouseButton [0/*left button*/],
                d->cursorWidth, d->cursorHeight));

        viewManager ()->restoreFastUpdates ();
    }

#if DEBUG_KP_TOOL_FLOW_BASE && 0
    if (document ()->rect ().contains (point))
    {
        QImage image = kpPixmapFX::convertToImage (*document ()->pixmap ());

        QRgb v = image.pixel (point.x (), point.y ());
        kDebug () << "(" << point << "): r=" << qRed (v)
                    << " g=" << qGreen (v)
                    << " b=" << qBlue (v)
                    << " a=" << qAlpha (v)
                    << endl;
    }
#endif

    setUserShapePoints (point);
}

static int randomNumberFrom0to99 ()
{
    return (rand () % 100);
}

QList <QPoint> kpToolFlowBase::interpolatePoints (const QPoint &thisPoint,
    const QPoint &lastPoint,
    double probability)
{
    QList <QPoint> ret;
 
    const int probabilityTimes100 = int (probability * 100);
#define SHOULD_DRAW()  (probabilityTimes100 == 100/*avoid rand() call*/ ||  \
                        ::randomNumberFrom0to99 () < probabilityTimes100)

#if 0                        
    kDebug () << "prob=" << probability
               << " *100=" << probabilityTimes100
               << endl;
#endif

   
    // Sweeps a pixmap along a line (modified Bresenham's line algorithm,
    // see MODIFIED comment below).
    //
    // Derived from the zSprite2 Graphics Engine

    const int x1 = thisPoint.x (),
                y1 = thisPoint.y (),
                x2 = lastPoint.x (),
                y2 = lastPoint.y ();

    // Difference of x and y values
    const int dx = x2 - x1;
    const int dy = y2 - y1;

    // Absolute values of differences
    const int ix = qAbs (dx);
    const int iy = qAbs (dy);

    // Larger of the x and y differences
    const int inc = ix > iy ? ix : iy;

    // Plot location
    int plotx = x1;
    int ploty = y1;

    int x = 0;
    int y = 0;

    if (SHOULD_DRAW ())
        ret.append (QPoint (plotx, ploty));
    
    
    for (int i = 0; i <= inc; i++)
    {
        // oldplotx is equally as valid but would look different
        // (but nobody will notice which one it is)
        int oldploty = ploty;
        int plot = 0;

        x += ix;
        y += iy;

        if (x > inc)
        {
            plot++;
            x -= inc;

            if (dx < 0)
                plotx--;
            else
                plotx++;
        }

        if (y > inc)
        {
            plot++;
            y -= inc;

            if (dy < 0)
                ploty--;
            else
                ploty++;
        }

        if (plot)
        {
            if (d->brushIsDiagonalLine && plot == 2)
            {
                // MODIFIED: every point is
                // horizontally or vertically adjacent to another point (if there
                // is more than 1 point, of course).  This is in contrast to the
                // ordinary line algorithm which can create diagonal adjacencies.

                if (SHOULD_DRAW ())
                    ret.append (QPoint (plotx, oldploty));
            }

            if (SHOULD_DRAW ())
                ret.append (QPoint (plotx, ploty));
        }    
    }
    
#undef SHOULD_DRAW

    return ret;
}


void kpToolFlowBase::drawLineSetupPainterMask (QPixmap *pixmap,
    QBitmap *maskBitmap,
    QPainter *painter, QPainter *maskPainter)
{
    const kpColor col = color (m_mouseButton);

    if (!col.isTransparent ())
    {
        painter->begin (pixmap);
        painter->setPen (col.toQColor ());
    }

    if (col.isTransparent () || !pixmap->mask ().isNull ())
    {
        *maskBitmap = kpPixmapFX::getNonNullMask (*pixmap);
        maskPainter->begin (maskBitmap);
        maskPainter->setPen (col.maskColor ());
    }
}

void kpToolFlowBase::drawLineTearDownPainterMask (QPixmap *pixmap,
    const QBitmap *maskBitmap,
    QPainter *painter, QPainter *maskPainter,
    bool drawingHappened)
{        
    if (painter->isActive ())
        painter->end ();

    if (maskPainter->isActive ())
        maskPainter->end ();


    if (drawingHappened)
    {
        if (!maskBitmap->isNull ())
            pixmap->setMask (*maskBitmap);
    }
}


// virtual
QRect kpToolFlowBase::drawPoint (const QPoint &point)
{
    return drawLine (point, point);
}

// virtual
void kpToolFlowBase::draw (const QPoint &thisPoint, const QPoint &lastPoint, const QRect &normalizedRect)
{
    if (!drawShouldProceed (thisPoint, lastPoint, normalizedRect))
        return;

    // sync: remember to restoreFastUpdates() in all exit paths
    viewManager ()->setFastUpdates ();

    QRect dirtyRect;

    // TODO: I'm beginning to wonder this drawPoint() "optimization" actually
    //       optimises.  Is it worth the complexity?  Hence drawPoint() impl above.
    if (d->brushIsDiagonalLine ?
            currentPointCardinallyNextToLast () :
            currentPointNextToLast ())
    {
        dirtyRect = drawPoint (thisPoint);
    }
    // in reality, the system is too slow to give us all the MouseMove events
    // so we "interpolate" the missing points :)
    else
    {
        dirtyRect = drawLine (thisPoint, lastPoint);
    }

    d->currentCommand->updateBoundingRect (dirtyRect);

    viewManager ()->restoreFastUpdates ();
    setUserShapePoints (thisPoint);
}

// virtual
void kpToolFlowBase::cancelShape ()
{
    d->currentCommand->finalize ();
    d->currentCommand->cancel ();

    delete d->currentCommand;
    d->currentCommand = 0;

    updateBrushAndCursor ();

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolFlowBase::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolFlowBase::endDraw (const QPoint &, const QRect &)
{
    d->currentCommand->finalize ();
    mainWindow ()->commandHistory ()->addCommand (d->currentCommand, false /* don't exec */);

    // don't delete - it's up to the commandHistory
    d->currentCommand = 0;

    updateBrushAndCursor ();

    setUserMessage (haventBegunDrawUserMessage ());
}


// TODO: maybe the base should be virtual?
kpColor kpToolFlowBase::color (int which)
{
#if DEBUG_KP_TOOL_FLOW_BASE && 0
    kDebug () << "kpToolFlowBase::color (" << which << ")" << endl;
#endif

    // Pen & Brush
    if (!colorsAreSwapped ())
        return kpTool::color (which);
    // only the (Color) Eraser uses the opposite color
    else
        return kpTool::color (which ? 0 : 1);  // don't trust !0 == 1
}


// protected
kpTempPixmap::UserFunctionType kpToolFlowBase::brushDrawFunction () const
{
    return d->brushDrawFunc;
}

// protected
void *kpToolFlowBase::brushDrawFunctionData () const
{
    return d->drawPackageForMouseButton [m_mouseButton];
}


// protected
int kpToolFlowBase::brushWidth () const
{
    return d->brushWidth;
}

// protected
int kpToolFlowBase::brushHeight () const
{
    return d->brushHeight;
}

// protected
kpToolFlowCommand *kpToolFlowBase::currentCommand () const
{
    return d->currentCommand;
}

// protected
void kpToolFlowBase::updateBrushAndCursor ()
{
#if DEBUG_KP_TOOL_FLOW_BASE && 1
    kDebug () << "kpToolFlowBase::updateBrushAndCursor()" << endl;
#endif

    if (haveSquareBrushes ())
    {
        d->brushDrawFunc = d->toolWidgetEraserSize->drawFunction ();
        d->cursorDrawFunc = d->toolWidgetEraserSize->drawCursorFunction ();

        for (int i = 0; i < 2; i++)
        {
            d->drawPackageForMouseButton [i] =
                &(d->eraserDrawPackageForMouseButton [i] =
                    d->toolWidgetEraserSize->drawFunctionData (color (i)));
        }

        d->brushWidth = d->brushHeight =
            d->cursorWidth = d->cursorHeight =
                d->toolWidgetEraserSize->eraserSize ();

        d->brushIsDiagonalLine = false;
    }
    else if (haveDiverseBrushes ())
    {
        d->brushDrawFunc = d->cursorDrawFunc = d->toolWidgetBrush->drawFunction ();

        for (int i = 0; i < 2; i++)
        {
            d->drawPackageForMouseButton [i] =
                &(d->brushDrawPackageForMouseButton [i] =
                    d->toolWidgetBrush->drawFunctionData (color (i)));
        }

        d->brushWidth = d->brushHeight =
            d->cursorWidth = d->cursorHeight =
                d->toolWidgetBrush->brushSize ();

        d->brushIsDiagonalLine = d->toolWidgetBrush->brushIsDiagonalLine ();
    }

    hover (hasBegun () ? m_currentPoint : currentPoint ());
}


// virtual private slot
void kpToolFlowBase::slotForegroundColorChanged (const kpColor & /*col*/)
{
#if DEBUG_KP_TOOL_FLOW_BASE
    kDebug () << "kpToolFlowBase::slotForegroundColorChanged()" << endl;
#endif

    updateBrushAndCursor ();
}

// virtual private slot
void kpToolFlowBase::slotBackgroundColorChanged (const kpColor & /*col*/)
{
#if DEBUG_KP_TOOL_FLOW_BASE
    kDebug () << "kpToolFlowBase::slotBackgroundColorChanged()" << endl;
#endif

    updateBrushAndCursor ();
}


QPoint kpToolFlowBase::hotPoint () const
{
    return hotPoint (m_currentPoint);
}

QPoint kpToolFlowBase::hotPoint (int x, int y) const
{
    return hotPoint (QPoint (x, y));
}

QPoint kpToolFlowBase::hotPoint (const QPoint &point) const
{
    /*
     * e.g.
     *    Width 5:
     *    0 1 2 3 4
     *        ^
     *        |
     *      Center
     */
    return point - QPoint (d->brushWidth / 2, d->brushHeight / 2);
}

QRect kpToolFlowBase::hotRect () const
{
    return hotRect (m_currentPoint);
}

QRect kpToolFlowBase::hotRect (int x, int y) const
{
    return hotRect (QPoint (x, y));
}

QRect kpToolFlowBase::hotRect (const QPoint &point) const
{
    QPoint topLeft = hotPoint (point);
    return QRect (topLeft.x (), topLeft.y (), d->brushWidth, d->brushHeight);
}


#include <kptoolflowbase.moc>
