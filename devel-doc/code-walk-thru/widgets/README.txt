
widgets/ Package


KolourPaint-specific widgets.


1. widgets/
2. widgets/colorSimilarity/
3. widgets/imagelib/effects/
4. widgets/toolbars/
5. widgets/toolbars/options/


1. widgets/
~~~~~~~~~~~

[>>>] kpColorCells

KolourPaint-specific subclass of the generic kpColorCellBase widget.
All non-KolourPaint-specific functionality should go into kpColorCellsBase.

This widget consists of rows of 11 cells of colors.  The cells become
shorter as soon as there are 3 rows.  After that, a vertical scrollbar
is usually displayed.

It stores a kpColorCollection and handles the modified state and
reading to / writing from a KUrl.

By default, it is set to kpDefaultColorCollection, with 2 rows.

Included in kpColorPalette.


[>>>] kpColorPalette

Contains the kpTransparentColorCell and the kpColorCells.

Included in kpColorToolBar.


[>>>] kpDefaultColorCollection

The default set of colors offered by KolourPaint, via kpColorCells, to the
user.  Not actually a widget.

It contains all of the ordinary colors (black, white, gray, colors of
the rainbow etc.) and a few others.

Subclass of the generic kpColorCollection.


[>>>] kpDocumentSaveOptionsWidget

Widget added, by kpMainWindow, to a saving file dialog for configuring
color depth or quality (currently, mutually exclusive).

Returns a kpDocumentSaveOptions.


[>>>] kpDualColorButton

Widget similar to KDualColorButton.  Shows the current foreground and
background drawing color.

Included in kpColorToolBar.


[>>>] kpPrintDialogPage

Tab that can be found in the "File / Print..." dialog for configuring
whether to print the image at the top-left or in the center.


[>>>] kpTransparentColorCell

A clickable widget for selectin theg transparent color.  Included in
kpColorPalette.


2. widgets/colorSimilarity/
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Widgets related to the "Color Similarity" feature.


[>>>] kpColorSimilarityCubeRenderer

Class with static methods (and no fields) for painting a 3D cube, that
visualizes the current strength setting of the color similarity feature.

Used by kpColorSimilarityFrame and kpColorSimilarityToolBarItem.

Not actually a widget.


[>>>] kpColorSimilarityFrame

Frame showing the cube rendered by kpColorSimilarityCubeRenderer.

Included in the kpColorSimilarityDialog.


[>>>] kpColorSimilarityHolder

Container for a floating point (C++ "double") color similarity strength
setting.  Contains color similarity constants.

Not actually a widget.


[>>>] kpColorSimilarityToolBarItem

Included in kpColorToolBar as a button showing the cube rendered by
kpColorSimilarityCubeRenderer.

When clicked, opens kpColorSimilarityDialog to configure the color
similarity strenght.

Is flashed every time a tool uses the color similarity feature to draw
the user's attention to the existence of the feature.  See
kpToolEnvironment::flashColorSimilarityToolBarItem()'s API Doc for a
full explanation.


3. widgets/imagelib/effects/
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Widgets for configuring the parameters of the top-level imagelib/effects
and displayed in the kpEffectsDialog.

kpEffectWidgetBase is the superclass of all the other classes and contains
the pure virtual createCommand() method for creating an instance of the
respective subclass of kpEffectCommandBase.


4. widgets/toolbars/
~~~~~~~~~~~~~~~~~~~~

Two KolourPaint-specific toolbars.  See also kolourpaint4ui.rc in the
top-level (for e.g. the Text Toolbar).


[>>>] kpColorToolBar

Toolbar normally placed horizontally at the bottom of the main window,
containing the kpDualColorButton, kpColorPalette and
kpcolorSimilarityToolBarItem.

Included in kpMainWindow.


[>>>] kpToolToolBar

Tool Box normally placed at the left of the main window, containing
the buttons for all kpTool's.  Stores the current tool and tool option
widgets (widgets/toolbar/options/).  Handles tool changes.

Included in kpMainWindow.


5. widgets/toolbars/options/
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Widgets for configuring the options for certain tools, located underneath
all the tool buttons in kpToolToolBar and shown/hidden depending on the
tool.  Options include line width and brush size.

kpToolWidgetBase is the superclass of all the other classes.  It maps
each option to a pixmap visualization (e.g. a square brush or a
circular brush).  It handles layout of these pixmaps within each option
widget (it contains overly complicated and fragile code for doing this,
which is why we currently fix all option widgets to be 44x66).  It stores
the currently selected option.