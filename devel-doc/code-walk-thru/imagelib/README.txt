
imagelib/


Document image data manipulation.


1. Image Data and the Graphics Card - A Primer
1.1. Separating the Graphics Client and Server
1.2. Color Models
1.3. The Document's Image Data Must Be Stored In Main Memory

2. KolourPaint's Image Library

3. Future Direction
3.1. Rewrite the Image Library
3.2. Graphics Card Hardware Acceleration

4. imagelib/
4.1. imagelib/effects/
4.2. imagelib/transforms/


1. Image Data and the Graphics Card - A Primer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In short, the monitor displays part of graphics memory, not main memory.
To display an image stored in main memory, you must copy it to graphics
memory, doing some conversions.


1.1. Separating the Graphics Client and Server
----------------------------------------------

Normally programs are run on a local computer.  However, the X11 protocol
permits program logic to be executed on a remote computer (the "network
server", misleading known as the "graphics client") and displayed on the
local computer (the "network client", misleading known as the "graphics
server").  For the purposes of this document, when we say server, we mean
graphics server and when we say client, we mean graphics client.  These are
the opposite meanings to the network version of these terms.

While this separation seems rather silly and not often used, this actually
makes us think a bit harder about the separation between:

  1. the graphics card, graphics memory, GPU & monitor (collectively,
     the graphics server)

  2. main memory and the CPU (collectively, graphics client).

In the case that the program is executed locally, the hardware bus
separates the client and server.  This is far slower than both main memory and
graphics memory, so copying data between the client and server must be
minimised.

If the program is executed remotely, a network bus of much higher latency
could separate the client and server and so this consideration becomes
even more important.


1.2. Color Models
-----------------

A color model is a way of representing a subset of all possible
human-visible colors e.g. Indexed RGB, RGB, RGBA, CMYK and XYZ.  Any color
model can be represented in a device-independent format (i.e. at full
quality) on the client in main memory.

TODO: more on indexed vs truecolor

The graphics memory on the server supports a limited number of color models
(at least RGB) at the current display depth.  Furthermore, this depth could be
8-bit or even 1-bit so could be very low quality.


1.3. The Document's Image Data Must Be Stored In Main Memory
------------------------------------------------------------

It follows that unless the server can support the document's color model, we
must not store the document image data directly in graphics memory.  Worse
still, some servers such as VNC servers, seem to corrupt/approximate graphics
memory so even with a color model match, we can still lose document
information, if we were to store it in graphics memory.

So we must store document data in main memory.  Every time we change the
document we must copy it from main memory (client) to graphics memory
(server), performing a colorspace conversion.  We cannot do separate drawing
operations on main memory and graphics memory, and assume that they will be in
sync, as there is no guarantee that, for instance, the graphics card will draw
lines in graphics memory, the same way we draw in main memory.  This is slow
because it crosses the bus or the network.


2. KolourPaint's Image Library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As document image data should be stored in main memory, the obvious choice
would be QImage.  However, in the beginning, KolourPaint was a Qt3 project
and QPainter did not support QImage.  Furthermore, storing and manipulating
the document in main memory (QImage), before converting and copying
it to graphics memory (QPixmap) using the software QPixmap::convertToImage()
was -- and still is -- too slow (from what I remember, it takes about 100ms to
convert a 1024x768x32 image for display on a screen with a matching color
model on a Celeron 2.2 Ghz!).

To avoid these problems, a hasty and horrible design decision was made:
KolourPaint stores and manipulates the document directly in graphics memory
(QPixmap).

**** This is the reason why KolourPaint is horribly flawed. ****

Any document that cannot be losslessly converted to the screen's color model
is corrupted.  For example, a 24-bit RGB document will lose color information
on a 16-bit screen -- resaving will make this permanent.

And it's worse than that because we also convert all indexed images to
truecolor RGB (except on an indexed 8-bit screen), resulting in the loss of
the palette.  We currently also nuke the alpha channel since that wasn't
supported under Qt3.  Qt4 supports it for QPixmap but only if XRENDER is
enabled, so that support is no good for our purposes anyway.

As KolourPaint's document data is QPixmap-based, the manipulation
methods are currently provided by the kpPixmapFX backend.  See the pixmapfx/
package documentation for details.


3. Future Direction
~~~~~~~~~~~~~~~~~~~

3.1. Rewrite the Image Library
------------------------------

[derived from Clarence's Google Summer of Code 2006 suggestion]

We must move the storage of document image data from graphics memory to
main memory.  This is very high priority and would fix the fundamental problem
with KolourPaint.  We would also like to support alpha channel editing.

We need to either write, or hopefully find, an easy-to-use image library for
manipulating and rendering both paletted (with and without alpha) and RGBA
document image data that:

  * is easy to use
  * is stable
  * performs well
  * is opensource
  * can be linked to without viral licensing effects

and then port KolourPaint to use it.  This would be a serious software
engineering exercise that would take months, as it breaks assumptions
(e.g. image data being a QPixmap, no alpha channel) littered across the entire
codebase.

The resulting KolourPaint would be:

  * screen depth independent (Bug 85280 and others)
  * support alpha (Bug 94607)
  * support multiple color models (not just the screen's color model),
    including editing of 8-bit paletted images (even with alpha)

Rather than writing an image library from scratch, you can investigate the
feasibility of using and extending QPainter/QImage.  Note that QImage
(off-screen main memory) -> QPixmap (on-screen graphics memory) conversions
are known to be very, very expensive even on a local display (this might be
fixed with future QPixmap MITSHM support, or could be hacked around by using a
tile-based renderer).  Also, QPainter/QImage does not support drawing on 8-bit
paletted images and does not load 8-bit paletted images that have alpha
properly (automatic conversion to 32-bit RGBA, losing the palette).

Alternatively, you could reuse code and ideas from:

  * X11/XPixmap/XRENDER API
  * Krita
  * GIMP/GEGL
  * mtPaint
  * XPaint
  * ImageMagick
    etc.

The TODO file has some additional pointers.  Be careful to not use reuse
virally-licensed code such as GPL.

SIMD assembler instructions (MMX, SSE, SSE2 etc.) can be used for performance.


3.2. Graphics Card Hardware Acceleration
----------------------------------------

The new image library would be storing the document data.  We need to convert
to the graphics memory's colorspace, while copying the data to graphics
memory.  OpenGL might offer limited colorspace conversion in hardware (e.g.
32-bit RGBA to 16-bit RGBA), and Qt now offers an OpenGL interface.

I also suspect that OpenGL also performs zooming (for zoomed views) much
faster than QPainter/QPixmap.

A neat trick (but I'm not sure if it's possible since I don't play with
OpenGL much) is that you could losslessly store document data in graphics
memory if you are certain of the way the server stores image data, by
using clever encoding.  This would avoid expensive bus traffic between
the graphics client and server.  For example, if the graphics memory only
supports 8-bit channels, you can still store 16-bit channels by using pairs of
pixels to represent single document pixels.  To render it you would program
the highly parallel shader hardware to do dithering.  Image
transformations could also be performed very quickly using shader hardware
and would be hundreds or thousands of times faster than the CPU.  A
complication is that shader access usually requires binary graphics drivers,
which many distributions do not ship by default because they are “non-free”.


4. imagelib/
~~~~~~~~~~~~

This package is supposed to contain all the data structures and methods for
manipulating document pixels.

kpImage's store document pixels, which of kpColor, and this should be in main
memory.  kpPainter draws shapes on top of kpImage's and there are various
effects/ and transforms/ that work on kpImage as well.

This encapsulation allows us to theoretically, simply plug in the new
image library, moving away from document pixels being stored in graphics
memory as QPixmap's, and manipulations being done with QPainter.  The current
situation is that kpImage is QPixmap and kpPainter is essentially QPainter.

The reality with "plugging in" the new image library is that parts of
KolourPaint still violate the imagelib/ package encapsulation, or use kpImage
(supposed to be document data in main memory) where QPixmap (view data in
graphics memory) should be used and vice-versa.  Also, the boundary between
where a kpImage is rendered (converted into graphics memory) is currently not
cleanly defined (but it's mainly done in kpView::paintEvent() and in the
selection text rendering code).


[>>>] kpColor

kpColor is an object-oriented abstraction of QRgb, for document image
data, with the additional restriction of enforcing the KolourPaint convention
of only supporting totally transparent and totally opaque colors.  Eventually,
this restriction will be dropped.  In the future, other color models such as
8-bit indexed will be supported.  It also provides better error handling,
reporting (noisy kError()'s) and recovery compared to Qt.  This abstraction
will allow us to eventually dump the Qt paint routines for document image
data.

In general, you should pass around kpColor objects instead of QRgb
and QColor.  Only convert an opaque kpColor to a QColor (using toQColor())
if you need to draw something on-screen.

Constructing a kpColor object from QColor is usually wrong since QColor's
come from on-screen pixels, which may lack the full color resolution of
kpColor, due to the limited color range on e.g. a 16-bit screen.


[>>>] kpDocumentMetaInfo

Complement of kpImage -- DPI, offset and image comments fields.


[>>>] kpFloodFill

Flood fill effect for document pixels.


[>>>] kpImage

Bitmap abstraction, storing document image data.  Does not care about
metadata (including DPI) -- that job is for kpDocumentMetaInfo

Supposed to be independent of the screen and represents precious
document data that should not be dithered down.  Should support 1-bit
indexed, 8-bit indexed, 16-bit RGB(A) and 24-bit RGB(A) colour models
at least.

Currently, this is actually the same as QPixmap so the reality is that its
color model is the same as the screen mode and it optionally contains a
0/1 transparency mask.  A current KolourPaint invariant is that it will
not contain an alpha channel -- it can either have a transparency mask or
nothing at all (see pixmapfx/ package documentation).


[>>>] kpPainter

Stateless painter with sane semantics that works on kpImage's i.e. it
works on document - not view - data.  kpPainter is to kpImage as QPainter
is to QPixmap.

This encapsulates the set of functionality used by all of KolourPaint's
document drawing functions and nothing more, permitting rewriting of
the image library.  Currently uses QPainter/kpPixmapFX as the backend
-- most calls are simply forwarded to kpPixmapFX.


4.1. imagelib/effects/
----------------------

A whole bunch of kpImage effects:

kpEffectBalance
kpEffectBlurSharpen
kpEffectEmboss
kpEffectFlatten
kpEffectGrayscale
kpEffectHSV
kpEffectInvert
kpEffectReduceColors
kpEffectToneEnhance


4.2. imagelib/transforms/
-------------------------

Crop and Auto Crop transforms -- see the user handbook.
