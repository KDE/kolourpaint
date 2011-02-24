
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


#define DEBUG_KP_VIEW 0
#define DEBUG_KP_VIEW_RENDERER ((DEBUG_KP_VIEW && 1) || 0)


#include <kpView.h>
#include <kpViewPrivate.h>

#include <QPainter>
#include <QPaintEvent>
#include <QTime>
#include <QScrollBar>

#include <KDebug>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDocument.h>
#include <kpTempImage.h>
#include <kpTextSelection.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>

//---------------------------------------------------------------------

// protected
QRect kpView::paintEventGetDocRect (const QRect &viewRect) const
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::paintEventGetDocRect(" << viewRect << ")";
#endif

    QRect docRect;

    // From the "we aren't sure whether to round up or round down" department:

    if (zoomLevelX () < 100 || zoomLevelY () < 100)
        docRect = transformViewToDoc (viewRect);
    else
    {
        // think of a grid - you need to fully cover the zoomed-in pixels
        // when docRect is zoomed back to the view later
        docRect = QRect (transformViewToDoc (viewRect.topLeft ()),  // round down
                         transformViewToDoc (viewRect.bottomRight ()));  // round down
    }

    if (zoomLevelX () % 100 || zoomLevelY () % 100)
    {
        // at least round up the bottom-right point and deal with matrix weirdness:
        // - helpful because it ensures we at least cover the required area
        //   at e.g. 67% or 573%
        docRect.setBottomRight (docRect.bottomRight () + QPoint (2, 2));
    }

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tdocRect=" << docRect;
#endif
    kpDocument *doc = document ();
    if (doc)
    {
        docRect = docRect.intersect (doc->rect ());
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tintersect with doc=" << docRect;
    #endif
    }

    return docRect;
}

//---------------------------------------------------------------------

// public static
void kpView::drawTransparentBackground (QPainter *painter,
                                        const QPoint &patternOrigin,
                                        const QRect &viewRect,
                                        bool isPreview)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::drawTransparentBackground() patternOrigin="
              << patternOrigin
              << " viewRect=" << viewRect
              << " isPreview=" << isPreview
               << endl;
#endif

    const int cellSize = !isPreview ? 16 : 10;

    // TODO: % is unpredictable with negatives.

    int starty = viewRect.y ();
    if ((starty - patternOrigin.y ()) % cellSize)
        starty -= ((starty - patternOrigin.y ()) % cellSize);

    int startx = viewRect.x ();
    if ((startx - patternOrigin.x ()) % cellSize)
        startx -= ((startx - patternOrigin.x ()) % cellSize);

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tstartXY=" << QPoint (startx, starty);
#endif

    painter->save ();

    // Clip to <viewRect> as we may draw outside it on all sides.
    painter->setClipRect (viewRect, Qt::IntersectClip/*honor existing clip*/);

    for (int y = starty; y <= viewRect.bottom (); y += cellSize)
    {
        for (int x = startx; x <= viewRect.right (); x += cellSize)
        {
            bool parity = ((x - patternOrigin.x ()) / cellSize +
                (y - patternOrigin.y ()) / cellSize) % 2;
            QColor col;

            if (parity)
            {
                if (!isPreview)
                    col = QColor (213, 213, 213);
                else
                    col = QColor (224, 224, 224);
            }
            else
                col = Qt::white;

            painter->fillRect (x, y, cellSize, cellSize, col);
        }
    }

    painter->restore ();
}

//---------------------------------------------------------------------

// protected
void kpView::paintEventDrawCheckerBoard (QPainter *painter, const QRect &viewRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView(" << objectName ()
               << ")::paintEventDrawCheckerBoard(viewRect=" << viewRect
               << ") origin=" << origin () << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    QPoint patternOrigin = origin ();

    if (scrollableContainer ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tscrollableContainer: contents[XY]="
                   << QPoint (scrollableContainer ()->horizontalScrollBar()->value (),
                              scrollableContainer ()->verticalScrollBar()->value ())
                   << endl;
    #endif
        // Make checkerboard appear static relative to the scroll view.
        // This makes it more obvious that any visible bits of the
        // checkboard represent transparent pixels and not gray and white
        // squares.
        patternOrigin = QPoint (scrollableContainer ()->horizontalScrollBar()->value(),
                                scrollableContainer ()->verticalScrollBar()->value());
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\t\tpatternOrigin=" << patternOrigin;
    #endif
    }

    // TODO: this static business doesn't work yet
    patternOrigin = QPoint (0, 0);

    drawTransparentBackground (painter, patternOrigin, viewRect);
}

//---------------------------------------------------------------------

// protected
void kpView::paintEventDrawSelection (QImage *destPixmap, const QRect &docRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1 || 0
    kDebug () << "kpView::paintEventDrawSelection() docRect=" << docRect;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
    #if DEBUG_KP_VIEW_RENDERER && 1 || 0
        kDebug () << "\tno doc - abort";
    #endif
        return;
    }

    kpAbstractSelection *sel = doc->selection ();
    if (!sel)
    {
    #if DEBUG_KP_VIEW_RENDERER && 1 || 0
        kDebug () << "\tno sel - abort";
    #endif
        return;
    }


    //
    // Draw selection pixmap (if there is one)
    //
#if DEBUG_KP_VIEW_RENDERER && 1 || 0
    kDebug () << "\tdraw sel pixmap @ " << sel->topLeft ();
#endif
    sel->paint (destPixmap, docRect);


    //
    // Draw selection border
    //

    kpViewManager *vm = viewManager ();
#if DEBUG_KP_VIEW_RENDERER && 1 || 0
    kDebug () << "\tsel border visible="
               << vm->selectionBorderVisible ()
               << endl;
#endif
    if (vm->selectionBorderVisible ())
    {
        sel->paintBorder (destPixmap, docRect, vm->selectionBorderFinished ());
    }


    //
    // Draw text cursor
    //

    // TODO: It would be nice to display the text cursor even if it's not
    //       within the text box (this can happen if the text box is too
    //       small for the text it contains).
    //
    //       However, too much selection repaint code assumes that it
    //       only paints inside its kpAbstractSelection::boundingRect().
    kpTextSelection *textSel = dynamic_cast <kpTextSelection *> (sel);
    if (textSel &&
        vm->textCursorEnabled () &&
        (vm->textCursorBlinkState () ||
        // For the current main window:
        //     As long as _any_ view has focus, blink _all_ views not just the
        //     one with focus.
        !vm->hasAViewWithFocus ()))  // sync: call will break when vm is not held by 1 mainWindow
    {
        QRect rect = vm->textCursorRect ();
        rect = rect.intersect (textSel->textAreaRect ());
        if (!rect.isEmpty ())
        {
            kpPixmapFX::fillXORRect (destPixmap,
                rect.x () - docRect.x (), rect.y () - docRect.y (),
                rect.width (), rect.height (),
                kpColor::White/*XOR color*/,
                kpColor::LightGray/*1st hint color if XOR not supported*/,
                kpColor::DarkGray/*2nd hint color if XOR not supported*/);
        }
    }
}

//---------------------------------------------------------------------

// protected
bool kpView::selectionResizeHandleAtomicSizeCloseToZoomLevel () const
{
    return (abs (selectionResizeHandleAtomicSize () - zoomLevelX () / 100) < 3);
}

//---------------------------------------------------------------------

// protected
void kpView::paintEventDrawSelectionResizeHandles (const QRect &clipRect)
{
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::paintEventDrawSelectionResizeHandles("
               << clipRect << ")" << endl;
#endif

    if (!selectionLargeEnoughToHaveResizeHandles ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tsel not large enough to have resize handles";
    #endif
        return;
    }

    kpViewManager *vm = viewManager ();
    if (!vm || !vm->selectionBorderVisible () || !vm->selectionBorderFinished ())
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tsel border not visible or not finished";
    #endif

        return;
    }

    const QRect selViewRect = selectionViewRect ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tselViewRect=" << selViewRect;
#endif
    if (!selViewRect.intersects (clipRect))
    {
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tdoesn't intersect viewRect";
    #endif
        return;
    }

    QRegion selResizeHandlesRegion = selectionResizeHandlesViewRegion (true/*for renderer*/);
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tsel resize handles view region="
               << selResizeHandlesRegion << endl;
#endif

    const bool doXor = !selectionResizeHandleAtomicSizeCloseToZoomLevel ();
    foreach (const QRect &r, selResizeHandlesRegion.rects ())
    {
        QRect s = r.intersect (clipRect);
        if (s.isEmpty ())
            continue;

        if (doXor)
        {
            kpPixmapFX::widgetFillXORRect (this,
                s.x (), s.y (), s.width (), s.height (),
                kpColor::White/*XOR color*/,
                kpColor::Aqua/*1st hint color if XOR not supported*/,
                kpColor::Red/*2nd hint color if XOR not supported*/);
        }
        else
        {
            QPainter p (this);
            p.fillRect (s, Qt::cyan);
        }
    }
}

//---------------------------------------------------------------------

// protected
void kpView::paintEventDrawTempImage (QImage *destPixmap, const QRect &docRect)
{
    kpViewManager *vm = viewManager ();
    if (!vm)
        return;

    const kpTempImage *tpi = vm->tempImage ();
#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView::paintEventDrawTempImage() tempImage="
               << tpi
               << " isVisible="
               << (tpi ? tpi->isVisible (vm) : false)
               << endl;
#endif

    if (!tpi || !tpi->isVisible (vm))
        return;

    tpi->paint (destPixmap, docRect);
}

//---------------------------------------------------------------------

// protected
void kpView::paintEventDrawGridLines (QPainter *painter, const QRect &viewRect)
{
    int hzoomMultiple = zoomLevelX () / 100;
    int vzoomMultiple = zoomLevelY () / 100;

    QPen ordinaryPen (Qt::gray);
    QPen tileBoundaryPen (Qt::lightGray);

    painter->setPen (ordinaryPen);

    // horizontal lines
    int starty = viewRect.top ();
    if (starty % vzoomMultiple)
        starty = (starty + vzoomMultiple) / vzoomMultiple * vzoomMultiple;
#if 0
    int tileHeight = 16 * vzoomMultiple;  // CONFIG
#endif
    for (int y = starty; y <= viewRect.bottom (); y += vzoomMultiple)
    {
    #if 0
        if (tileHeight > 0 && (y - viewRect.y ()) % tileHeight == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }
    #endif

        painter->drawLine (viewRect.left (), y, viewRect.right (), y);

    #if 0
        if (tileHeight > 0 && (y - viewRect.y ()) % tileHeight == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    #endif
    }

    // vertical lines
    int startx = viewRect.left ();
    if (startx % hzoomMultiple)
        startx = (startx + hzoomMultiple) / hzoomMultiple * hzoomMultiple;
#if 0
    int tileWidth = 16 * hzoomMultiple;  // CONFIG
#endif
    for (int x = startx; x <= viewRect.right (); x += hzoomMultiple)
    {
    #if 0
        if (tileWidth > 0 && (x - viewRect.x ()) % tileWidth == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }
    #endif

        painter->drawLine (x, viewRect.top (), x, viewRect.bottom ());

    #if 0
        if (tileWidth > 0 && (x - viewRect.x ()) % tileWidth == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    #endif
    }
}

//---------------------------------------------------------------------

// This is called "_Unclipped" because it may draw outside of
// <viewRect>.
//
// There are 2 reasons for doing so:
//
// A. If, for instance:
//
//    1. <viewRect> = QRect (0, 0, 2, 3) [top-left of the view]
//    2. zoomLevelX() == 800
//    3. zoomLevelY() == 800
//
//    Then, the local variable <docRect> will be QRect (0, 0, 1, 1).
//    When the part of the document corresponding to <docRect>
//    (a single document pixel) is drawn with QPainter::scale(), the
//    view rectangle QRect (0, 0, 7, 7) will be overwritten due to the
//    8x zoom.  This view rectangle is bigger than <viewRect>.
//
//    We can't use QPainter::setClipRect() since it is buggy in Qt 4.3.1
//    and clips too many pixels when used in combination with scale()
//    [qt-bugs@trolltech.com issue N181038].  ==> MK 10.2.2011 - fixed since Qt-4.4.4
//
// B. paintEventGetDocRect() may, by design, return a larger document
//    rectangle than what <viewRect> corresponds to, if the zoom levels
//    are not perfectly divisible by 100.
//
// This over-drawing is dangerous -- see the comments in paintEvent().
// This over-drawing is only safe from Qt's perspective since Qt
// automatically clips all drawing in paintEvent() (which calls us) to
// QPaintEvent::region().
void kpView::paintEventDrawDoc_Unclipped (const QRect &viewRect)
{
#if DEBUG_KP_VIEW_RENDERER
    QTime timer;
    timer.start ();
    kDebug () << "\tviewRect=" << viewRect;
#endif

    kpViewManager *vm = viewManager ();
    const kpDocument *doc = document ();

    Q_ASSERT (vm);
    Q_ASSERT (doc);

    if (viewRect.isEmpty ())
        return;

    QRect docRect = paintEventGetDocRect (viewRect);

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tdocRect=" << docRect;
#endif

    QPainter painter (this);
    //painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImage docPixmap;
    bool tempImageWillBeRendered = false;

    // LOTODO: I think <docRect> being empty would be a bug.
    if (!docRect.isEmpty ())
    {
        docPixmap = doc->getImageAt (docRect);

    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tdocPixmap.hasAlphaChannel()="
                  << docPixmap.hasAlphaChannel () << endl;
    #endif

        tempImageWillBeRendered =
            (!doc->selection () &&
             vm->tempImage () &&
             vm->tempImage ()->isVisible (vm) &&
             docRect.intersects (vm->tempImage ()->rect ()));

    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\ttempImageWillBeRendered=" << tempImageWillBeRendered
                   << " (sel=" << doc->selection ()
                   << " tempImage=" << vm->tempImage ()
                   << " tempImage.isVisible=" << (vm->tempImage () ? vm->tempImage ()->isVisible (vm) : false)
                   << " docRect.intersects(tempImage.rect)=" << (vm->tempImage () ? docRect.intersects (vm->tempImage ()->rect ()) : false)
                   << ")"
                   << endl;
    #endif
    }


    //
    // Draw checkboard for transparent images and/or views with borders
    //

    if (docPixmap.hasAlphaChannel() ||
        (tempImageWillBeRendered && vm->tempImage ()->paintMayAddMask ()))
    {
        paintEventDrawCheckerBoard (&painter, viewRect);
    }

    if (!docRect.isEmpty ())
    {
        //
        // Draw docPixmap + tempImage
        //

        if (doc->selection ())
        {
            paintEventDrawSelection (&docPixmap, docRect);
        }
        else if (tempImageWillBeRendered)
        {
            paintEventDrawTempImage (&docPixmap, docRect);
        }

    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\torigin=" << origin ();
    #endif
        // Blit scaled version of docPixmap + tempImage.
    #if DEBUG_KP_VIEW_RENDERER && 1
        QTime scaleTimer; scaleTimer.start ();
    #endif
        // This is the only troublesome part of the method that draws unclipped.
        painter.translate (origin ().x (), origin ().y ());
        painter.scale (double (zoomLevelX ()) / 100.0,
                       double (zoomLevelY ()) / 100.0);
        painter.drawImage (docRect, docPixmap);
        //painter.resetMatrix ();  // back to 1-1 scaling
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tscale time=" << scaleTimer.elapsed ();
    #endif

    }  // if (!docRect.isEmpty ()) {

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tdrawDocRect done in: " << timer.restart () << "ms";
#endif
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpView::paintEvent (QPaintEvent *e)
{
    // sync: kpViewPrivate
    // WARNING: document(), viewManager() and friends might be 0 in this method.
    // TODO: I'm not 100% convinced that we always check if their friends are 0.

#if DEBUG_KP_VIEW_RENDERER && 1
    QTime timer;
    timer.start ();
#endif

    kpViewManager *vm = viewManager ();

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "kpView(" << objectName () << ")::paintEvent() vm=" << (bool) vm
               << " queueUpdates=" << (vm && vm->queueUpdates ())
               << " fastUpdates=" << (vm && vm->fastUpdates ())
               << " viewRect=" << e->rect ()
               << " topLeft=" << QPoint (x (), y ())
               << endl;
#endif

    if (!vm)
        return;

    if (vm->queueUpdates ())
    {
        // OPT: if this update was due to the document,
        //      use document coordinates (in case of a zoom change in
        //      which view coordinates become out of date)
        addToQueuedArea (e->region ());
        return;
    }

    kpDocument *doc = document ();
    if (!doc)
        return;


    // It seems that e->region() is already clipped by Qt to the visible
    // part of the view (which could be quite small inside a scrollview).
    QRegion viewRegion = e->region ();
    QVector <QRect> rects = viewRegion.rects ();
#if DEBUG_KP_VIEW_RENDERER
    kDebug () << "\t#rects = " << rects.count ();
#endif

    // Draw all of the requested regions of the document _before_ drawing
    // the grid lines, buddy rectangle and selection resize handles.
    // This ordering is important since paintEventDrawDoc_Unclipped()
    // may draw outside of the view rectangle passed to it.
    //
    // To illustrate this, suppose we changed each iteration of the loop
    // to call paintEventDrawDoc_Unclipped() _and_ then,
    // paintEventDrawGridLines().  If there are 2 or more iterations of this
    // loop, paintEventDrawDoc_Unclipped() in one iteration may draw over
    // parts of nearby grid lines (which were drawn in a previous iteration)
    // with document pixels.  Those grid line parts are probably not going to
    // be redrawn, so will appear to be missing.
    foreach (const QRect &r, rects)
    {
        paintEventDrawDoc_Unclipped (r);
    }


    //
    // Draw Grid Lines
    //

    if (isGridShown ())
    {
        QPainter painter (this);
    #if DEBUG_KP_VIEW_RENDERER && 1
        QTime gridTimer; gridTimer.start ();
    #endif
        foreach (const QRect &r, rects)
            paintEventDrawGridLines (&painter, r);
    #if DEBUG_KP_VIEW_RENDERER && 1
        kDebug () << "\tgrid time=" << gridTimer.elapsed ();
    #endif
    }


    const QRect bvsvRect = buddyViewScrollableContainerRectangle ();
    if (!bvsvRect.isEmpty ())
    {
        kpPixmapFX::widgetDrawStippledXORRect (this,
            bvsvRect.x (), bvsvRect.y (), bvsvRect.width (), bvsvRect.height (),
            kpColor::White, kpColor::White,  // Stippled XOR colors
            kpColor::LightGray, kpColor::DarkGray,  // Hint colors if XOR not supported
            e->rect ());
    }


    if (doc->selection ())
    {
        // Draw resize handles on top of possible grid lines
        paintEventDrawSelectionResizeHandles (e->rect ());
    }

#if DEBUG_KP_VIEW_RENDERER && 1
    kDebug () << "\tall done in: " << timer.restart () << "ms";
#endif
}

//---------------------------------------------------------------------
