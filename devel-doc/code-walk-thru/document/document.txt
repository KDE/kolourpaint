
document/ Package


[>>>] kpDocument

Class holding:

1. Image data ("kpImage")
2. Meta information for the image ("kpDocumentMetaInfo")
3. URL for where the document came from (empty for an untitled document)
4. Save options, such as file format ("kpDocumentSaveOptions")
5. Modified state
6. The current selection, if any ("kpAbstractSelection")

Functionality includes:

1. Opening from and saving to URLs
2. Returning the whole kpImage or returning a rectangular region of the
   kpImage as a smaller kpImage
3. Changing the whole kpImage or changing a rectangular region of the
   kpImage.  This is the method that kpCommand's and kpTool's use to
   change the image data.
4. Selection functionality:
   - Getting/setting the selection
   - Getting the pixels in the selected image region
   - Pulling selected image region off the document and pushing down a
     selection onto the document
5. Signalling when the document (including the image data) changes.
   kpMainWindow::setDocument() connects the contentsChanged() signal to
   kpViewManager, so that changes to the document are automatically
   reflected in the kpView's

Created by kpMainWindow::setDocument() and usually connected to a
kpViewManager.  Is observed by 0 or more views but is usually observed by
1-2 (the main view and possibly, the thumbnail).

If setting the selection, you must inform kpViewManager of the intended
appearance of the selection border.  If setting a text selection, you must
also specific text cursor parameters (such as the position within the text
lines).


[>>>] kpDocumentSaveOptions

Options for saving the kpDocument, stored in kpDocument and
expressed in the GUI as in the saving file dialog (created by kpMainWindow)
and kpDocumentSaveOptionsWidget.  These are:

1. Mimetype
2. Color Depth
3. Whether to dither if the specified Color Depth is lower than that of
   kpDocument's kpImage image data
4. Lossy Compression Quality (for e.g. JPEG)

kpDocument will set the detected Mimetype and Color Depth when
opening an image.  It will also set the "Dither" option to false.

kpDocument's kpImage image data is stored at the current screen
depth (usually 24-bit), due to the flawed design of the image library (see
the documentation for imagelib/ package).  The normal intention of the
"Dither" option is for users who are saving the image data (of a new
document) at a lower color depth and who wish to increase the quality of
the reduction.  Users may prefer not to enable the option because they
don't want the image to get more grainy with altered pixel colors.

Now, let us suppose that instead of drawing a new document, the user opens
an 8-bit document.  This is converted to a kpImage, which is 24-bit,
without
any pixel colors being changed (you actually lose the 8-bit palette
though).  So that if the user were to press "Save", the document will be
saved as an 8-bit image, the "Color Depth" field of
kpDocumentSaveOptionsWidget is set by kpDocument to 8-bit.  Since the image
data came from an 8-bit image in the first place, the reduction from the
24-bit kpImage back down to 8-bit will be lossless as there will not be
more than 256 colors i.e. opening an 8-bit image and saving it straight
back should not change the appearance of the image.
This is why kpDocument will set the "Dither" boolean to false.  The reason
If kpDocument had set
"Dither" to true, this would not have been lossless and repeatedly
opening and resaving the same document would make it progressively blurier.

It cannot set the compression quality as Qt does not offer this information
-- this is why, after opening a JPEG in KolourPaint, clicking "Save" brings
up the saving file diaog (so that the user can specify a quality).

It also maintains information about the capabilities of many image
mimetypes.
