
Views
~~~~~

The source code is stored in the views/ folder.  There can be 1 or more
views per kpDocument document.

ACTION: These classes are badly designed because subclasses limit
functionality instead of increasing it.  We need to tip the inheritance
hierarchy upside-down.

TODO: class diagram





View depends on existence of document - for simplicty.
For instance, views pass on mouse events to tools which assume existence.
If you destroy document, you must kill view first.
setDocument() kills view and doc and creates doc and view.

Exception is kpView::deleteLater():
r385274 | dang | 2005-02-02 22:08:27 +1100 (Wed, 02 Feb 2005) | 21 lines

* kpView: fix crash on access to deleted kpDocument in paintEvent()
          due to kpMainWindow::setDocument(0) calling
          "kpThumbnailView->deleteLater()" - thumbnailView was alive
          even after document had died.  Used QGuardedPtr.
          Crash can be reproduced by
          opening a document, switching to brush tool, opening thumbnail,
          zooming to 800%, then CTRL+W (close).
          This only goes to show that deleteLater(),
QTimer::singleShot(0,...)
          and other event loop tricks only cause trouble.

which is why kpView uses a guardded pointer to document.


