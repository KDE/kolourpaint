
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

#ifndef __kpview_h__
#define __kpview_h__

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>

class QDragEnterEvent;
class QDropEvent;
class QFocusEvent;
class QKeyEvent;
class QRect;
class kpTool;
class kpMainWindow;

class kpView : public QWidget
{
Q_OBJECT

public:
    kpView (QWidget *parent, const char *name,
                kpMainWindow *mainWindow,
                int width, int height,
                bool autoVariableZoom = false);
    virtual ~kpView ();

    bool hasVariableZoom () const;

    // all incompatible with autoVariableZoom
    bool setZoomLevel (int hzoom, int vzoom);
    void showGrid (bool yes = true);
    bool canShowGrid (int hzoom = -1, int vzoom = -1) const;

    int zoomViewToDocX (int zoomedCoord) const;
    int zoomViewToDocY (int zoomedCoord) const;
    QPoint zoomViewToDoc (const QPoint &zoomedCoord) const;
    QRect zoomViewToDoc (const QRect &zoomedRect) const;

    int zoomDocToViewX (int doc_coord) const;
    int zoomDocToViewY (int doc_coord) const;
    QPoint zoomDocToView (const QPoint &doc_coord) const;
    QRect zoomDocToView (const QRect &doc_rect) const;

    virtual void resize (int w, int h);

public slots:
    // connect to document resize signal
    bool slotUpdateVariableZoom ();

private:
    bool updateVariableZoom (int viewWidth, int viewHeight);

    // event handlers;
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseMoveEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void keyReleaseEvent (QKeyEvent *e);
    virtual void focusInEvent (QFocusEvent *e);
    virtual void focusOutEvent (QFocusEvent *e);
    virtual void enterEvent (QEvent *e);
    virtual void leaveEvent (QEvent *e);
    virtual void paintEvent (QPaintEvent *e);

    void paint (const QPixmap &pixmap, const QRect &viewRect, const QRect &docRect);

    kpMainWindow *m_mainWindow;

    bool m_autoVariableZoom;
    int m_hzoom, m_vzoom;
    bool m_showGrid;
};

#endif  // __kpview_h__
