
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

#include <qbitmap.h>
#include <qcursor.h>
#include <qpainter.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kptoolselection.h>
#include <kpviewmanager.h>

#define DEBUG_KPTOOLSELECTION 1

kpToolSelection::kpToolSelection (kpMainWindow *mainWindow)
    : kpTool (QString::null, QString::null, mainWindow, "tool_selection_base_class")
{
    setMode (kpToolSelection::Rectangle);
}

kpToolSelection::~kpToolSelection ()
{
}

// virtual
void kpToolSelection::begin ()
{
}

// virtual
void kpToolSelection::end ()
{
    if (viewManager ()->selectionActive ())
    {
        // finalize - and put it down
        document ()->setPixmapAt (viewManager ()->tempPixmap (),
                                  viewManager ()->tempPixmapRect ().topLeft ());

        viewManager ()->invalidateTempPixmap ();
    }
}

// virtual
void kpToolSelection::beginDraw ()
{
    m_dragMoving = false;

    if (viewManager ()->selectionActive ())
    {
        QRect selectionRect = viewManager ()->tempPixmapRect ();

        if (selectionRect.contains (m_currentPoint)) // TODO: MASK
        {
            if (m_controlPressed)
            {
                document ()->setPixmapAt (viewManager ()->tempPixmap (),
                                          selectionRect.topLeft ());
            }

            m_startDragFromSelectionTopLeft = m_currentPoint - selectionRect.topLeft ();
            m_dragMoving = true;

            viewManager ()->setSelectionBorderType (kpViewManager::NoBorder);
        }
        else
        {
            // starting another selection - better end the current one
            document ()->setPixmapAt (viewManager ()->tempPixmap (),
                                      selectionRect.topLeft ());
        }
    }
}

// virtual
void kpToolSelection::hover (const QPoint &point)
{
    if (viewManager ()->selectionActive () && viewManager ()->tempPixmapRect ().contains (point))
        viewManager ()->setCursor (QCursor (Qt::SizeAllCursor));
    else
        viewManager ()->setCursor (QCursor (Qt::CrossCursor));
}

// virtual
void kpToolSelection::draw (const QPoint &thisPoint, const QPoint &lastPoint,
                                const QRect &normalizedRect)
{
    if (!viewManager ()->selectionActive ())
    {
    #if DEBUG_KPTOOLSELECTION
        kdDebug () << "kpToolSelection::draw() starting selection at " << normalizedRect.topLeft () << endl;
    #endif
        QPixmap pixmap = document ()->getPixmapAt (normalizedRect);

        if (m_mode == kpToolSelection::Ellipse)
        {
            QBitmap mask (pixmap.width (), pixmap.height ());
            mask.fill (Qt::color0);
            QPainter painter;
            painter.begin (&mask);
            painter.setBrush (Qt::color1);
            painter.drawEllipse (0, 0, normalizedRect.width (), normalizedRect.height ());
            painter.end ();
            pixmap.setMask (mask);
        }

        sendSelectionToViewManager (pixmap, normalizedRect);
    }
    else
    {
        if (m_dragMoving)
        {
        #if DEBUG_KPTOOLSELECTION
            kdDebug () << "kpToolSelection::draw() moving selection" << endl;
        #endif
            if (m_shiftPressed && lastPoint != QPoint (-1, -1))
                document ()->setPixmapAt (viewManager ()->tempPixmap (), lastPoint - m_startDragFromSelectionTopLeft);

            sendSelectionToViewManager (viewManager ()->tempPixmap (),
                                        m_currentPoint - m_startDragFromSelectionTopLeft,
                                        false/*no sel border*/);
        }
        else
        {
        #if DEBUG_KPTOOLSELECTION
            kdDebug () << "kpToolSelection::draw () resizing selection to " << normalizedRect << endl;
        #endif
            QPixmap pixmap = document ()->getPixmapAt (normalizedRect);

            if (m_mode == kpToolSelection::Ellipse)
            {
                QBitmap mask (pixmap.width (), pixmap.height ());
                mask.fill (Qt::color0);
                QPainter painter;
                painter.begin (&mask);
                painter.setBrush (Qt::color1);
                painter.drawEllipse (0, 0, normalizedRect.width (), normalizedRect.height ());
                painter.end ();
                pixmap.setMask (mask);
            }

            sendSelectionToViewManager (pixmap, normalizedRect);
        }
    }
}

// virtual
void kpToolSelection::cancelShape ()
{
}

// virtual
void kpToolSelection::endDraw (const QPoint &thisPoint, const QRect &normalizedRect)
{
    if (viewManager ()->selectionActive ())
    {
        // if we just created a new selection, we've "ripped off" part of the document, forming the selection
        if (!m_dragMoving)
        {
            QRect selectionRect = viewManager ()->tempPixmapRect ();
            QPixmap pixmap = viewManager ()->tempPixmap ();
            pixmap.fill (backgroundColor ());
            document ()->setPixmapAt (pixmap, selectionRect.topLeft ());
        }
        else
        {
            sendSelectionToViewManager (viewManager ()->tempPixmap (),
                                        viewManager ()->tempPixmapRect ());
        }
    }
}

// private
void kpToolSelection::sendSelectionToViewManager (const QPixmap &pixmap, const QRect &rect, bool showBorder)
{
    sendSelectionToViewManager (pixmap, rect.topLeft (), showBorder);
}

// private
void kpToolSelection::sendSelectionToViewManager (const QPixmap &pixmap, const QPoint &topLeft, bool showBorder)
{
    kpViewManager::SelectionBorderType selBorder = kpViewManager::NoBorder;

    if (showBorder)
    {
        if (m_mode == kpToolSelection::Rectangle)
            selBorder = kpViewManager::Rectangle;
        else if (m_mode == kpToolSelection::Ellipse)
            selBorder = kpViewManager::Ellipse;
        else if (m_mode == kpToolSelection::FreeForm)
            selBorder = kpViewManager::FreeForm;
    }

    viewManager ()->setTempPixmapAt (pixmap, topLeft,
                                     kpViewManager::SelectionPixmap, selBorder);
}

#include <kptoolselection.moc>
