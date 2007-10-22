
views/ Package.


Widgets for visualizing kpDocument.


1. Inheritance Hierarchy
2. views/
3. views/manager/


1. Inheritance Hierarchy
~~~~~~~~~~~~~~~~~~~~~~~~

                  kpView
                  ^    ^
                 /      \
               /          \
             /              \
    kpZoomedView         kpThumbnailView
                             ^    ^
                            /      \
                          /          \
                        /              \
      kpUnzoomedThumbnailView    kpZoomedThumbnailView



2. views/
~~~~~~~~~

[>>>] kpView

Abstract base class for all views.

A view is a widget that renders a possibly zoomed onscreen representation
of a kpDocument.

1 view corresponds to 1 document.
1 document corresponds to 0 or more views.

Views are created by kpMainWindow.  kpViewManager signals each view to
update.  External classes generally use the interface provided by kpView
and rarely need to use subclass-specific interfaces.

kpView can also:

1. Display contents offset by an origin
2. Display a grid
3. Display a "thumbnail rectangle"
4. Transform to-and-from document coordinates

View events (e.g. mouse press) are forwarded to the currently selected
kpTool.

All the painting code is located in the kpView base, not the subclasses.

Each view depends on the existence of the document -- for simplicity.  If
you want to destroy the kpDocument it's observing, you must destroy the
view first.  Due to deleteLater() being used on kpView, this deletion
order is briefly violated -- hence the need for QPointer in kpView.


[>>>] kpZoomedView

Concrete zoomed view of a document, used as the main view in KolourPaint.

It resizes according to the size of the document and the zoom level.

Derived from kpView.  Created by kpMainWindow::setDocument().


[>>>] kpThumbnailView

Abstract base class for all thumbnail views.  Derived from kpView.

Created by kpMainWindow and placed into a kpThumbnail window.


[>>>] kpUnzoomedThumbnailView

Concrete unzoomed thumbnail view of a document, dervied from
kpThumbnailView.  Unlike kpZoomedThumbnailView, it never changes the zoom
level.  And unlike kpZoomedView, it never resizes itself.  Instead, it
changes its origin according to the main view's scrollable container so
that the top-left most document pixel displayed in the kpMainWindow's
kpViewScrollableContainer will be visible.


[>>>] kpZoomedThumbnailView

Concrete zoomed thumbnail view of a document, derived from
kpThumbnailView.  Unlike kpZoomedView, it never resizes itself.  Instead,
it changes its zoom level to accommodate the display of entire document
in the view, while maintaining aspect.


3. views/manager/
~~~~~~~~~~~~~~~~~

[>>>] kpViewManager

1 kpViewManager is created by each kpMainWindow and stores pointers to
kpView's, each created by that kpMainWindow.

Functionality includes:

1. Determining which view the mouse cursor is under.

2. Setting the mouse cursor shape for all views.

3. Keeping track of the current kpTempImage, as set by kpTool's and
   queried by the kpView painting code.

4. If a kpSelection is active, keeping track of the appearance of
   the selection border, as set by kpTool's and queried by kpView
   painting code.  Only a maximum of 1 of kpSelection and kpTempImage
   are allowed to exist simultaneously.

5. If a kpTextSelection is active, keeping track of the artificial text
   cursor and periodically asking the views to update it.

6. Controls the timing and speed of updates.  See setQueueUpdates() and
   setFastUpdates().

7. Updates arbitrary regions in all of the views.
   kpMainWindow::setDocument() connects kpDocument's contentsChanged()
   signal to kpViewManager's updateViews(), so that all document changes
   are automatically reflected in the view.  Consecutive
   document modifications should be wrapped with a
   setQueueUpdates()/restoreQueueUpdates() block for efficiency.
