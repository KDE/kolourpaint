
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


#ifndef __kpview_h__
#define __kpview_h__

#include <qwidget.h>
#include <qregion.h>
#include <qwmatrix.h>


class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QFocusEvent;
class QKeyEvent;
class QPixmap;
class QPoint;
class QRect;
class QRegion;
class QSize;

class kpDocument;
class kpSelection;
class kpTool;
class kpMainWindow;
class kpViewManager;


class kpView : public QWidget
{
Q_OBJECT

public:
    kpView (QWidget *parent, const char *name,
                kpMainWindow *mainWindow,
                int width, int height,
                bool autoVariableZoom = false);
    virtual ~kpView ();

private:
    void setHasMouse (bool yes = true);

public:
    kpViewManager *viewManager () const;
    kpDocument *document () const;
    kpSelection *selection () const;
    bool hasVariableZoom () const;

    // all incompatible with autoVariableZoom
    int zoomLevelX (void) const;
    int zoomLevelY (void) const;
    bool setZoomLevel (int hzoom, int vzoom);
    void showGrid (bool yes = true);
    bool canShowGrid (int hzoom = -1, int vzoom = -1) const;

    // TODO: rename zoom*To* to transform*To* since we also do origin transformation
    int zoomViewToDocX (int zoomedCoord) const;
    int zoomViewToDocY (int zoomedCoord) const;
    QPoint zoomViewToDoc (const QPoint &zoomedCoord) const;
    QRect zoomViewToDoc (const QRect &zoomedRect) const;

    int zoomDocToViewX (int doc_coord) const;
    int zoomDocToViewY (int doc_coord) const;
    QPoint zoomDocToView (const QPoint &doc_coord) const;
    QRect zoomDocToView (const QRect &doc_rect) const;

    virtual void resize (int w, int h);
    virtual void resizeEvent (QResizeEvent *e);

    // (for kpViewManager)
    void addToQueuedArea (const QRect &rect);
    void addToQueuedArea (const QRegion &region);
    void invalidateQueuedArea ();
    void updateQueuedArea ();

signals:
    void sizeChanged (int width, int height);
    void sizeChanged (const QSize &size);


public:
    QRect selectionViewRect () const;

    QPoint mouseViewPoint () const;
    QPoint mouseViewPointRelativeToSelection () const;
    bool mouseOnSelection () const;

    int textSelectionMoveBorderAtomicSize () const;
    bool mouseOnSelectionToMove () const;

protected:
    bool selectionLargeEnoughToHaveResizeHandlesIfAtomicSize (int atomicSize) const;
public:
    int selectionResizeHandleAtomicSize () const;
    bool selectionLargeEnoughToHaveResizeHandles () const;

    QRegion selectionResizeHandlesViewRegion () const;

    enum SelectionResizeType
    {
        None = 0,
        Left = 1,
        Right = 2,
        Top = 4,
        Bottom = 8
    };

    // Returns a bitwise OR of the SelectionResizeType's
    int mouseOnSelectionResizeHandle () const;

    bool mouseOnSelectionToSelectText () const;


public slots:
    // connect to document resize signal
    bool slotUpdateVariableZoom ();

private:
    bool updateVariableZoom (int viewWidth, int viewHeight);

    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseMoveEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void keyReleaseEvent (QKeyEvent *e);
    virtual void focusInEvent (QFocusEvent *e);
    virtual void focusOutEvent (QFocusEvent *e);
    virtual void enterEvent (QEvent *e);
    virtual void leaveEvent (QEvent *e);
    virtual void dragEnterEvent (QDragEnterEvent *);
    virtual void dragLeaveEvent (QDragLeaveEvent *);

    kpMainWindow *m_mainWindow;

    bool m_autoVariableZoom;
    int m_hzoom, m_vzoom;
    QWMatrix m_docToViewMatrix;
    bool m_showGrid;

    QRegion m_queuedUpdateArea;

private:
    QRect paintEventGetDocRect (const QRect &viewRect) const;
    void paintEventDrawCheckerBoard (QPainter *painter, const QRect &viewRect);
    void paintEventDrawSelection (QPixmap *destPixmap, const QRect &docRect);
    bool selectionResizeHandleAtomicSizeCloseToZoomLevel () const;
    void paintEventDrawSelectionResizeHandles (QPainter *painter, const QRect &viewRect);
    void paintEventDrawTempPixmap (QPixmap *destPixmap, const QRect &docRect);
    void paintEventDrawGridLines (QPainter *painter, const QRect &viewRect);

    virtual void paintEvent (QPaintEvent *e);

    QPixmap *m_backBuffer;
    QPoint m_origin;
    bool m_needBorder;
};

#endif  // __kpview_h__
