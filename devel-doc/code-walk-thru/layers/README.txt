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






Non-Rectangular Image Selection Borders
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rectangular image selections are straightforward since they have
easy-to-handle
rectangular borders.  In contrast, elliptical and freeform image selections
are difficult to implement.  The following methods must act consistently:

1. calculatePoints() - used to implement paintBorder().

2. shapeBitmap() - used by the "Selection / Clear" menu item.

3. shapeRegion() - used for pulling the selection image off the document
and contains().

This means that "Selection / Clear" must paint all pixels on, and inside,
the
border and none outside the border.  Similarly, a pull of the selection
image
must pull off those pixels on, and inside, the border and none outside.
Otherwise, selection borders would be visually misleading.



Moving Selections
-----------------

Hides the selection border during the move.  The text cursor must be
disabled.






Temp Pixmaps
~~~~~~~~~~~~

Should be merged with selections and future layer class.
