
dialogs/ Package


KolourPaint-specific dialogs.


1. dialogs/
2. dialogs/imagelib/
4. dialogs/imagelib/effects/
3. dialogs/imagelib/transforms/


1. dialogs/
~~~~~~~~~~~

[>>>] kpColorSimilarityDialog

Dialog for configuring the current color similarity strength.  Invoked
from kpColorSimilarityToolBarItem.

Contains a spinbox for configuring the strength and a
kpColorSimilarityFrame, for visualizing the new strength.


[>>>] kpDocumentSaveOptionsPreviewDialog

Displays a scaled-down preview image and estimated file size for images to
be saved.  Can be resized.

Is enabled by the "Preview" button in saving file dialogs, created by
kpMainWindow.


2. dialogs/imagelib/
~~~~~~~~~~~~~~~~~~~~

Dialogs related to similarly named classes in the top-level imagelib/
folder.


[>>>] kpDocumentMetaInfoDialog

Dialog for editing document meta information (see kpDocumentMetaInfo).
It contains:

    1. DPI spinboxes
    2. Offset spinboxes
    3. Text Fields

Created by kpMainWindow as the "File / Properties..." dialog.


3. dialogs/imagelib/effects/
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[>>>] kpEffectsDialog

The "Image / More Effects..." dialog, created by kpMainWindow.

Creates instances of a number of kpEffectWidgetBase subclasses.

Subclass of kpTransformPreviewDialog.


4. dialogs/imagelib/transforms/
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[>>>] kpTransformPreviewDialog

Base class for effects and transforms dialogs, displaying the result of the
effects/transforms -- as an optional preview and an optional dimensions
display.  Subclasses add a custom widget for configuring the parameters
the effect/transform.


[>>>] kpTransformFlipDialog

"Image / Flip..." dialog created by kpMainWindow.


[>>>] kpTransformResizeScaleDialog

"Image" / "Resize / Scale ..." dialog created by kpMainWindow.

Subclass of kpTransformPreviewDialog.


[>>>] kpTransformRotateDialog

"Image / Rotate..." dialog created by kpMainWindow.

Subclass of kpTransformPreviewDialog.


[>>>] kpTransformSkewDialog

"Image / Skew..." dialog created by kpMainWindow.

Subclass of kpTransformPreviewDialog.
