
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
