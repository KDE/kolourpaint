
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

#include <math.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptool.h>
#include <kptoolpen.h>
#include <kpview.h>
#include <kpviewmanager.h>


#define DEBUG_KPVIEW 1

kpView::kpView (QWidget *parent, const char *name,
                    kpMainWindow *mainWindow,
                    int width, int height,
                    bool autoVariableZoom)
    : QWidget (parent, name, Qt::WStaticContents | Qt::WNoAutoErase /* no flicker */),
      m_mainWindow (mainWindow),
      m_autoVariableZoom (autoVariableZoom),
      m_hzoom (100), m_vzoom (100),
      m_showGrid (false)
{
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
    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;
    
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
}


/*
 * Zoom/Grid
 */

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
#if DEBUG_KPVIEW
    kdDebug () << "kpView::updateVariableZoom (" << viewWidth << "," << viewHeight << ")" << endl;
#endif

    if (!hasVariableZoom ())
    {
        kdError () << "kpView::slotUupdateVariableZoom() called with !hasVariableZoom" << endl;
        return false;
    }

    if (m_mainWindow->document ()->width () > viewWidth)
        m_hzoom = viewWidth * 100 / m_mainWindow->document ()->width ();
    else
        m_hzoom = 100;

    if (m_mainWindow->document ()->height () > viewHeight)
        m_vzoom = viewHeight * 100 / m_mainWindow->document ()->height ();
    else
        m_vzoom = 100;

    // keep aspect ratio
    if (m_hzoom < m_vzoom)
        m_vzoom = m_hzoom;
    else
        m_hzoom = m_vzoom;

    update ();
    return true;
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

    resize (zoomDocToViewX (m_mainWindow->document ()->width ()),
            zoomDocToViewY (m_mainWindow->document ()->height ()));

    return true;
}

void kpView::showGrid (bool yes)
{
    if (m_showGrid == yes)
        return;

    m_showGrid = yes;
    update ();
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
    return zoomedCoord * 100 / m_hzoom;
}

int kpView::zoomViewToDocY (int zoomedCoord) const
{
    return zoomedCoord * 100 / m_vzoom;
}

QPoint kpView::zoomViewToDoc (const QPoint &zoomedCoord) const
{
    return QPoint (zoomViewToDocX (zoomedCoord.x ()),
                   zoomViewToDocY (zoomedCoord.y ()));
}

QRect kpView::zoomViewToDoc (const QRect &zoomedRect) const
{
    return QRect (zoomViewToDoc (zoomedRect.topLeft ()),
                  zoomViewToDoc (zoomedRect.bottomRight ()));
}

/*
 * doc->view
 */

// view_coord = ceil (doc_coord * zoom / 100)
int kpView::zoomDocToViewX (int doc_coord) const
{
    if (m_hzoom % 100 == 0)
        return doc_coord * m_hzoom / 100;
    else
        return (int) ceil (double (doc_coord) * double (m_hzoom) / 100.0);
}

int kpView::zoomDocToViewY (int doc_coord) const
{
    if (m_vzoom % 100 == 0)
        return doc_coord * m_vzoom / 100;
    else
        return (int) ceil (double (doc_coord) * double (m_vzoom) / 100.0);
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

        int width = zoomDocToViewX (doc_rect.width ());
        int height = zoomDocToViewY (doc_rect.height ());

        // can't just zoom the top-left & bottom-right coords
        // because when zoom is > 100, we have to fill entire
        // boxes representing individual pixels
        return QRect (topLeft.x (), topLeft.y (), width, height);
    }
}

// virtual
void kpView::resize (int w, int h)
{
    QWidget::resize (w, h);
    update ();  // OPT
}


/*
 * Event Handlers
 */

// virtual
void kpView::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KPVIEW && 0
    kdDebug () << "kpView::mousePressEvent (" << e->x () << "," << e->y () << ")" << endl;
#endif

    setHasMouse (true);
    m_mainWindow->tool ()->mousePressEvent (e);

    e->accept ();
}

// virtual
void kpView::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KPVIEW && 0
    kdDebug () << "kpView::mouseMoveEvent (" << e->x () << "," << e->y () << ")" << endl;
#endif

    setHasMouse (true);
    m_mainWindow->tool ()->mouseMoveEvent (e);

    e->accept ();
}

// virtual
void kpView::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KPVIEW && 0
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
    kdDebug () << "kpView::focusInEvent()" << endl;
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
#if DEBUG_KPVIEW
    kdDebug () << "kpView::enterEvent" << endl;
#endif

    setHasMouse (true);
    m_mainWindow->tool ()->enterEvent (e);
}

// virtual
void kpView::leaveEvent (QEvent *e)
{
#if DEBUG_KPVIEW
    kdDebug () << "kpView::leaveEvent" << endl;
#endif

    setHasMouse (false);
    m_mainWindow->tool ()->leaveEvent (e);
}

// private virtual
void kpView::dragEnterEvent (QDragEnterEvent *)
{
#if DEBUG_KPVIEW
    kdDebug () << "kpView::dragEnterEvent" << endl;
#endif

    setHasMouse (true);
}

// private virtual
void kpView::dragLeaveEvent (QDragLeaveEvent *)
{
#if DEBUG_KPVIEW
    kdDebug () << "kpView::dragLeaveEvent" << endl;
#endif

    setHasMouse (false);
}

// virtual
void kpView::paintEvent (QPaintEvent *e)
{
    QRect rect = e->rect ();
    QRect docRect = zoomViewToDoc (rect);

#if DEBUG_KPVIEW && 0
    kdDebug () << "kpView::paintEvent (" << rect.x () << "," << rect.y () << ")"
               << " (w=" << rect.width () << ",h=" << rect.height () << ")"
               << endl;
#endif

    QPainter painter;

    bool m_tempPixmapActive = m_mainWindow->viewManager ()->tempPixmapActive ();
    QRect m_tempPixmapRect = m_mainWindow->viewManager ()->tempPixmapRect ();
    QPixmap m_tempPixmap = m_mainWindow->viewManager ()->tempPixmap ();
    bool selActive = m_mainWindow->viewManager ()->selectionActive ();

    if (m_tempPixmapActive && docRect == m_tempPixmapRect && !selActive)
    {
        paint (m_tempPixmap, rect, m_tempPixmapRect);
    }
    else
    {
        // get doc image
        kpDocument *doc = m_mainWindow->document ();
        QPixmap pixmap = doc->getPixmapAt (docRect);

        // add temp image
        if (m_tempPixmapActive && docRect.intersects (m_tempPixmapRect))
        {
            QPainter painter (&pixmap);
            painter.drawPixmap (m_tempPixmapRect.topLeft () - docRect.topLeft (), m_tempPixmap);

            if (selActive)
            {
                enum kpViewManager::SelectionBorderType type = m_mainWindow->viewManager ()->selectionBorderType ();

                if (type == kpViewManager::Rectangle)
                {
                #if DEBUG_KPVIEW
                    kdDebug () << "\tselection border = rectangle" << endl;
                #endif
                    painter.setRasterOp (Qt::XorROP);
                    painter.setPen (QPen (Qt::white, 1, Qt::DotLine));
                #if DEBUG_KPVIEW && 1
                    kdDebug () << "\t\tx=" << m_tempPixmapRect.x () - docRect.x ()
                               << " y=" << m_tempPixmapRect.y () - docRect.y ()
                               << " w=" << m_tempPixmapRect.width ()
                               << " h=" << m_tempPixmapRect.height ()
                               << endl;
                #endif
                    painter.drawRect (m_tempPixmapRect.x () - docRect.x (),
                                      m_tempPixmapRect.y () - docRect.y (),
                                      m_tempPixmapRect.width (), m_tempPixmap.height ());
                    painter.end ();
                }
                else if (type == kpViewManager::Ellipse)
                {
                #if DEBUG_KPVIEW
                    kdDebug () << "\tselection border = ellipse" << endl;
                #endif
                    painter.setRasterOp (Qt::XorROP);
                    painter.setPen (QPen (Qt::white, 1, Qt::DotLine));
                    painter.drawEllipse (m_tempPixmapRect.x () - docRect.x (),
                                         m_tempPixmapRect.y () - docRect.y (),
                                         m_tempPixmapRect.width (), m_tempPixmap.height ());
                    painter.end ();
                }
                else if (type == kpViewManager::FreeForm)
                {
                #if DEBUG_KPVIEW
                    kdDebug () << "\tselection border = freeForm" << endl;
                #endif
                }
                else
                {
                #if DEBUG_KPVIEW
                    kdDebug () << "\tselection border = none" << endl;
                #endif
                }
            }
        }

        paint (pixmap, rect, docRect);
    }
}

// private
void kpView::paint (const QPixmap &pixmap, const QRect &viewRect, const QRect &docRect)
{
    QPainter painter (this);

    // blt the pixmap
    painter.scale (double (m_hzoom) / 100.0, double (m_vzoom) / 100.0);
    painter.drawPixmap (docRect, pixmap);

    // display grid
    // TODO: flicker - dbl-buffer too slow
    if (m_showGrid && canShowGrid ())
    {
        int hzoomMultiple = m_hzoom / 100;
        int vzoomMultiple = m_vzoom / 100;

        painter.resetXForm ();  // back to 1-1 scaling

        QPen ordinaryPen (Qt::gray), tileBoundaryPen (Qt::lightGray);

        painter.setPen (ordinaryPen);

        // horizontal lines
        int starty = viewRect.top ();
        if (starty % vzoomMultiple)
            starty = (starty + vzoomMultiple) / vzoomMultiple * vzoomMultiple;
        int tileHeight = 16 * vzoomMultiple;  // CONFIG
        for (int y = starty; y <= viewRect.bottom (); y += vzoomMultiple)
        {
            if (tileHeight > 0 && y % tileHeight == 0)
            {
                painter.setPen (tileBoundaryPen);
                //painter.setRasterOp (Qt::XorROP);
            }

            painter.drawLine (viewRect.left (), y, viewRect.right (), y);

            if (tileHeight > 0 && y % tileHeight == 0)
            {
                painter.setPen (ordinaryPen);
                //painter.setRasterOp (Qt::CopyROP);
            }
        }

        // vertical lines
        int startx = viewRect.left ();
        if (startx % hzoomMultiple)
            startx = (startx + hzoomMultiple) / hzoomMultiple * hzoomMultiple;
        int tileWidth = 16 * hzoomMultiple;  // CONFIG
        for (int x = startx; x <= viewRect.right (); x += hzoomMultiple)
        {
            if (tileWidth > 0 && x % tileWidth == 0)
            {
                painter.setPen (tileBoundaryPen);
                //painter.setRasterOp (Qt::XorROP);
            }

            painter.drawLine (x, viewRect.top (), x, viewRect.bottom ());

            if (tileWidth > 0 && x % tileWidth == 0)
            {
                painter.setPen (ordinaryPen);
                //painter.setRasterOp (Qt::CopyROP);
            }
        }
    }

    painter.end ();
}

#include <kpview.moc>
