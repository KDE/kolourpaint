Commands [commands/]


Overview
~~~~~~~~

Commands wrap changes to a document, to make them undoable and redoable.

These classes are a clone of the KDE KCommand hierarchy with these extra
features required by KolourPaint:

- Methods for peeking the undo/redo history
- Undo/Redo history limited by both number and size (in bytes) of stored

These class are an implementation of the Command Design Pattern.


Foundation Classes
~~~~~~~~~~~~~~~~~~

kpCommandSize - static methods that estimate the sizes of objects in bytes.
-------------

This is used by the command history to trim stored commands, once a
certain amount of memory is used by those commands.

kpCommandSize::SizeType is a large integer typedef for commands sizes,
which is not expected to overlow.


kpCommand - abstract base class for all commands
---------

size() must be implemented to return the estimated size of all fields in
the command, in bytes.  kpCommandSize methods can help here.

execute() must be implemented to run a command for the first time and also
to redo.

unexecute() must be implemented for undo.


kpNamedCommand - abstract convenience base class for commands
--------------

This is the same as kpCommand except that name() is implemented to return
the name passed to the constructor.


kpCommandHistoryBase

kpCommandHistory


Interactions
~~~~~~~~~~~~

kpCommandSize methods are generally only called inside implementations of
kpCommand::size().

kpCommand, kpNamedCommand and kpMacroCommand are referenced extensively
outside this module.  There is a substantial hierarchy of classes derived
from these base classes, for each operation that acts on the document.
Implementations of execute() and unexecute() should mutate the kpDocument
returned by document() and control views using the kpViewManager returned
by viewManager().  In order to reduce coupling with classes outside of the
package, accesses to such classes should be done in kpCommandEnvironment
only.

execute() and unexecute() may be called without a command being in
the command history.

Commands are generally created, and added to the command history, by tools
or by kpMainWindow.  Sometimes they are deleted and never placed in the
command history as, for instance, a tool may construct a kpCommand while
the user is dragging a shape but discards it when the user cancels that
shape.

A single kpCommandHistory is stored per kpMainWindow and accessible
through the user-visible undo/redo actions.


Derived Classes
~~~~~~~~~~~~~~~


Additional Interactions
~~~~~~~~~~~~~~~~~~~~~~~

imagelib commands use imagelib












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



