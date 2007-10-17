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

