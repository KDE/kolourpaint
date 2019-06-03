
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


#ifndef KP_VIEW_H
#define KP_VIEW_H


#include <QWidget>

#include "kpDefs.h"


class QDragEnterEvent;
class QDragLeaveEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

class kpAbstractSelection;
class kpDocument;
class kpTextSelection;
class kpTool;
class kpToolToolBar;
class kpViewManager;
class kpViewScrollableContainer;


/**
 * @short Abstract base class for all views.
 *
 * This is the abstract base class for all views.  A view is a widget that
 * renders a possibly zoomed onscreen representation of a document.
 *
 * 1 view corresponds to 1 document.
 * 1 document corresponds to 0 or more views.
 *
 * @see kpViewManager
 *
 * @author Clarence Dang <dang@kde.org>
 */
class kpView : public QWidget
{
Q_OBJECT

public:
    /**
     * Constructs a view.
     *
     * @param document The document this view is representing.
     * @param toolToolBar The tool tool bar.
     * @param viewManager The view manager.
     * @param buddyView The view this view watches over (e.g. a thumbnail
     *                  view would watch over the main view).  May be 0.
     *                  See for example, highlightBuddyViewRectangle().
     * @param scrollableContainer This view's scrollable container.
     *                            May be 0.
     *
     * You must call adjustEnvironment() at the end of your constructor.
     */
    kpView (kpDocument *document,
            kpToolToolBar *toolToolBar,
            kpViewManager *viewManager,
            kpView *buddyView,
            kpViewScrollableContainer *scrollableContainer,
            QWidget *parent);

    /**
     * Destructs this view.  Informs the viewManager() that the mouse
     * cursor is no longer above this view.
     */
    ~kpView () override;


    //
    // Constants (enforced by methods)
    //
    static const int MinZoomLevel, MaxZoomLevel;


    /**
     * @returns the document.
     */
    kpDocument *document () const;

protected:
    /**
     * @returns the document's selection.
     */
    kpAbstractSelection *selection () const;

    kpTextSelection *textSelection () const;

public:
    /**
     * @returns the tool tool bar.
     */
    kpToolToolBar *toolToolBar () const;

protected:
    /**
     * @returns the currently selected tool.
     */
    kpTool *tool () const;

public:
    /**
     * @returns the view manager.
     */
    kpViewManager *viewManager () const;

    /**
     * @returns the buddy view.
     */
    kpView *buddyView () const;

    /**
     * @returns the buddyView()'s scrollable container.
     */
    kpViewScrollableContainer *buddyViewScrollableContainer () const;

    /**
     * @returns this view's scrollable container.
     */
    kpViewScrollableContainer *scrollableContainer () const;


    /**
     * @returns the horizontal zoom level (100 is unzoomed).
     */
    int zoomLevelX (void) const;

    /**
     * @returns the vertical zoom level (100 is unzoomed).
     */
    int zoomLevelY (void) const;

    /**
     * Sets the horizontal and vertical zoom levels.
     *
     * @param hzoom Horizontal zoom level.
     * @param vzoom Vertical zoom level.
     *
     * This method automatically bounds <hzoom> and <vzoom> to be between
     * MinZoomLevel and MaxZoomLevel inclusive.
     *
     * If reimplementing, you must call this base implementation.
     */
    virtual void setZoomLevel (int hzoom, int vzoom);


    /**
     * @returns in views coordinates, where the top-left document() pixel
     *          will be rendered (default: (0,0)).
     */
    QPoint origin () const;

    /**
     * Sets the origin.
     *
     * @param origin New origin.
     *
     * If reimplementing, you must call this base implementation.
     */
    virtual void setOrigin (const QPoint &origin);


    /**
     * @returns whether at this zoom level, the grid can be enabled.
     *          This is based on whether the grid can be sensibly rendered.
     */
    bool canShowGrid () const;

    /**
     * @returns whether the grid is currently shown.
     */
    bool isGridShown () const;

    /**
     * Turns on/off the grid.
     *
     * @param yes Whether to enable the grid.
     */
    void showGrid (bool yes = true);


    /**
     * @returns whether to draw a rectangle highlighting the area of
     *          buddyView() visible through buddyViewScrollableContainer().
     */
    bool isBuddyViewScrollableContainerRectangleShown () const;

    /**
     * Turns on/off the rectangle highlighting the area of buddyView()
     * visible through buddyViewScrollableContainer() and redraws.
     *
     * @param yes Whether to turn on the rectangle.
     */
    void showBuddyViewScrollableContainerRectangle (bool yes = true);

protected:
    /**
     * @returns the current rectangle highlighting the area of buddyView()
     *          visible through buddyViewScrollableContainer(), that is being
     *          rendered by this view.
     */
    QRect buddyViewScrollableContainerRectangle () const;

protected slots:
    /**
     * Updates the buddyViewScrollableContainerRectangle() and redraws
     * appropriately.
     *
     * This is already connected to zoomLevelChanged() and originChanged();
     * buddyView() and buddyViewScrollableContainer() signals.  There is probably no
     * need to call this function directly.
     */
    void updateBuddyViewScrollableContainerRectangle ();


public:

    /**
     * @param viewX Horizontal position in view coordinates.
     *
     * @returns viewX transformed to document coordinates, based on the
     *                origin() and zoomLevelX().
     */
    double transformViewToDocX (double viewX) const;

    /**
     * @param viewY Vertical position in view coordinates.
     *
     * @returns viewY transformed to document coordinates, based on the
     *                origin() and zoomLevelY().
     */
    double transformViewToDocY (double viewY) const;

    /**
     * @param viewPoint Position in view coordinates.
     *
     * @returns viewPoint transformed to document coordinates, based on the
     *                    origin(), zoomLevelX() and zoomLevelY().
     */
    QPoint transformViewToDoc (const QPoint &viewPoint) const;

    /**
     * @param viewRect Rectangle in view coordinates.
     *
     * @returns viewRect transformed to document coordinates based on the
     *                   origin(), zoomLevelX() and zoomLevelY().
     *
     * For bounding rectangles, you should use this function instead of
     * transformViewToDocX(), transformViewToDocY() or
     * transformViewToDoc(const QPoint &) which act on coordinates only.
     */
    QRect transformViewToDoc (const QRect &viewRect) const;


    /**
     * @param docX Horizontal position in document coordinates.
     *
     * @returns docX transformed to view coordinates, based on the origin()
     *               and zoomLevelX().
     */
    double transformDocToViewX (double docX) const;

    /**
     * @param docY Vertical position in document coordinates.
     *
     * @returns docY transformed to view coordinates, based on the origin()
     *               and zoomLevelY().
     */
    double transformDocToViewY (double docY) const;

    /**
     * @param docPoint Position in document coordinates.
     *
     * @returns docPoint transformed to view coordinates, based on the
     *                   origin(), zoomLevelX(), zoomLevelY().
     */
    QPoint transformDocToView (const QPoint &docPoint) const;

    /**
     * @param docRect Rectangle in document coordinates.
     *
     * @return docRect transformed to view coordinates, based on the
     *                 origin(), zoomLevelX() and zoomLevelY().
     *
     * For bounding rectangles, you should use this function instead of
     * transformDocToViewX(), transformDocToViewY() or
     * transformDocToView(const QPoint &) which act on coordinates only.
     */
    QRect transformDocToView (const QRect &docRect) const;


    /**
     * @param viewPoint Position in view coordinates.
     * @param otherView View whose coordinate system the return value will
     *                  be in.
     *
     * @returns viewPoint transformed to the coordinate system of
     *          @param otherView based on this and otherView's origin(),
     *          zoomLevelX() and zoomLevelY().  This has less rounding
     *          error than otherView->transformDocToView (transformViewToDoc (viewPoint)).
     */
    QPoint transformViewToOtherView (const QPoint &viewPoint,
                                     const kpView *otherView);


    /**
     * @returns the approximate view width required to display the entire
     *          document(), based on the zoom level only.
     */
    int zoomedDocWidth () const;

    /**
     * @returns the approximate view height required to display the entire
     *          document(), based on the zoom level only.
     */
    int zoomedDocHeight () const;


protected:
    /**
     * Updates the viewManager() on whether or not the mouse is directly
     * above this view.  Among other things, this ensures the brush cursor
     * is updated.
     *
     * This should be called in event handlers.
     *
     * @param yes Whether the mouse is directly above this view.
     */
    void setHasMouse (bool yes = true);


public:
    /**
     * Adds a region (in view coordinates) to the dirty area that is
     * repainted when the parent @ref kpViewManager is set not to queue
     * updates.
     *
     * @param region Region (in view coordinates) that needs repainting.
     */
    void addToQueuedArea (const QRegion &region);

    /**
     * Convenience function.  Same as above.
     *
     * Adds a rectangle (in view coordinates) to the dirty area that is
     * repainted when the parent @ref kpViewManager is set not to queue
     * updates.
     *
     * @param rect Rectangle (in view coordinates) that needs repainting.
     */
    void addToQueuedArea (const QRect &rect);

    /**
     * Removes the dirty region that has been queued for updating.
     * Does not update the view.
     */
    void invalidateQueuedArea ();

    /**
     * Updates the part of the view described by dirty region and then
     * calls invalidateQueuedArea().  Does nothing if @ref kpViewManager
     * is set to queue updates.
     */
    void updateQueuedArea ();

    QVariant inputMethodQuery (Qt::InputMethodQuery query) const override;

public slots:
    /**
     * Call this when the "environment" (e.g. document size) changes.  The
     * environment is defined by the caller and should be based on the type
     * of view.  For instance, an unzoomed thumbnail view would also
     * include in its environment the contents position of an associated
     * scrollable container.
     *
     * This is never called by the kpView base class.
     *
     * Implementors should change whatever state is necessary for their
     * type of view.  For instance, an unzoomed view would resize itself;
     * a zoomed thumbnail would change the zoom level.
     */
    virtual void adjustToEnvironment () = 0;


public:
    // If <returnViewPoint> is not KP_INVALID_POINT, it spits it back.
    // Else, it returns the current mouse position in view coordinates.
    // REFACTOR: Seems like a bad API.
    QPoint mouseViewPoint (const QPoint &returnViewPoint = KP_INVALID_POINT) const;


signals:
    /**
     * Emitted after all zooming code has been executed.
     *
     * @param zoomLevelX New zoomLevelX()
     * @param zoomLevelY New zoomLevelY()
     */
    void zoomLevelChanged (int zoomLevelX, int zoomLevelY);

    /**
     * Emitted after all resizing code has been executed.
     *
     * @param size New view size.
     */
    void sizeChanged (const QSize &size);

    /**
     * Convenience signal - same as above.
     *
     * Emitted after all resizing code has been executed.
     *
     * @param width New view width.
     * @param height New view height.
     */
    void sizeChanged (int width, int height);

    /**
     * Emitted after all origin changing code has been executed.
     *
     * @param origin The new origin.
     */
    void originChanged (const QPoint &origin);



//
// Selections
//

public:
    QRect selectionViewRect () const;

    // (if <viewPoint> is KP_INVALID_POINT, it uses QCursor::pos())

    QPoint mouseViewPointRelativeToSelection (const QPoint &viewPoint = KP_INVALID_POINT) const;
    bool mouseOnSelection (const QPoint &viewPoint = KP_INVALID_POINT) const;

    int textSelectionMoveBorderAtomicSize () const;
    bool mouseOnSelectionToMove (const QPoint &viewPoint = KP_INVALID_POINT) const;

protected:
    bool selectionLargeEnoughToHaveResizeHandlesIfAtomicSize (int atomicSize) const;
public:
    int selectionResizeHandleAtomicSize () const;
    bool selectionLargeEnoughToHaveResizeHandles () const;

    QRegion selectionResizeHandlesViewRegion (bool forRenderer = false) const;

    enum SelectionResizeType
    {
        None = 0,
        Left = 1,
        Right = 2,
        Top = 4,
        Bottom = 8
    };

    // Returns a bitwise OR of the SelectionResizeType's
    int mouseOnSelectionResizeHandle (const QPoint &viewPoint = KP_INVALID_POINT) const;

    bool mouseOnSelectionToSelectText (const QPoint &viewPoint = KP_INVALID_POINT) const;


//
// Events
//

protected:
    void mouseMoveEvent (QMouseEvent *e) override;
    void mousePressEvent (QMouseEvent *e) override;
    void mouseReleaseEvent (QMouseEvent *e) override;
public:
    // (needs to be public as it may also get event from
    //  QScrollView::contentsWheelEvent())
    void wheelEvent (QWheelEvent *e) override;


protected:
    void keyPressEvent (QKeyEvent *e) override;
    void keyReleaseEvent (QKeyEvent *e) override;

protected:
    void inputMethodEvent (QInputMethodEvent *e) override;

protected:
    bool event (QEvent *e) override;


protected:
    void focusInEvent (QFocusEvent *e) override;
    void focusOutEvent (QFocusEvent *e) override;


protected:
    void enterEvent (QEvent *e) override;
    void leaveEvent (QEvent *e) override;


protected:
    void dragEnterEvent (QDragEnterEvent *) override;
    void dragLeaveEvent (QDragLeaveEvent *) override;


protected:
    void resizeEvent (QResizeEvent *e) override;


//
// Painting
//

protected:
    // Returns the document rectangle that, when scaled to the view,
    // is "guaranteed" to at least cover <viewRect> and possibly more
    // ("guaranteed" in quotes because it doesn't seem so reliable for
    // zoom levels that aren't multiples of 100%).
    QRect paintEventGetDocRect (const QRect &viewRect) const;
public:
    /**
     * Draws an opaque background representing transparency.
     *
     * Currently, it draws a checkerboard which, if it were to be drawn
     * in its entirety, is tiled from <patternOrigin>.
     *
     * @param painter Painter.
     * @param patternOrigin Logical top-left point of the checkerboard,
     *                      relative to the painter, if it were to be
     *                      drawn in its entirety.
     * @param viewRect Rectangle to paint in relative to the painter.
     * @param isPreview Whether the view is for a preview as opposed to
     *                  e.g. editing.  If set, this function may render
     *                  slightly differently.
     */
    static void drawTransparentBackground (QPainter *painter,
                                           const QPoint &patternOrigin,
                                           const QRect &viewRect,
                                           bool isPreview = false);
protected:
    // Draws a checkerboard that looks static even if the view is scrollable.
    void paintEventDrawCheckerBoard (QPainter *painter,
        const QRect &viewRect);

    // Draws the selection and its border onto <destPixmap>.
    // <destPixmap> is the part of the document given by <docRect>.
    void paintEventDrawSelection (QImage *destPixmap, const QRect &docRect);

    // Draws the parts of the selection's resize handles that are inside
    // <clipRect> onto the view
    void paintEventDrawSelectionResizeHandles (const QRect &clipRect);
    void paintEventDrawTempImage (QImage *destPixmap, const QRect &docRect);

    // Draws the parts of the grid lines that are inside <viewRect> on
    // <painter>.
    void paintEventDrawGridLines (QPainter *painter, const QRect &viewRect);

    void paintEventDrawDoc_Unclipped (const QRect &viewRect);
    void paintEvent (QPaintEvent *e) override;


private:
    struct kpViewPrivate *d;
};


#endif  // KP_VIEW_H
