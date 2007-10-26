
layers/ package


1. tempImage/

2. selections/
2.1. Class Diagram
2.2. Introduction
2.3. Image Selections
2.3.1. Internals
2.3.2. Image Selection Transparency
2.3.3. Lifecycle
2.4. Text Selections
2.4.1. Text Styles
2.4.2. Life Cycle
2.5. More on the Selection Lifecycle
2.5.1. Border Creation Command
2.5.2. Tricky Undo/Redo Situations


1. tempImage/
~~~~~~~~~~~~~

Most tools have the shape, currently being drawn, layered above the
document as a temporary image, hence the name kpTempImage.  In other words,
the document does not contain the pixels of that shape.  When the user
ends the shape, it is pasted down onto the kpDocument.

It supports 3 modes:

  1. SetImage: Uses kpPixmapFX::setImageAt() to overwrite pixels of the
               document in the view.

  2. PaintImage: Uses kpPixmapFX::paintImageAt().

  3. UserFunction: Arbitrary rendering of the kpTempImag.e

This is managed by kpViewManager.


2. selections/
~~~~~~~~~~~~~~

2.1. Class Diagram
------------------

                                 kpAbstractSelection
                                 ^                 ^
                                /                   \
                              /                       \
                            /                           \
                          /                               \
                 kpAbstractImageSelection           kpTextSelection
                 ^           ^  ^       \                 |
                /            |   \       \                | has
              /              |     \      \ has           |
            /                |       \     \          kpTextStyle
 kpRectangularImageSelection |        \     \
                             |         |   kpImageSelectionTransparency
          kpEllipticalImageSelection   |
                                       |
                           kpFreeFormImageSelection


This is missing kpSelectionDrag and kpSelectionFactory.


2.2. Introduction
-----------------

KolourPaint's selections are similar to those in text editors and are a
primitive version of layers found in other image programs.  When a
selection is active, all editing must be done on the selection and not the
main image below (with the notable exception of being able to resize the
image below).  Before editing can continue on the image below, the
selection must either be deleted or pushed onto the image below.

Selections are per-document (kpDocument) and stored in kpDocument.  However,
the rendered borders are per-view-manager (kpViewManager) and code is also
spread in the view (kpView).  This original idea for splitting selection
concerns between the document and the view classes was so that the selection
border would be the same size regardless of the zoom level, as it would be
expressed in view pixels, not document pixels.  However, we have yet to
actually implement this.

Selections in KolourPaint result in amazingly complicated code because
selection commands are individually undoable and redoable.  For instance,
if you were to select an area of the image, move that selection, scale that
selection and then move that selection again, each of these 3 commands
would be in the command history and KolourPaint would remember the
selection border for each.  In contrast, with other paint programs, all 3
selection commands would be grouped into 1.  Worse still in those programs,
undoing would leave you in the state before selecting the area and redoing
after that would leave you in the state after the selection is pushed down,
after those 3 manipulations.  You would not have an active selection anymore –
you have lost the selection border.

KolourPaint selections can be either:

  1. Image: rectangles, ellipses or polygons

  2. Text (in a rectangular box)

Note that transparency in all types of selections means that the
transparent bits of the selection will not affect the part of the document
it is floating on top of (they paint on top of the document, like
QPainter::CompositionMode_SourceOver). This is in contrast to drawing tools
(e.g. Line) which really set document pixels to the transparent color (they
set document pixels, like QPainter::CompositionMode_Source).


2.3. Image Selections
---------------------

Image selections contain a number of document pixels inside and on the
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
and therefore refer to transparent pixels inside the bounding rectangle of the
selection but outside of the selection shape.


2.3.1. Internals
~~~~~~~~~~~~~~~~

Rectangular image selections are straightforward since they have
easy-to-handle rectangular borders.  In contrast, elliptical and freeform
image selections are difficult to implement.  The following methods must act
consistently:

1. calculatePoints() - used to implement paintBorder().

2. shapeBitmap() - used by the "Selection / Clear" menu item.

3. shapeRegion() - used for pulling the selection image off the document
                   and for contains().

For instance, "Selection / Clear" must paint all pixels on, and inside, the
border and none outside the border.  Similarly, a pull of the selection
image must pull off those pixels on, and inside, the border and none outside.
Otherwise, selection borders would be visually misleading.

This is difficult to do esp. under Qt4.


2.3.2. Image Selection Transparency
...................................

To complicate matters, image selections have a concept of "selection
transparency" (class kpImageSelectionTransparency).  When we say a
"transparent selection", we mean that the selection's selection transparency
is set to transparent – we say nothing about the document pixels covered by
the selection.  Similarly, when we say an "opaque selection", we mean that the
selection's selection transparency is set to opaque – we say nothing about
the document pixels covered by the selection.

In an "opaque selection", selection transparency does nothing.  In a
"transparent selection", all pixels in the selection that are similar to
the current background color (see Color Similarity in the imagelib/
documentation) are treated as transparent.  This is why changes to the
background color or color similarity setting, while a selection is active, are
undoable/redoable commands.  This feature is used for primitive background
subtraction – for copying an object rendered against a fairly solid
background.

The area of the document covered by the selection, _before_ selection
transparency is applied, is stored in kpAbstractImageSelection and accessible
via the baseImage() method.  The selection image, after any selection
transparency is applied, is accessible via the transparentPixmap() method.
Copying the image to the clipboard, and all image effect commands, operate on
the image data before selection transparency is applied.  However, pushing the
selection onto the image uses the the selection image data after
selection transparency is applied.

As an example, suppose:

  1. There is currently a floating, rectangular selection of a red apple on a
     blue background.
  2. The background color is blue.
  3. The selection transparency is opaque.

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


2.3.3. Lifecycle
................

When the user drags to select an area of the document, the document is not
modified so the command history is not changed.  This is to allow the user
to continually change the selected area without nuking the redo history.  As
the current selection is just a border, all kpSelection image data accessors
(baseImage() and transparentImage()) will return a null image.  Selection
transparency has no effect, yet.

In order to modify the contents of the selection (e.g. an image effect on a
selection or drag-moving the selection), the image data _must_ be copied or
pulled from the document to form a "floating selection" that hovers as a
temporary layer on top of the document.  Afer this, it is no longer just a
border.  Floating selections' image data accessors will return non-null
images.  Such an operation is considered to modify the document and is
recorded in the command history as a kpToolSelectionPullFromDocumentCommand
(which wraps a call to kpDocument::imageSelectionPullFromDocument()).

*** The above pulling from the document is mandatory before manipulating
    an image selection. ***

Pulling from the document destructively leaves in the place of all opaque
pixels inside the selection (after applying selection transparency), the
background color.  However, if the pull is being caused by a drag-move, the
user can non-destructively pull from the document by holding CTRL.
KolourPaint actually implements this by destructively pulling and then gets
kpToolSelectionMoveCommand to stamp the transparent selection back onto the
document, without destroying the selection, by using
kpDocument::selectionCopyOntoDocument().

Floating selections can be manipulated by:

  * Image effect commands

  * Several selection commands

    - kpToolSelectionMoveCommand for moving the selection around

    - kpToolSelectionResizeScaleCommand for scaling the selection (but not
      resizing – you can only resize text selections)

    - kpToolImageSelectionTransparencyCommand for changing selection
      transparency parameters (opaque/transparent selection choice, background
      color and color similarity)

    - kpToolSelectionDestroyCommand for destroying the selection by
      either deleting it or by pushing its contents (after selection
      transparency is applied) onto the document (using
      kpDocument::selectionPushOntoDocument()).

If the document has a selection that is just a border, you can just delete
it using kpDocument::selectionDelete(), as that need not be undoable.
However, if it is floating (has been pulled from the document), you _must_ use
kpToolSelectionDestoryCommand to either delete it or to push it onto
the document.  Otherwise, the undo/redo history will be very confusing.

**** The above is very important. ****


2.4. Text Selections
--------------------

Text selections are rectangular boxes containing a list of text lines.
Instead of selection transparency, there is text style (kpTextStyle).


2.4.1. Text Styles
...................

A text style contains the font family, size in points, color and effects
(bold, italic, underline, strike through).

Background color is also stored but is effectively transparent if selection
transparency is set to transparent.  In other words, a transparent
background can be achieved through either a transparent background color
or, if selection transparency is set to transparent, any color.

Changing text style parameters is an undoable operation
(kpToolTextChangeStyleCommand).


2.4.2. Life Cycle
.................

When the user drags to create a text box, it is, like with image
selections, just a border.

Before modifying the text box (e.g. adding text or drag-moving the text
box), you must give the text box "content" (i.e. mark it as editable / make
it a "floating" selection) using kpToolTextGiveContentCommand.  This is unlike
KolourPaint/KDE3, where merely dragging to create a text box added a command
to the command history and the box was editable immediately.

*** The above "giving of content" is mandatory before manipulating a
    text selection. ***

There are commands to add and remove text and each text operation is
undoable.  There is currently no dynamic wordwrap, unlike KWrite.

For simplicity, it is not possible to convert to and from an image selection
and a text selection.  As a result, you cannot apply image effects onto a
text selection.  You can resize a text box but not scale it (the opposite is
true for an image selection).

kpTextSelection has the same requirement as kpAbstractImageSelection with
regards to the use of kpToolSelectionDestroyCommand.


2.5. More on the Selection Lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2.5.1. Border Creation Command
..............................

It was previously stated that for both image and text selections, dragging
out a border does not count as a command, but that making a selection floating
(by using kpToolSelectionPullFromDocumentCommand
or kpToolTextGiveContentCommand) counts as a command.

However, the truth is more complicated.  Let us suppose that the user has
created out a rectangular image selection border and then drag-moved that.
A kpMacroCommand, consisting of kpToolSelectionPullFromDocumentCommand
and kpToolSelectionMoveCommand, is certainly added to the command history.
But just before that, KolourPaint actually adds that border creation as a
kpToolSelectionCreateCommand.

Let us now suppose that the user presses Undo till just before the border was
created.  Now, if the user presses Redo, s/he gets the border back, thanks to
the redoing of kpToolSelectionCreateCommand, in the state _before_ the move
command has executed.  Reproducing the border in this fashion is
convenient since it is sometimes hard to drag it out again exactly (esp. if
it's a freeform selection border) and this allows to user to easily do a
different operation (e.g. scale) with this border.

It's actually slightly more complicated than this -- see
kpCommandHistory::addCreateSelectionCommand().


2.5.2. Tricky Undo/Redo Situations
..................................

As mentioned previously, the lifecycle of a floating selection, is started by
kpToolSelectionCreateCommand and usually (but not always e.g. when pasting an
image), kpToolSelectionPullFromDocumentCommand or
kpToolTextGiveContentCommand.

Since the selection is no longer a border, _all_ changes to the selection
are recorded as commands.  The only way to get rid of the selection is to
record another command -- kpToolSelectionDestroyCommand.  Both
kpToolSelectionCreateCommand and kpToolSelectionDestroyCommand ensure
that the main window's settings (e.g. background color) match the selection
being created or destroyed

During the lifecycle of the floating selection, _all_ mutations most be
recorded as commands.  This includes changes to the selection
transparency or text style, and such commands will synchronize the main
window's settings.  Even if a change in selection transparency or text
style (e.g. background color) has no immediate effect on the selection, it
_must_ still be recorded as a command -- see
kpAbstractImageSelectionTool::changeImageSelectionTransparency() for a
discussion.
