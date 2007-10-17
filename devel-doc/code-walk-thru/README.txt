Entry Point
~~~~~~~~~~~
kolourpaint.cpp:main() initializes KolourPaint and creates a whole bunch of
kpMainWindow's.


Main Window
~~~~~~~~~~~

Source code stored in mainwindow/ folder.  Is a monster-sized class.

Far, too often main window pointers are passed around everywhere (since
given a pointer to a main window, you can get a pointer to anything else).
Unfortunately, this means that tools and commands are tied to the GUI.
Worse still, we cannot have 2 main windows viewing the same document.
ACTION: Change tools to use a kpToolEnvironment interface and commands to
use a kpCommandEnvironment.

The kpmainwindow_statusbar.cpp tries very hard to ensure that the status
bar contains the correct text.  Far too many other KDE applications allow
some status bar text to be, for instance, wiped by a temporary notice and
never re-appear.  KolourPaint does this right.

ACTION: However, the implementation is too concrete and should be converted
into a general status bar update pattern that can be pushed into kdelibs.
Different objects, say the view, could indicate things that affect that
status bar, say the zoom level.


Views
~~~~~

The source code is stored in the views/ folder.  There can be 1 or more
views per kpDocument document.

ACTION: These classes are badly designed because subclasses limit
functionality instead of increasing it.  We need to tip the inheritance
hierarchy upside-down.

TODO: class diagram


Commands and Undo/Redo
~~~~~~~~~~~~~~~~~~~~~~

Changes must not be made to the document directly using the document class.
 You must encapsulate each change in a kpCommand so that it is undoable and
redoable.  Failure to do this results in inconsistent behaviour when
undoing and redoing.

Interestingly, the Color Picker is undo-able and redo-able even though it
does not affect the document.  This is because we want to protect the
user's hard work looking for a particular needle color in the image
haystack.  We may want to rethink this in the future as making something a
command has a nasty side effect with linear command histories, as we now
discuss.

Suppose, under KolourPaint's linear command history (which is similar to
Konqueror's Back / Forward buttons and KWrite's Undo/Redo), one has
performed the Line, Circle and Rectangle commands.  Undoing would remove
the rectangle.  Undoing again would remove the circle.  Undoing yet again
would remove the line.  However, it's possible to “undo the undo” and
return to the states before we undid anything.  If we now redo, we traverse
the command history in the opposite direction: a redo recreates the line, a
second redo recreates the circle and a third redo recreates the rectangle.

The problem is suppose we had pressed undo those 3 times.  If we were now
to do something that adds a command to the history e.g. draw a different
line, draw a polygon or use the Color Picker, the redo history is cleared.
It is not possible to redo the Line, Circle or Rectangle commands by
pressing any combination of undo and redo.  A tree-like command history
would solve this problem but would probably add quite some UI complexity
and memory footprint.

Many command classes are quite similar, so in the future, we might merge
some of the classes e.g. kpToolPolygonalCommand and
kpToolRectangularCommand.

TODO: kpCommand is similar to KCommand except for the size() method;
kpMacroCommand is similar to KMacroCommand; kpCommandHistory is similar to
KCommandHistory except for more sophisticated clipping based on size and a
hacky interface for accessing the commands in the history directly (TODO:
why?  It had something to do with undoing and redoing selections – we
should cover selections and undo history in detail).  Eventually, all this
should be pushed into kdelibs.

This section covers the external interface to command classes, usually
invoked by tool classes, and the internals so that you can understand the
implementation of a command class.

External
--------

In this section, we highlight the difference between command classes that
do not need "finalization" and those that do.

Some command classes are considered to be complete after construction, such
as the image effect and transform commands.

Other command classes (esp. the ones corresponding to tools) are
constructed at the start of a shape drag and methods are called to update
the command as the drag progresses e.g. kpToolFlowCommand for brushes, pens
and erasers.  Commonly, these classes have a finalize() method which must
be called at the end of the drag, before adding the command to the command
history or if the drag is cancelled, before deleting the command.

For instance, finalize() might crop the saved image to the dirty rectangle
or disable a timer.  Failure to call finalize() results in undefined
behaviour such as redoing not working (due to a saved image not be cropped)
or crashes (due to the timer firing when the drag is no longer active).
ACTION: this is bad class design and finalize() should be called
automatically.

Internal
--------

In this section, we describe the implementation of command classes derived
from the kpCommand base class.

name() should be the name of the corresponding tool e.g. “Line” or “Curve”.
 If the selection was involved, you should prepend “Selection: “ (note the
trailing space) to the name (e.g. “Selection: Reduce to Grayscale”) instead
of “Reduce to Grayscale” (which is for if the entire image was changed).

size() is used for the purposes of clipping the command history to keep
KolourPaint memory usage sane.

execute() and unexecute() are usually called by kpCommandHistory in
response to the user triggering the redo and undo actions, respectively.

execute() should store a copy of the area of the document that will be
modified and then actually modify the document e.g. draw the line.  Note
that this copy is not normally stored in the constructor to save memory:
consider the case after a user undoes a command – we simply do not need
that copy at the moment as the document has not been modified.

unexecute() should restore that area of the document and erase the copy, to
save memory.  If it is an invertible operation (e.g. Invert or Flip), you
can actually avoid storing a copy of the dirty area in execute() and simply
apply the operation again.  Although this is slower, we value memory rather
than performance, for a larger command history (see size() above).

In both execute() and unexecute(), for a command created by a non-drag
operation (e.g. Image effects), Qt::WaitCursor should be used as the
operation usually takes quite some time – see the  kpSetOverrideCursorSaver
class.  We don't use wait cursors for drag operations (e.g. lines) as they
are usually fast to render and the cursor flicker is distracting.  Of
course, this is not a hard rule.


Tools
~~~~~

The view [CITE] forwards events unchanged to the current tool.  The tool
events lifecycle is as follows:

All tools' constructors are called at program startup.  Each tool creates
its KAction.  Each tool is placed into the Tool Box [CITE].

When a tool is selected, its begin() method is called.  This displays the
option widgets [CITE] (and connects their signals to slots to track option
changes), changes the cursor, changes the statusbar message and performs
other tool-specific initialisation.  When a tool is deselected, its end()
method is called to do the reverse.  hasBegun() returns true in between
begin() and end().

globalDraw() is called when the user double clicks on the tool in the Tool
Box.  reselect() is called if the user clicks on an already-selected tool.

When a mouse button is pressed (TODO: keyboard shortcuts), hasBegunDraw()
returns true and m_mouseButton is set to 0 for the left button and 1 for
the right button.  This results in the idiom 1 – mouseButton () which
returns the mouse button _not_ being pressed.  m_shiftPressed,
m_controlPressed and m_altPressed reflect the state of the keyboard
modifiers.  m_startPoint contains the document coordinate of the mouse.
beginDraw() is called.  Generally, the left button draws in the
foregroundColor() and the right button draws in the backgroundColor() but
this is up to the tool.  The middle mouse button pastes text from the
clipboard.

TODO: maybe we want a table of the above properties rather than text
paragraph?

Immediately after beginDraw() is called, draw() will be called.  As long as
the mouse stays down, draw() is also called every time the mouse is
dragged, a keyboard modifier is changed or for any other update KolourPaint
feels like.  While m_startPoint will be unchanged from beginDraw(),
m_currentPoint will be modified to reflect the current document coordinate
of the mouse and m_currentViewPoint will be the current view coordinate.
m_lastPoint will be the value of m_startPoint on the previous call to
draw() or (-1,-1) if this is the first call to draw() after beginDraw().
Note that (-1,-1) is always an invalid document coordinate for a start
point (after all, how could you possibly click there?) but is valid for the
current point (you could drag to there). Note that these variables are also
valid during beginDraw().

For most tools, draw() will create a kpTempPixmap containing the shape
being drawn and send it to the view manager [CITE] to display as a
temporary layer on top of the document.

After releasing the mouse button, endDraw() is called, the drag is
considered complete and hasBegunDraw() returns false.  endDraw() should
erase the kpTempPixmap and add the command to the command history,
committing the shape-drawing operation.

Instead of releasing the mouse button that initiated the drag, the user may
press the other mouse button while the initiating mouse button is still
held down.  cancelShape() is called to cancel the operation – it should
erase the kpTempPixmap but not commit to the command history.

releasedAllButtons() is called when no mouse buttons are no longer held.
This is for updating the status bar message.  Consider the case where
beginDraw() has been called due to a left mouse press.  If the right mouse
button is then pressed, while still holding down the left, cancelShape()
will be called.  But after that, as long as at least one button remains
depressed, no matter what buttons are pressed or released, no drawing
methods are called.  Only after all buttons are released, will
releasedAllButtons() be called and the tool starts recognising events
again.

hover() is called when the mouse is moved over the view but differs from
draw(), in that hover() is only called outside of a draw operation i.e.
when hasBegunDraw() returns false and equivalently, when no buttons being
pressed.

A shape that is not constructed from a single drag (such as a polygon;
TODO: explain while selections use this) can use an extension of the
drawing “pattern”.  By implementing hasBegunShape(), beginShape() and
endShape().  This abstraction is necessary at the kpTool level to ensure
that endDraw() is called, if needed before endShape() and endShape() is
automatically called before end().  TODO: expand
TODO: statusbar message – when call?

setUserMessage() sets the string to be displayed as the first field in the
statusbar, which gives the user instructions on what to do next e.g.
“Curve: Drag out the start and end points” or “Curve: Left drag to set the
first control point or right click to finish”.  This can be empty but never
should be because instructions should always be available.

setUserShapePoints() can be passed 0, 1 and 2 points and this determines
what is displayed in the second field of the statusbar:
0 points: mouse is outside of all views, display nothing
1 point: mouse is moving inside view or drawing with a flow tool (e.g.
Pen), show that the currrent point
2 points: user is dragging out a shape from a start point to and end point,
show these 2 points separated by a dash e.g. the start and end points of a
line.  This automatically calls setUserShapeSize() with the calculated size
of the shape with these points (can have negative components for a left
and/or up drag).  You can disable this automatic call by disabling the
setSize argument.  ACTION: document setSize in .h

setUserShapeSize() is passed the size of the current drag (can have
negative components for a left and/or up drag) and is displayed in the
third field.  If this is KP_INVALID_SIZE, no drawing is happening and it
displays nothing.  Note that QSize() is not KP_INVALID_SIZE as QSize() is
actually a legal size (0x0; TODO: check).
TODO: keep track of color changes etc.
TODO: draw() not called at endDraw() - causes problems at least with
polygon


Selections
~~~~~~~~~~

KolourPaint's selections are similar to those in text editors and are a
primitive version of layers found in other image programs.  When a
selection is active, all editing must be done on the selection and not the
main image below (with the notable exception of being able to resize the
image below).  Before editing can continue on the image below, the
selection must either be deleted or pushed onto the image below.

kpselection.cpp contains the kpSelection data structure for selections.
Selections are per-document (kpDocument).  However, the rendered borders
are per-view-manager (kpViewManager) and code is also spread in the view
(kpView).  This original idea for splitting selection concerns between the
document and the view classes was so that the selection border would be the
same size regardless of the zoom level, as it would be expressed in view
pixels, not document pixels.  ACTION: We have yet to actually implement
this.

ACTION: group code together like an aspect.

Selections in KolourPaint result in amazingly complicated code because
selection commands are individually undoable and redoable.  For instance,
if you were to select an area of the image, move that selection, scale that
selection and then move that selection again, each of these 3 commands
would be in the command history and KolourPaint would remember the
selection border for each.  In contrast, with other paint programs, all 3
selection commands would be grouped into 1.  Worse still, undoing would
leave you in the state before selecting the area and redoing after that
would leave you in the state after the selection is pushed down, after
those 3 manipulations.  You do not have an active selection anymore – you
have lost the selection border.

KolourPaint selections can be either:
1.Shapes: rectangles, ellipses or polygons
2.Text (in a rectangular box)

ACTION: split kpSelection into subclasses for rectangular, elliptical
selections etc.
ACTION: somehow combine with kpTempPixmap, which is a per-view-manager
concept for storing the currently drawn non-selection shape (e.g. a line).
ACTION: generalise into a layer class.

Note that transparency in all types of selections means that the
transparent bits of the selection will not affect the part of the document
it is floating on top of (they paint on top of the document).  This is in
contrast to drawing tools (e.g. Line) which really set document pixels to
the transparent color (they set document pixels).

Shapes
------

Shape selections contain a number of document pixels inside and on the
border of the shape.  Note that some of these pixels may be transparent as
the document happened to be transparent there.  In contrast, all pixels
outside of the border are always set to transparent.

Now, consider as an example, a mask of an elliptical selection:

The opaque pixels of this mask are inside or on the border of the ellipse
and therefore refer to document pixels in the selection.  Importantly,
there is nothing saying that these documents pixels are either transparent
or opaque – an opaque portion of the mask does not mean an opaque document
pixel.

Similarly, the transparent pixels of this mask are outside of the ellipse
and therefore refer to transparent pixels (TODO: clarify that these latter
pixels are due to pulling behavior) inside the bounding rectangle of the
selection but outside of the selection shape.


Selection Transparency
----------------------

To complicate matters, selections have a concept of "selection
transparency" (class kpSelectionTransparency).  When we say a "transparent
selection", we mean that the selection's selection transparency is set to
transparent – we say nothing about the document pixels covered by the
selection.  Similarly, when we say an "opaque selection", we mean that the
selection's selection transparency is set to opaque – we say nothing about
the document pixels covered by the selection.

In an "opaque selection", selection transparency does nothing.  In a
"transparent selection", all pixels in the selection that are similar to
the current background color (CITE: see color similarity) are treated as
transparent.  This is why changes to the background color or color
similarity setting, while a selection is active, are undoable/redoable
commands.  This feature is used for primitive background subtraction – for
copying an object rendered against a fairly solid background.

The area of the document covered by the selection, _before_ selection
transparency is applied, is stored in kpSelection and accessible via the
pixmap() and opaquePixmap() methods.  The selection image, after any
selection transparency is applied, is accessible via the
transparentPixmap() method.  Copying the image to the clipboard, and all
image effect commands, operate on the image data before selection
transparency is applied (TODO: name a good reason why?).  However, pushing
the selection onto the image uses the the selection image data after
selection transparency is applied.

As an example, suppose:
1.There is currently a floating, rectangular selection of a red apple on a
blue background.
2.The background color is blue.
3.The selection transparency is opaque.

If we were to move this selection on top of a green area of the document,
we would have an apple on top of a striking blue area on top of the green.
If we were to now switch the selection transparency to transparent, we
would now have just the apple on top of the green.  All blue from the
selection would be removed.  If the apple happened to have any blue (maybe
someone dropped some blue paint on it), it too would be removed, resulting
in a hole in the apple.

If we were to deselect the selection, pushing it onto the document, we
would end up with this apple on top of the green.  However, if we were to
copy the selection to another program, the blue would still be in the image
data in the destination program.

If we were to now invert the selection, the blue would become yellow.  The
background of the selection would not be removed by selection transparency
as it is now yellow, not blue.  If we were to invert again or undo, the
background would revert to blue and it would be viewed as transparent again
due to selection transparency.

Lifecycle
---------

When the user drags to select an area of the document, the document is not
modified so the command history is not changed.  This is to allow the user
to selection to continually change the selected area without nuking the
redo history [CITE].  As the current selection is just a border, all
kpSelection image data accessors (pixmap(), opaquePixmap(),
transparentPixmap()) will return a null image.  Selection transparency has
no effect, yet.

In order to modify the contents of the selection (e.g. an image effect on a
selection or drag-moving the selection), the image data must be copied or
pulled from the document to form a "floating selection" that hovers as a
temporary layer on top of the document.  It is no longer just a border.
Floating selections' image data accessors will return non-null images.
Such an operation is considered to modify the document and is recorded in
the command history as a kpToolSelectionPullFromDocumentCommand (which
wraps a call to kpDocument::selectionPullFromDocument()).

Pulling from the document destructively leaves in the place of all opaque
pixels inside the selection (after applying selection transparency), the
background color (TODO: a good reason why).  However, if the pull is being
caused by a drag-move, the user can non-destructively pull from the
document by holding CTRL.  KolourPaint actually implements this by
destructively pulling and then gets kpToolSelectionMoveCommand to stamp the
transparent selection back onto the document, without destroying the
selection, by using kpDocument::selectionCopyOntoDocument().

Floating selections can be manipulated by:

* Image effect commands

* Several selection commands

  - kpToolSelectionMoveCommand for moving the selection around

  - kpToolSelectionResizeScaleCommand for scaling the selection (but not
    resizing – you can only resize text selections)
    kpToolSelectionTransparencyCommand for changing selection transparency
    parameters (opaque/transparent selection choice, background color and
    color similarity)

  - kpToolSelectionPushOntoDocumentCommand for destroying the selection by
    either throwing it away (using kpDocument::selectionDelete()) or by
    pushing its contents (after selection transparency is applied) onto the
    document (using kpDocument::selectionPushOntoDocument()).

Text
----

Text selections are rectangular boxes containing a list of text lines.  The
selection transparency field is ignored with text style (kpTextStyle)
taking its place.

Text Styles
...........

A text style contains the font family, size in points, color and effects
(bold, italic, underline, strike through).

Background color is also stored but is effectively transparent if selection
transparency is set to transparent.  In other words, a transparent
background can be achieved through either a transparent background color
or, if selection transparency is set to transparent, any color.  Changes to
the background color (TODO: and color similarity out of laziness?) are
undoable as they affect the selection.

Life Cycle
..........

When the user drags to create a text box, it instantly becomes floating and
this is considered to modify the document so is undoable.  There is no
pulling from the document to create a text box.    ACTION: simply dragging
out a text box should not modify the document – it's like dragging out a
selection border.

It is possible to add and remove text and each text operation is undoable.
However, the cursor position is not maintained in the command history
(ACTION: it would be neat to do so).  ACTION: There is currently no dynamic
wordwrap, unlike KWrite.

pixmap(), opaquePixmap() and transparentPixmap() always return a non-null
pixmap and they all return the same thing, since selection transparency is
ignored.  However, it is generally incorrect to use these methods for text
boxes with transparent backgrounds.  This is because text boxes are
floating and rendered on top of a document, which usually contains some
opaque content, permitting text antialiasing.  But these methods are not
aware of this underlying content so cannot antialias, giving a different –
and lower quality – result compared to what the user sees on screen.  Use
the paint() method, which accepts the background image data, instead.

For simplicity, it is not possible to convert to and from a shape selection
and a text selection.  As a result, you cannot apply image effects onto a
text selection.  You can resize a text box but not scale it.


tools/selection/ contains selection tools.

commands/tools/selection/ contains the commands used by those tools and
permit undo and redo of selection modifications.  This makes KolourPaint
enormously complex.


TODO: selection undo/redo in detail – use cases and deselecting a selection
if selecting a new selection border; restoring selection transparency
settings (e.g. background color) on undo/redo and keeping them in sync with
selections


Temp Pixmaps
~~~~~~~~~~~~

Should be merged with selections and future layer class.
