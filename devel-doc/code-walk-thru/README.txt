
KolourPaint 4 BETA Development Documentation


1. Package Overview
2. Top-Level Files
3. Instance Diagram
4. Future Directions


1. Package Overview
~~~~~~~~~~~~~~~~~~~

0. /             - miscellaneous
1. commands/     - wraps changes to a document, to make them undoable and
                   redoable
2. compat/       - hacks around Qt and KDE library bugs (until they're fixed)
3. cursors/      - KolourPaint-specific mouse cursors
4. dialogs/      - KolourPaint-specific dialogs
5. document/     - document class
6. environments/ - facades to centralize interactions between major packages
7. generic/      - non-KolourPaint-specific code that can be moved out into
                   kdelibs
8. imagelib/     - document image data manipulation [SHOULD READ]
9. layers/       - selections and tools' "temp images" [SHOULD READ]
10. lgpl/        - allows reuse of LGPL code, while preventing license
                   infection
11. mainWindow/  - mainwindow class
12. pixmapfx/    - QPixmap manipulation and QPainter wrapper [MUST READ]
13. tools/       - tools
14. views/       - widgets for visualizing the document
15. widgets/     - KolourPaint-specific widgets

WARNING: If you are modifying KolourPaint, you MUST read the pixmapfx/
         documentation at least.


2. Top-Level Files
~~~~~~~~~~~~~~~~~~

These files didn't really belong elsewhere.


[>>>] kolourpaint.cpp

main() entry point.

Initializes KolourPaint and creates one kpMainWindow.
If files are specified on the command, one is created for each filename.


[>>>] kpDefs.h

Global #define's and config key names.


[>>>] kpThumbnail

Thumbnail window containing a kpThumbnailView.


[>>>] kpViewScrollableContainer

Like QScrollArea but provides:

  1. Drag scrolling

  2. Grips allowing drag-resizing of the main view


3. Instance Diagram
~~~~~~~~~~~~~~~~~~~

A quick overview of which classes might exist at a snapshot in time,
with their owners (owners are higher up in the tree):

                 kolourpaint.cpp
                       |
                       |
          +------------+------------+
          |            |            |
          |            |      kpMainWindow
    kpMainWindow       |
                  kpMainWindow
                       |
+----------+-----------+-----+-------++------------------+---------+
|          |                 |       | \_                |         |
|          |                 |       |   \__             |         |
|     kpDocument       kpViewManager |      \___    kpToolToolBar  |
|          |                 |       |          \        |         |
|          |                 | kpViewScrollable |        |         |
|   +------+------+          |    Container     |        |   kpColorToolBar
|   |             |          |       |          |        |         |
|   |             |          +-------+-------kpView  ]   |         |
|   |             |                                      |         |
| kpImage   kpAbstractSelection                          |         |
|                                                        |         |
|                     +__________________________________+         |
+-------+             |              /                             |
        |             |     +-------+-------+                      |
   kpCommandHistory   |     |       |       |           +____+____/
        |             |     |       |       |          /     |
  +-----+----+        |  kpTool  kpTool   kpTool       |     |
  |          |        |                                |     |
kpCommand  kpCommand  |                  kpDualColorButton   |
                      |                                      |
                      |                                  kpColorPalette
               kpToolWidgetBase                              |
               derived objects                       +-------+-------+
                                                     |               |
                                                     |          kpColorCells
                                         kpTransparentColorCell


kpMainWindow stores exactly 1 kpCommandHistory, kpToolToolBar and
kpColorToolBar.  It stores 1 kpDocument, 1 kpViewManager, and at least 1
kpView, if there is an open kpDocument (which can be new and untitled).  If
there is at least 1 kpView, one of them must be the "main view" inside the
kpViewScrollableContainer.

Both kpTool and kpCommand operate on the assumption that kpDocument, kpView
and kpViewManager are not 0.  kpTool and kpCommand must not be invoked if
those are 0.

kpDocument's carry an optional kpAbstractSelection, which may be 0.

kpCommand and kpDocument are not GUI classes.  They should not access the
kpMainWindow, kp*ToolBar or kpTool.

kpTool's create kpCommand's.


4. Future Directions
~~~~~~~~~~~~~~~~~~~~

See TODO.  The most important one is to rewrite the image library (see the
imagelib/ documentation).
