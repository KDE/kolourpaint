
generic/ Package


Non-KolourPaint-specific code that can be moved out into kdelibs.


1. generic/
2. generic/widgets/


1. generic/
~~~~~~~~~~~

[>>>] kpAbstractScrollAreaUtils

Estimating the sizes of scrollbars in a QAbstractSCcollArea.


[>>>] kpSetOverrideCursorSaver

A less error-prone way of setting the override cursor, compared to
the QApplication::{set,restore}OverrideCursor() pair.


[>>>] kpWidgetMapper

Maps from global screen coordinates to the coordinate system of a
widget and vice-versa.


2. generic/widgets/
~~~~~~~~~~~~~~~~~~~

Non-KolourPaint-specific version of the top-level widgets/ folder.


[>>>] kpResizeSignallingLabel

Same as QLabel but emits the resized() signal when it's resized.


[>>>] kpSqueezedTextLabel

KSqueezedTextLabel done properly -- squeeze at the end of the string,
not the middle.

In KDE 4, the functionality might now be offered
by KSqueezedTextLabel::setTextElideMode().

Used by the statusbar code in kpMainWindow.


[>>>] kpSubWindow

A tool window with the following properties, in order of importance:

1. Stays on top of its parent window, but not on top of any other
   window (this rules out Qt::WindowStaysOnTop)
2. Does not auto-hide when its parent window loses focus
   (this rules out Qt::Tool).
3. Does not have a taskbar entry.

We mean "tool window" in the window system sense.  It has nothing
to do with kpTool so to avoid confusion, we do not name it "kpToolWindow".

Used by kpThumbnail and kpDocumentSaveOptionsPreviewDialog.
