
lgpl/ Package


LGPL code -- compiled into in a dynamic library -- used by the main BSD
KolourPaint.  This allows reuse of LGPL code, while preventing license
infection.

For instance, if a KDE class cannot be subclassed to give the required
behavior, a copy of the class is put in here and modified.


1. lgpl/
2. lgpl/generic/
3. lgpl/generic/widgets/


1. lpgl/
~~~~~~~~

[>>>] kolourpaint_lgpl_export.h

Provides the "KOLOURPAINT_LGPL_EXPORT" macro for specifying symbol
visibility at link time.  Must be used by all class declarations in this
package.


2. lpgl/generic/
~~~~~~~~~~~~~~~~

LGPL version of the top-level generic/ folder.  Like generic/, non
KolourPaint-specific code only.


[>>>] kpColorCellection

Fork of KColorCollection -- essentially, a container for a list of
QColor's, which can be read and write from and to a file.

Stored by kpColorPalette as the current color palette.


[>>>] kpUrlFormatter

Pretty-prints URLs to QString.


3. lgpl/generic/widgets/
~~~~~~~~~~~~~~~~~~~~~~~~

LGPL version of the top-level generic/widgets/ folder.


[>>>] kpColorCellsBase

Fork of KColorCells -- "a table [widget] of editable color cells.".
Non-KolourPaint-specific functionality is put in here.

KolourPaint-specific functionality is put in kpColorCells, a subclass of
this.