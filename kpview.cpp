
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define DEBUG_KP_VIEW 0

#include <math.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>
#if DEBUG_KP_VIEW
    #include <qdatetime.h>
#endif

#include <kdebug.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptool.h>
#include <kptoolpen.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpView::kpView (QWidget *parent, const char *name,
                kpMainWindow *mainWindow,
                int width, int height,
                bool autoVariableZoom)
    : QWidget (parent, name, Qt::WStaticContents | Qt::WNoAutoErase /* no flicker */),
      m_mainWindow (mainWindow),
      m_autoVariableZoom (autoVariableZoom),
      m_hzoom (100), m_vzoom (100),
      m_showGrid (false),
      m_backBuffer (0),
      m_origin (QPoint (0, 0)),
      m_needBorder (false)
{
    m_docToViewMatrix.setTransformationMode (QWMatrix::Areas);

    if (autoVariableZoom)
    {
        updateVariableZoom (width, height);
        setMinimumSize (width, height);
    }
    resize (width, height);

    setFocusPolicy (QWidget::WheelFocus);
    setMouseTracking (true);  // mouseMoveEvent's even when no mousebtn down
}

void kpView::setHasMouse (bool yes)
{
    kpViewManager *vm = viewManager ();

    if (vm)
    {
        if (yes && vm->viewUnderCursor () != this)
            vm->setViewUnderCursor (this);
        else if (!yes && vm->viewUnderCursor () == this)
            vm->setViewUnderCursor (0);
    }
}

kpView::~kpView ()
{
    setHasMouse (false);
    delete m_backBuffer; m_backBuffer = 0;
}


/*
 * Zoom/Grid
 */

kpViewManager *kpView::viewManager () const
{
    return m_mainWindow ? m_mainWindow->viewManager () : 0;
}

kpDocument *kpView::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

bool kpView::hasVariableZoom () const
{
    return m_autoVariableZoom;
}

bool kpView::slotUpdateVariableZoom ()
{
    return updateVariableZoom (width (), height ());
}

bool kpView::updateVariableZoom (int viewWidth, int viewHeight)
{
#if DEBUG_KP_VIEW
    kdDebug () << "kpView::updateVariableZoom (" << viewWidth << "," << viewHeight << ")" << endl;
#endif

    if (!hasVariableZoom ())
    {
        kdError () << "kpView::slotUupdateVariableZoom() called with !hasVariableZoom" << endl;
        m_hzoom = m_vzoom = 100;
        m_origin = QPoint (0, 0);
        m_needBorder = false;
        return false;
    }

    kpDocument *doc = document ();
    if (!doc)
    {
        m_hzoom = m_vzoom = 100;
        m_origin = QPoint (0, 0);
        m_needBorder = false;
        return false;
    }

#if DEBUG_KP_VIEW
    kdDebug () << "\tdoc: width=" << doc->width ()
               << " height=" << doc->height ()
               << endl;
#endif

    m_hzoom = viewWidth * 100 / doc->width ();
    m_vzoom = viewHeight * 100 / doc->height ();

    // keep aspect ratio
    if (m_hzoom < m_vzoom)
        m_vzoom = m_hzoom;
    else
        m_hzoom = m_vzoom;

#if DEBUG_KP_VIEW && 1
    kdDebug () << "\tproposed zoom=" << m_hzoom << endl;
#endif
    if (m_hzoom > 100 || m_vzoom > 100)
    {
    #if DEBUG_KP_VIEW && 1
        kdDebug () << "\twon't magnify - setting zoom to 100%" << endl;
    #endif
        m_hzoom = 100, m_vzoom = 100;
    }

    int zoomedDocWidth = doc->width () * m_hzoom / 100;
    int zoomedDocHeight = doc->height () * m_vzoom / 100;

    m_origin = QPoint ((viewWidth - zoomedDocWidth) / 2,
                        (viewHeight - zoomedDocHeight) / 2);
#if DEBUG_KP_VIEW && 1
    kdDebug () << "\torigin=" << m_origin << endl;
#endif

    m_needBorder = ((zoomedDocWidth != viewWidth) ||
                    (zoomedDocHeight != viewHeight));

    setMask (QRegion (QRect (m_origin.x (), m_origin.y (),
                      zoomedDocWidth, zoomedDocHeight)));

#if DEBUG_KP_VIEW && 1
    kdDebug () << "\tneedBorder=" << m_needBorder << endl;
#endif

    m_docToViewMatrix.reset ();
    m_docToViewMatrix.scale (m_hzoom / 100.0, m_vzoom / 100.0);

    if (viewManager ())
        viewManager ()->updateView (this);

    return true;
}

int kpView::zoomLevelX (void) const
{
    return m_hzoom;
}

int kpView::zoomLevelY (void) const
{
    return m_vzoom;
}

bool kpView::setZoomLevel (int hzoom, int vzoom)
{
    if (hasVariableZoom ())
    {
        kdError () << "KpView::setZoomLevel() called with autoVariableZoom" << endl;
        return false;
    }

    if (hzoom == m_hzoom && vzoom == m_vzoom)
        return true;

    m_hzoom = hzoom;
    m_vzoom = vzoom;

    m_docToViewMatrix.reset ();
    m_docToViewMatrix.scale (m_hzoom / 100.0, m_vzoom / 100.0);

    resize (zoomDocToViewX (m_mainWindow->document ()->width ()),
            zoomDocToViewY (m_mainWindow->document ()->height ()));

    return true;
}

void kpView::showGrid (bool yes)
{
    if (m_showGrid == yes)
        return;

    m_showGrid = yes;
    if (viewManager ())
        viewManager ()->updateView (this);
}

bool kpView::canShowGrid (int hzoom, int vzoom) const
{
    if (hzoom == -1) hzoom = m_hzoom;
    if (vzoom == -1) vzoom = m_vzoom;

    // minimum zoom level < 600% would probably be reported as a bug by users
    // who thought that the grid was a part of the image!
    return (!hasVariableZoom ()) &&
           (hzoom >= 600 && hzoom % 100 == 0) &&
           (vzoom >= 600 && vzoom % 100 == 0);
}

// view -> doc
int kpView::zoomViewToDocX (int zoomedCoord) const
{
    return (zoomedCoord - m_origin.x ()) * 100 / m_hzoom;
}

int kpView::zoomViewToDocY (int zoomedCoord) const
{
    return (zoomedCoord - m_origin.y ()) * 100 / m_vzoom;
}

QPoint kpView::zoomViewToDoc (const QPoint &zoomedCoord) const
{
    return QPoint (zoomViewToDocX (zoomedCoord.x ()),
                   zoomViewToDocY (zoomedCoord.y ()));
}

QRect kpView::zoomViewToDoc (const QRect &zoomedRect) const
{
    if (m_hzoom == 100 && m_vzoom == 100)
        return zoomedRect;
    else
    {
        QPoint topLeft = zoomViewToDoc (zoomedRect.topLeft ());

        // don't call zoomViewToDoc[XY]() - need to round up dimensions
        int width = qRound (double (zoomedRect.width ()) * 100.0 / double (m_hzoom));
        int height = qRound (double (zoomedRect.height ()) * 100.0 / double (m_vzoom));

        // like QWMatrix::Areas
        return QRect (topLeft.x (), topLeft.y (), width, height);
    }
}

/*
 * doc->view
 */

int kpView::zoomDocToViewX (int doc_coord) const
{
    return (doc_coord * m_hzoom / 100) + m_origin.x ();
}

int kpView::zoomDocToViewY (int doc_coord) const
{
    return (doc_coord * m_vzoom / 100) + m_origin.y ();
}

QPoint kpView::zoomDocToView (const QPoint &doc_coord) const
{
    return QPoint (zoomDocToViewX (doc_coord.x ()),
                   zoomDocToViewY (doc_coord.y ()));
}

QRect kpView::zoomDocToView (const QRect &doc_rect) const
{
    if (m_hzoom == 100 && m_vzoom == 100)
        return doc_rect;
    else
    {
        QPoint topLeft = zoomDocToView (doc_rect.topLeft ());

        // don't call zoomDocToView[XY]() - need to round up dimensions
        int width = qRound (double (doc_rect.width ()) * double (m_hzoom) / 100.0);
        int height = qRound (double (doc_rect.height ()) * double (m_vzoom) / 100.0);

        // like QWMatrix::Areas
        return QRect (topLeft.x (), topLeft.y (), width, height);
    }
}

// virtual
void kpView::resize (int w, int h)
{
    QWidget::resize (w, h);

    // TODO: The QWidget::resizeEvent() description says that this isn't needed
    //       but appears to be needed when resizing the view to a smaller size.
    //
    //       Anyway, we should update a smaller area in some cases.
    if (viewManager ())
        viewManager ()->updateViews ();
}


void kpView::addToQueuedArea (const QRect &rect)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name ()
               << ")::addToQueuedArea() already=" << m_queuedUpdateArea
               << " - plus - " << rect
               << endl;
#endif
    m_queuedUpdateArea += rect;
}

void kpView::addToQueuedArea (const QRegion &region)
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name ()
               << ")::addToQueuedArea()r already=" << m_queuedUpdateArea
               << " - plus - " << region
               << endl;
#endif
    m_queuedUpdateArea += region;
}

void kpView::invalidateQueuedArea ()
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::invalidateQueuedArea()" << endl;
#endif

    m_queuedUpdateArea = QRegion ();
}

void kpView::updateQueuedArea ()
{
    kpViewManager *vm = viewManager ();
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name ()
               << ")::updateQueuedArea() vm=" << (bool) vm
               << " queueUpdates=" << (vm && vm->queueUpdates ())
               << " fastUpdates=" << (vm && vm->fastUpdates ())
               << " area=" << m_queuedUpdateArea
               << endl;
#endif

    if (!vm)
        return;

    if (vm->queueUpdates ())
        return;

    if (!m_queuedUpdateArea.isNull ())
        vm->updateView (this, m_queuedUpdateArea);

    invalidateQueuedArea ();
}


/*
 * Event Handlers
 */

// virtual
void kpView::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::mousePressEvent (" << e->x () << "," << e->y () << ")" << endl;
#endif

    setHasMouse (true);
    m_mainWindow->tool ()->mousePressEvent (e);

    e->accept ();
}

// virtual
void kpView::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::mouseMoveEvent (" << e->x () << "," << e->y () << ")" << endl;
#endif

    setHasMouse (true);
    m_mainWindow->tool ()->mouseMoveEvent (e);

    e->accept ();
}

// virtual
void kpView::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::mouseReleaseEvent (" << e->x () << "," << e->y () << ")" << endl;
#endif

    m_mainWindow->tool ()->mouseReleaseEvent (e);

    e->accept ();
}

// virtual
void kpView::keyPressEvent (QKeyEvent *e)
{
    m_mainWindow->tool ()->keyPressEvent (e);
    e->accept ();
}

// virtual
void kpView::keyReleaseEvent (QKeyEvent *e)
{
    m_mainWindow->tool ()->keyReleaseEvent (e);
    e->accept ();
}

// virtual
void kpView::focusInEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::focusInEvent()" << endl;
#endif
    m_mainWindow->tool ()->focusInEvent (e);
}

// virtual
void kpView::focusOutEvent (QFocusEvent *e)
{
    m_mainWindow->tool ()->focusOutEvent (e);
}

// virtual
void kpView::enterEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::enterEvent" << endl;
#endif

    setHasMouse (true);
    m_mainWindow->tool ()->enterEvent (e);
}

// virtual
void kpView::leaveEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::leaveEvent" << endl;
#endif

    setHasMouse (false);
    m_mainWindow->tool ()->leaveEvent (e);
}

// private virtual
void kpView::dragEnterEvent (QDragEnterEvent *)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::dragEnterEvent" << endl;
#endif

    setHasMouse (true);
}

// private virtual
void kpView::dragLeaveEvent (QDragLeaveEvent *)
{
#if DEBUG_KP_VIEW && 0
    kdDebug () << "kpView::dragLeaveEvent" << endl;
#endif

    setHasMouse (false);
}


//
// Renderer
//

// private
QRect kpView::paintEventGetDocRect (const QRect &viewRect) const
{
#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView::paintEventGetDocRect(" << viewRect << ")" << endl;
#endif

    QRect docRect;

    // From the "we aren't sure whether to round up or round down" department:

    if (m_hzoom < 100 || m_vzoom < 100)
        docRect = zoomViewToDoc (viewRect);
    else
    {
        // think of a grid - you need to fully cover the zoomed-in pixels
        // when docRect is zoomed back to the view later
        docRect = QRect (zoomViewToDoc (viewRect.topLeft ()),  // round down
                         zoomViewToDoc (viewRect.bottomRight ()));  // round down
    }

    if (m_hzoom % 100 || m_vzoom % 100)
    {
        // at least round up the bottom-right point and deal with matrix weirdness:
        // - helpful because it ensures we at least cover the required area
        //   at e.g. 67% or 573%
        // - harmless since Qt clips for us anyway
        docRect.setBottomRight (docRect.bottomRight () + QPoint (2, 2));
    }

#if DEBUG_KP_VIEW && 1
    kdDebug () << "\tdocRect=" << docRect << endl;
#endif
    kpDocument *doc = document ();
    if (doc)
    {
        docRect = docRect.intersect (doc->rect ());
    #if DEBUG_KP_VIEW && 1
        kdDebug () << "\tintersect with doc=" << docRect << endl;
    #endif
    }

    return docRect;
}

// private
void kpView::paintEventDrawCheckerBoard (QPainter *painter, const QRect &viewRect)
{
    m_mainWindow->drawTransparentBackground (painter, width (), height (), viewRect);
}

// private
void kpView::paintEventDrawTempPixmap (QPixmap *destPixmap, const QRect &docRect)
{
    kpViewManager *vm = viewManager ();
    if (!vm) return;

    QRect tempPixmapRect = vm->tempPixmapRect ();
    QPixmap tempPixmap = vm->tempPixmap ();


    //
    // Draw tempPixmap on top of destPixmap
    //

#define PARAMS destPixmap, tempPixmapRect.topLeft () - docRect.topLeft (), tempPixmap
    if (vm->shouldBrushBeDisplayed (this))
        kpPixmapFX::paintPixmapAt (PARAMS);
    else
        kpPixmapFX::setPixmapAt (PARAMS);
#undef PARAMS


    //
    // Draw selection border
    //

    if (vm->selectionActive ())
    {
        QPainter destPixmapPainter (destPixmap);

        destPixmapPainter.setRasterOp (Qt::XorROP);
        destPixmapPainter.setPen (QPen (Qt::white, 1, Qt::DotLine));

        switch (vm->selectionBorderType ())
        {
        case kpViewManager::Rectangle:
        #if DEBUG_KP_VIEW && 1
            kdDebug () << "\tselection border = rectangle" << endl;
            kdDebug () << "\t\tx=" << tempPixmapRect.x () - docRect.x ()
                       << " y=" << tempPixmapRect.y () - docRect.y ()
                       << " w=" << tempPixmapRect.width ()
                       << " h=" << tempPixmapRect.height ()
                       << endl;
        #endif
            destPixmapPainter.drawRect (tempPixmapRect.x () - docRect.x (),
                                        tempPixmapRect.y () - docRect.y (),
                                        tempPixmapRect.width (),
                                        tempPixmap.height ());
            break;

        case kpViewManager::Ellipse:
        #if DEBUG_KP_VIEW && 1
            kdDebug () << "\tselection border = ellipse" << endl;
        #endif
            destPixmapPainter.drawEllipse (tempPixmapRect.x () - docRect.x (),
                                           tempPixmapRect.y () - docRect.y (),
                                           tempPixmapRect.width (),
                                           tempPixmap.height ());
            break;

        case kpViewManager::FreeForm:
        #if DEBUG_KP_VIEW
            kdDebug () << "\tselection border = freeForm" << endl;
        #endif
            break;

        default:
            break;
        }

        destPixmapPainter.end ();
    }
}

// private
void kpView::paintEventDrawGridLines (QPainter *painter, const QRect &viewRect)
{
    int hzoomMultiple = m_hzoom / 100;
    int vzoomMultiple = m_vzoom / 100;

    QPen ordinaryPen (Qt::gray);
    QPen tileBoundaryPen (Qt::lightGray);

    painter->setPen (ordinaryPen);

    // horizontal lines
    int starty = viewRect.top ();
    if (starty % vzoomMultiple)
        starty = (starty + vzoomMultiple) / vzoomMultiple * vzoomMultiple;
    int tileHeight = 16 * vzoomMultiple;  // CONFIG
    for (int y = starty - viewRect.y (); y <= viewRect.bottom () - viewRect.y (); y += vzoomMultiple)
    {
        if (0 && tileHeight > 0 && y % tileHeight == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }

        painter->drawLine (0, y, viewRect.right () - viewRect.left (), y);

        if (0 && tileHeight > 0 && y % tileHeight == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    }

    // vertical lines
    int startx = viewRect.left ();
    if (startx % hzoomMultiple)
        startx = (startx + hzoomMultiple) / hzoomMultiple * hzoomMultiple;
    int tileWidth = 16 * hzoomMultiple;  // CONFIG
    for (int x = startx - viewRect.x (); x <= viewRect.right () - viewRect.x (); x += hzoomMultiple)
    {
        if (0 && tileWidth > 0 && x % tileWidth == 0)
        {
            painter->setPen (tileBoundaryPen);
            //painter.setRasterOp (Qt::XorROP);
        }

        painter->drawLine (x, 0, x, viewRect.bottom () - viewRect.top ());

        if (0 && tileWidth > 0 && x % tileWidth == 0)
        {
            painter->setPen (ordinaryPen);
            //painter.setRasterOp (Qt::CopyROP);
        }
    }
}


// virtual
void kpView::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_VIEW && 1
    QTime timer;
    timer.start ();
#endif

    kpViewManager *vm = viewManager ();
    const kpDocument *doc = document ();

#if DEBUG_KP_VIEW && 1
    kdDebug () << "kpView(" << name () << ")::paintEvent() vm=" << (bool) vm
               << " queueUpdates=" << (vm && vm->queueUpdates ())
               << " viewRect=" << e->rect ()
               << " erased=" << e->erased ()
               << " doc=" << doc
               << endl;
#endif

    if (!vm || !doc)
        return;

    if (vm->queueUpdates ())
    {
        // OPT: if this update was due to the document,
        //      use document coordinates (in case of a zoom change in
        //      which view coordinates become out of date)
        addToQueuedArea (e->region ());
        return;
    }

    QRect viewRect = e->rect ().intersect (rect ());
    if (viewRect.isEmpty ())
        return;
    QRect docRect = paintEventGetDocRect (viewRect);

#if DEBUG_KP_VIEW && 1
    kdDebug () << "\tdocRect=" << docRect << endl;
#endif

// uncomment to cause deliberate flicker (identifies needless updates)
#if DEBUG_KP_VIEW && 0
    QPainter flickerPainter (this);
    flickerPainter.fillRect (viewRect, Qt::red);
    flickerPainter.end ();
#endif


    //
    // Prepare Back Buffer
    //

    if (!m_backBuffer ||
        m_backBuffer->width () < viewRect.width () ||
        m_backBuffer->height () < viewRect.height () ||
        m_backBuffer->width () > width () ||
        m_backBuffer->height () > height ())
    {
        // don't use QPixmap::resize() as that wastes time copying pixels
        // that will be overwritten anyway
        delete m_backBuffer;
        m_backBuffer = new QPixmap (viewRect.width (), viewRect.height ());
    }

// uncomment to catch bits of the view that the renderer forgot to update
#if 0
    m_backBuffer->fill (Qt::green);
#endif

    QPainter backBufferPainter;
    backBufferPainter.begin (m_backBuffer);


    //
    // Draw checkboard for transparent images and/or views with borders
    //

    QPixmap docPixmap;

    if (!docRect.isEmpty ())
        docPixmap = doc->getPixmapAt (docRect);

    if (docPixmap.mask () || m_needBorder)
    {
    #if DEBUG_KP_VIEW && 1
        kdDebug () << "\tmask=" << (bool) docPixmap.mask ()
                << " needBorder=" << m_needBorder
                << endl;
    #endif
        paintEventDrawCheckerBoard (&backBufferPainter, viewRect);
    }
    else
    {
    #if DEBUG_KP_VIEW && 1
        kdDebug () << "\tno mask" << endl;
    #endif
    }


    if (!docRect.isEmpty ())
    {
        //
        // Draw docPixmap + tempPixmap
        //

        if (vm->tempPixmapActive () &&
            (!vm->brushActive () || vm->shouldBrushBeDisplayed (this)) &&
            docRect.intersects (vm->tempPixmapRect ()))
        {
        #if DEBUG_KP_VIEW && 1
            kdDebug () << "\ttempPixmap: active"
                    << " rect=" << vm->tempPixmapRect ()
                    << " sel=" << vm->selectionActive ()
                    << endl;
        #endif
            paintEventDrawTempPixmap (&docPixmap, docRect);
        }

    #if DEBUG_KP_VIEW && 1
        kdDebug () << "\torigin=" << m_origin << endl;
    #endif
        // blit scaled version of docPixmap + tempPixmap onto Back Buffer
        backBufferPainter.translate (m_origin.x () - viewRect.x (),
                                    m_origin.y () - viewRect.y ());
        backBufferPainter.scale (double (m_hzoom) / 100.0,
                                double (m_vzoom) / 100.0);
        backBufferPainter.drawPixmap (docRect, docPixmap);
        backBufferPainter.resetXForm ();  // back to 1-1 scaling
    }  // if (!docRect.isEmpty ()) {


    //
    // Draw Grid Lines
    //

    if (m_showGrid && canShowGrid ())
        paintEventDrawGridLines (&backBufferPainter, viewRect);


    //
    // Blit Back Buffer to View
    //

    backBufferPainter.end ();

    bitBlt (this, viewRect.topLeft (),
            m_backBuffer, QRect (0, 0, viewRect.width (), viewRect.height ()));

#if DEBUG_KP_VIEW && 1
    kdDebug () << "\tall done in: " << timer.restart () << "ms" << endl;
#endif
}

#include <kpview.moc>
