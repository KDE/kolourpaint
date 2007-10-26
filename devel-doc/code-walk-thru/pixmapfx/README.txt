
pixmapfx/ Package


QPixmap manipulation and QPainter wrapper.

**** This is "must read" documentation ****

This package is supposed to only be used for drawing widgets and "view"
pixels stored in graphics memory (i.e. visualizing the image data stored in
kpDocument).  kpPainter in imagelib/ should be used on document pixels
instead but in reality, this does not currently always happen.


1. QPixmap, QImage and QPainter Primer

2. kpPixmapFX and Document Pixels

3. Using QPainter
3.1 Goal
3.2. Qt3 and Qt4 Differences
3.3. Design
3.4. Drawing on QPixmap
3.5. Drawing on QWidget
3.6. Effects of Violating the No-Alpha-Channel Invariant
3.7. The 32-bit Screen Problem

4. Method Overview
4.1. Screen Depth
4.2. QPixmap/QImage Conversion Functions
4.3. Abstract Drawing
4.4. Get/Set Parts of Pixmap
4.5. Mask Operations
4.6. Effects
4.7. Transforms
4.8. Drawing Shapes
4.9. Drawing Using Raster Operations


1. QPixmap, QImage and QPainter Primer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

QPixmap's are stored in graphics memory and run at screen depth or 1-bit
(QBitmap).  QPainter is used to draw on QPixmap's with great speed since
it can be done with the graphics hardware.  The accuracy of QPainter on
QPixmap is heavily dependent on the display driver, so you cannot depend on
QPainter to give consistent results on all platforms -- even X11 over VNC
has been known to corrupt pixels!

In contrast, QImage's are stored in main memory and support multiple depths
-- 1-bit, 8-bit and 32-bit.  Only Qt4 allows QPainter to operate on
QImage's and even then, only on 32-bit images.  QPainter gives consistent
and reliable results on all platforms.  Paletted QImage's (i.e. 1-bit and
8-bit) must be handled differently to 32-bit truecolor QImage's (see
kpEffectBalance::applyEffect()) and QPainter does not work on them.

The only way to read the color value of a QPixmap pixel is to convert it
to a QImage.  This is a slow, lossless conversion from graphics memory
to main memory.

To render a QImage, it must be converted to a QPixmap (else this will be
done internally by Qt).  This is a slow, possibly _lossy_ conversion from
main memory to graphics memory, since the screen/QPixmap may be running at a
low depth than QImage, resulting in color loss.  It is not lossy on a 24-bit
screen since that supports all possible RGB colors, which is why 24-bit
screens are encouraged over 15-bit and 16-bit screens.

To get QPainter to give consistent results on a QPixmap, to work around
QPainter giving no guarantees on QPixmap, convert it to QImage,
run QPainter (whose behavior is guaranteed on QImage) and then convert it
back to QPixmap.  But this is slow.

Under Qt4, QPainter adds the pen width on top of the size of rectangular
shapes (e.g. rectangle, rounded rectangle, ellipse)
i.e. with a pen width of 1:

    QPainter::drawRect(0, 0, 5/*width*/, 5/*height*/)

will give you a 6x6 rectangle.  All other methods (e.g.
QPainter::fillRect(0, 0, 5/*width*/, 5/*height*/, Qt::green)) will work as
expected i.e. you will get a 5x5 rectangle.  On the other hand, rectangular
QRegion's are 1 pixel shorter and thinner than you would expect!


2. kpPixmapFX and Document Pixels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

kpDocument pixels should be manipulated with kpPainter instead.

However, as kpDocument's image data is currently a QPixmap, kpPixmapFX is
also abused to manipulate kpDocument pixels.  Currently, kpPainter forwards
calls to kpPixmapFX, so effectively the pixmapfx/ package doubles as the
guts of the KolourPaint image library, but this will be changed when a
proper image library is developed in imagelib/ [see the imagelib/ package
documentation for details].


3. Using QPainter
~~~~~~~~~~~~~~~~~

3.1 Goal
--------

The goal of KolourPaint is to work on all X11 displays of depth 8 or
greater.  It is important to support displays that don't support the
fairly common XRENDER (alphablending) extension, since XRENDER is not
supported by commercial/older UNIXes (Solaris or if using XFree 3.3.6) and
some VNC clients & remote X11 servers (e.g. XMing).  Regrettably, QPainter
in Qt4 produces different results with and without XRENDER.  In order to
make KolourPaint as maintainable as possible, we would like just one code
path that works for both.  This section describes how we do that.

This discussion assumes the use of opaque colors and pure transparency only --
no alpha colors -- which is what KolourPaint currently supports for document
pixels.


3.2. Qt3 and Qt4 Differences
----------------------------

QPixmap's normally run at the screen depth, which is usually 24-bit,
sometimes 16-bit or 15-bit, occasionally 8-bit and theoretically 1-bit, 2-bit
& 4-bit.  KolourPaint encourages the use of 24-bit screens and discourages
16-bit and 15-bit usage due to QImage -> QPixmap conversions being lossy.
Anything lower than 15-bit is not supported, even though attempts to support
them can be found in the source code.

QPixmap::depth() is of, and returns, the screen depth, unless it is a QBitmap,
where it returns 1.  QPixmap's can optionally have a 1-bit QBitmap mask which
permits 0-1 transparency but not translucency.  QPixmap's without masks are
fully opaque.

In Qt3, QPixmap::mask() returned a pointer to the mask if it had one,
or null if not -- this call took essentially no time.  In Qt4, QPixmap::mask()
does some computation so while the call is still quite fast, it is now
a bit inefficient to check for the existence of the mask using
!QPixmap::mask().isNull().  The way to do it under Qt4 is QPixmap::hasAlpha()
(this might work under Qt3 too but I've never had a need to try it) and this
takes virtually no time to execute.

In Qt3 and in Qt4 without XRENDER, drawing on an opaque QPixmap
(i.e. one without a mask) with QPainter works as expected.  However, if
the QPixmap might have transparent pixels (i.e. it has a mask), QPainter
will not mutate the mask.  This is why we draw all shapes twice -- once
on the QPixmap/RGB layer and again on the mask layer (using Qt::color1
to mark opaque pixels and Qt::color0 to mark transparent pixels).  If we
are drawing pixels in the transparent color, we need ensure that there is a
mask layer (if there isn't one, we create a fully opaque QBitmap with the
same size as the QPixmap) to draw in Qt::color0 and we don't
need to draw on the RGB layer (but we can, although it makes no difference
since those pixels will be transparent).  This works quite well except for
antialiased text as the pixels set by QPainter on the RGB layer are slightly
different to the ones set on the mask.

Qt4 with XRENDER opens a can of worms.  While drawing with QPainter on a
QPixmap that does not have a mask is fine, if you use QPainter composition
modes or you draw on a QPixmap that _does_ have a mask, you introduce a
256-level alpha channel, even if a mask will suffice.  QPainter draws on both
the RGB layer _and_ alpha layer, unlike Qt4 without XRENDER and Qt3.
QPixmap::depth() will now return 32.  More importantly, QPixmap::mask()
is now computed from the alpha channel using an expensive QPixmap -> QImage ->
QPixmap conversion and is unbearably slow.  It will never return a null mask
so QPixmap::hasAlpha() will always return true.  QPixmap::hasAlphaChannel()
now returns true.


3.3. Design
-----------

We want a shared code path that works on Qt4 regardless of XRENDER.
Recall that without XRENDER, we must draw on the RGB layer and on the mask
layer (if it exists or if we are drawing in the transparent color).  To draw
on the mask layer requires accessing the mask with QPixmap::mask().
Since with XRENDER, QPixmap::mask() is very slow on a pixmap with an alpha
channel, we must avoid introducing an alpha channel.  This is the KolourPaint
no-alpha-channel invariant as enforced by the KP_PFX_CHECK_NO_ALPHA_CHANNEL
assert-macro.

The technique to avoid an introducing alpha channel under XRENDER is to:

  1. Do not use QPainter composition modes on QPixmap's.

  2. For QPixmap's with masks, move away the mask before opening QPainter
     on those QPixmap's.

This happens to work without XRENDER as well because:

  1. Qt4 without XRENDER does not support composition modes on QPixmap
     anyway and we avoid them like the plague to allow for a single,
     unifed code path.

     **** Using QPainter composition modes on QPixmap or QWidget is a
          common mistake for this reason ****

  2. Of course, 2. above works on screens without XRENDER.


3.4. Drawing on QPixmap
-----------------------

The design above is captured by the kpPixmapFX::draw() methods.  Higher-level
methods such as kpPixmapFX::setPixmapAt() and kpPixmapFX::drawLine() use those
draw() methods.  These higher-level methods are fast, require no conversions
to-or-from QImage, ensure that no alpha channel is produced and that the
results are consistent on all platforms.

You must use either the lower-level draw(), or those higher-level methods, on
all QPixmap's that are passed to kpPixmapFX methods (e.g. document image
data).  You must not use QPainter on such QPixmap's.  If you are drawing on a
QWidget, not a QPixmap, and are not using kpPixmapFX, you are free to use
QPainter as usual.

The below examples illustrates most cases.  They assume that a QPixmap called
"pixmap", which already satisfies kpPixmapFX's no-alpha-channel invariant,
is within scope.


Example 1 - QPainter on QPixmap (WRONG):

    QPainter painter (&pixmap);
    painter.setPen (Qt::green);
    painter.drawLine (0, 0, 10, 10);

  **** This is a common mistake ****

Definitely wrong as QPainter may introduce an alpha channel.

On the other hand, you are allowed to use this on QPixmap's that aren't
eventually passed through kpPixmapFX methods.  But you cannot use this on
document image data.

Example of legitimate uses are for rendering GUI elements like menubars and
toolbars, such as in the Qt and KDE libraries.  In such situations, you are
also allowed to use translucent colors and access the alpha channel as you
please.  But you must not use composition modes since they are not supported
by QPainter on QPixmap's without XRENDER.  However you can get around this by
converting to a QImage -- see Example 4.


Example 2 - QPainter on QPixmap, followed by alpha channel nuke (WRONG):

    QPainter painter (&pixmap);
    painter.setPen (Qt::green);
    painter.drawLine (0, 0, 10, 10);
    kpPixmapFX::ensureNoAlphaChannel (&pixmap);

This should be avoided because I swear it changes transparent pixels into
opaque ones sometimes.  And it is too slow for interactive tools since it
uses QPixmap::mask()


Example 3 - Using kpPixmapFX methods (CORRECT):

    kpPixmapFX::drawLine (&pixmap,
        0, 0, 10, 10,
        kpColor::Green,
        1/*pen width*/);

Fine as kpPixmapFX::draw*() methods ensure that no alpha channel is produced.

Note that kpPixmapFX's rectangular-shape-drawing methods work around
QPainter/Qt4 producing results higher and wider than expected.
i.e. With a pen width of 1, kpPixmapFX::drawRect(pixmap, 0, 0, 5/*width*/,
5/*height*/) gives you a 5x5 rectangle, while QPainter::drawRect(0, 0,
5/*width*/, 5/*height*/) will give you a 6x6 rectangle.


Example 4 - QPainter on QImage (CORRECT):

   QImage qimage = kpPixmapFX::convertToQImage (pixmap);

   QPainter painter (&qimage);
   painter.setPen (Qt::green);
   painter.drawLine (0, 0, 10, 10);

   painter.end ();

   pixmap = kpPixmapFX::convertToPixmap (qimage);

Note that when we convert from QPixmap -> QImage -> QPixmap, we never use
kpPixmapFX::convertToPixmap (qimage, true/*dither*/) or QPixmap::fromImage
(qimage) to avoid extra dithering and the image getting progressively blurier
with more conversoins.

This is OK for image effects but too slow for interactive tools since it
uses QPixmap::mask().

On the bright side, QPainter composition modes also work on QImage, even
without XRENDER.

Strangely, this seems to handle transparent pixels correctly, unlike Example
2.

WARNING: On an 8-bit screen:

             QPixmap result = convertToPixmap (convertToQImage (pixmap));

         <result> is slightly differently colored to <pixmap>.

         Since this technique is currently frequently used on document image
         data (and there is no easy way around it), KolourPaint should not be
         used on an 8-bit screen.


3.5. Drawing on QWidget
-----------------------

CORRECT:

    QWidget *w = <widget>;
    QPainter (w);
    painter.setPen (Qt::green);
    painter.drawLine (0, 0, 10, 10);

There are no limitations on how you draw on a QWidget except that you should
not use QPainter composition modes as, like QPixmap's, they do not work
without XRENDER.  You can get around this problem by drawing onto a QImage and
rendering it onto the QWidget, instead of drawing directly on the QWidget.

You are free to ignore the no-alpha-channel invariant with QPixmap's
that don't pass through kpPixmapFX methods and QWidget's.


3.6. Effects of Violating the No-Alpha-Channel Invariant
--------------------------------------------------------

The effects of introducing an alpha channel, and breaking the invariant, are
dependent on whether XRENDER is supported.

Without XRENDER, it is impossible to break this invariant since alpha channels
are not supported.  However it is possible to see the effects of incorrect
code, as described in the next paragraph.

With XRENDER, as previously discussed, KolourPaint becomes really slow as
QPixmap::mask() is slow with alpha channels.  KP_PFX_CHECK_NO_ALPHA_CHANNEL
will print a kError() telling you that you have a bug.  Generally speaking,
this means that you used the incorrect code in Example 1 above.  You probably
also assumed that QPainter would draw on the mask layer as well (which it does
with XRENDER, after converting the mask layer to an alpha layer).  However,
the same code without XRENDER will not draw on the mask automatically -- this
can be seen as pixels which are transparent instead of being opaque and
vice-versa.

With XRENDER, the slowness would really only be perceivable with interactive
tools.  However, most interactive tools use kpPixmapFX::setPixmapAt() in
response to every mouse move, which, assuming KP_PFX_CHECK_NO_ALPHA_CHANNEL
has not been set to assert-die, will fortuitously convert the alpha channel
back into a mask by using the kpPixmapFX drawing technique, hence fixing the
slowness.  There is also a hack in kpTool::endDrawInternal() that does this
as a backup. However, breaking the invariant and depending on these automatic
fixers is incorrect -- not just because of momentarily bad performance but
also because:

  1. As described in Example 2 above, creating an alpha channel and changing
     it back into a mask seems unreliable.

  2. As described above, the same code without XRENDER will not draw on the
     mask, when it needs to.


3.7. The 32-bit Screen Problem
------------------------------

In recent X servers (e.g. CentOS 5), 24-bit screens seem to expose 32-bit
visuals, rather than only 24-bit visuals, presumably to support full-blown
alpha blending.  On such servers, if
/trunk/qt-copy/patches/0182-argb-visuals-default.diff from KDE SVN -r720997 is
also applied (it was deleted -r720998 but I expect it to find its way back
into Qt4 eventually), _all_ QPixmap's have alpha channels by default and
QPixmap::depth() will always return 32.  See "Qt3 and Qt4 Differences" for the
consequences of this -- this violates the no-alpha-channel invariant.

We could solve this problem by abandoning the idea of a single code
path for all displays:

  1. No XRENDER
  2. With XRENDER: 15-bit, 16-bit and 24-bit
  3. With XRENDER: 32-bit

which would remove the need for this fragile no-alpha-channel invariant.  But
with more code, comes even greater maintenance responsibility.

Much of the need for kpPixmapFX comes from it being abused as an image library
(i.e. for document pixels).  If we wrote a proper image library, not based on
QPixmap, we probably wouldn't have such a big problem, since kpPixmapFX would
no longer be needed in the troublesome cases with masks.

The current solution is to revert to a 24-bit visual by disabling XRENDER
[kpPixmapFX::initMaskOpsPre()].  This is ugly since fonts and icons are no
longer antialiased, but at least it works.


4. Method Overview
~~~~~~~~~~~~~~~~~~

4.1. Screen Depth
-----------------

Centralizes utility methods for querying screen depth.


4.2. QPixmap/QImage Conversion Functions
----------------------------------------

[>>>] QImage convertToQImage (const QPixmap &pixmap)

Encapsulates QPixmap::toImage().


[>>>] QPixmap convertToPixmap (const QImage &image, bool pretty = false,
          const WarnAboutLossInfo &wali = WarnAboutLossInfo ())

Opposite of convertToQImage().

Use, with "pretty" set to false, instead of QPixmap::fromImage(),
to avoid successive QPixmap -> QImage -> QPixmap conversions making the image
progressively blurier.  This will also establish the no-alpha-channel
invariant.

"pretty" set to true dithers if necessary but is rarely (if ever) used
because of the above reason.


[>>>] QPixmap convertToPixmapAsLosslessAsPossible (const QImage &image,
          const WarnAboutLossInfo &wali = WarnAboutLossInfo ())

Use this if the source is really a QImage (e.g. loaded from a file), as
opposed to a QImage that came from a QPixmap.  This invokes a dodgy heuristic
to determine whether or not to dither, to make the pixmap look as much like
the original QImage as possible.


4.3. Abstract Drawing
---------------------

The low-level methods for drawing on a QPixmap and maintaining the alpha
channel invariant, whose design was mentioned earlier.  Usually, the
high-level "Drawing Shapes" methods, which use this as a backend, are used
instead.


4.4. Get/Set Parts of Pixmap
----------------------------

getPixmapAt() and setPixmapAt() get and set rectangular regions of a pixmap.
setPixmapAt() is like QPainter::drawPixmap()
with QPainter::CompositionMode_Source, while paintPixmapAt() is like with
QPainter::CompositionMode_SourceOver.


4.5. Mask Operations
--------------------

Miscellaneous mask operations.

initMaskOpsPre() disables XRENDER for 32-bit visuals, as previously discussed.

hasMask() and hasAlphaChannel() encapsulate QPixmap methods.

KP_PFX_CHECK_NO_ALPHA_CHANNEL() checks the no-alpha-channel invariant.
You should put this at the start and end of all methods you add to deal with
document pixels.

ensureNoAlphaChannel() kills the alpha channel.
Generally, the only time you need to call it is when you get a QImage
from a foreign source (e.g. from a file or clipboard, not originally
from a QPixmap) and need to convert it to a QPixmap.  You don't need
to call it anywhere else since internally, all KolourPaint code is
supposed to maintain the QPixmap-has-no-alpha-channel invariant.
However, convertToPixmap() and convertToPixmapAsLosslessAsPossible()
call this method for you anyway.

ensureTransparentAt() and ensureOpaqueAt() are ways of manipulating the mask.
Other ways are to use getPixmapAt()/setPixmapAt() and the shape drawing
methods.


4.6. Effects
------------

[>>>] void fill (QPixmap *destPixmapPtr, const kpColor &color)

      Fills an image in the given color.


4.7. Transforms
---------------

resize(), scale(), rotate(), flip()


4.8. Drawing Shapes
-------------------

drawPolyline(), drawLine(), drawPolygon(), drawCurve(), fillRect(),
drawRect(), drawRoundedRect(), drawEllipse()

Use these instead of QPainter as previously discussed, as they maintain the
no-alpha-channel invariant.  Most support foreground color, fill color, pen
width and simulated stippling.

The rectangular-shape-drawing methods work around QPainter/Qt4 producing
results higher and wider than expected, as previously discussed.


4.9. Drawing Using Raster Operations
------------------------------------

Simulates raster operations which are not available in Qt4.