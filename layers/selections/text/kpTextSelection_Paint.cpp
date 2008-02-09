
// REFACTOR: Move into kpPainter

/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DEBUG_KP_SELECTION 0


#include <kpTextSelection.h>
#include <kpTextSelectionPrivate.h>

#include <QBitmap>
#include <QFont>
#include <QList>
#include <QPainter>

#include <KDebug>

#include <kpPixmapFX.h>
#include <kpTextStyle.h>


static void DebugAlpha (const QPixmap &pm)
{
#if DEBUG_KP_SELECTION
    kDebug () << "\tpixmap: hasMask=" << !pm.mask ().isNull ()
              << " hasAlpha=" << pm.hasAlpha ()
              << " hasAlphaChannel=" << pm.hasAlphaChannel ()
              << endl;
#endif
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);
}


struct DrawTextLinesPackage
{
    QRect wholeAreaRect, textAreaRect;

    QList <QString> textLines;
    kpTextStyle textStyle;
};

// TODO: QPainter::drawText() draws the same text with the same font differently
//       on the RGB and mask layers.  The effect of this can be best seen by
//       contrasting the rendering of a text box with opaque text but a
//       see-through background, on top of transparent document areas compared
//       to opaque document areas.
static void DrawTextHelper (QPainter *p, bool drawingOnRGBLayer, void *data)
{
    DrawTextLinesPackage *pack = static_cast <DrawTextLinesPackage *> (data);

    const QFontMetrics fontMetrics (pack->textStyle.font ());

#if DEBUG_KP_SELECTION
    kDebug () << "kpTextSelection_Paint.cpp:DrawTextHelper(drawingOnRGBLayer="
              << drawingOnRGBLayer << ")" << endl;

    kDebug () << "\theight=" << fontMetrics.height ()
               << " leading=" << fontMetrics.leading ()
               << " ascent=" << fontMetrics.ascent ()
               << " descent=" << fontMetrics.descent ()
               << " lineSpacing=" << fontMetrics.lineSpacing ()
               << endl;
#endif


    // Has an opaque background?
    const kpColor backColor = pack->textStyle.effectiveBackgroundColor ();
    if (backColor.isOpaque ())
    {
        // Fill in the background.
        p->fillRect (pack->wholeAreaRect,
            kpPixmapFX::draw_ToQColor (backColor, drawingOnRGBLayer));
    }


    // Don't draw on the RGB layer if the text is transparent for 2 reasons:
    //
    // 1. It saves CPU - transparency is determined by the mask, not the RGB
    //    layer anyway.
    // 2. More importantly, since QPainter::drawText() draws the same text
    //    with the same font differently on the RGB and mask layers, we don't
    //    draw on the RGB layer to avoid artifacts.
    //    TODO: Don't the shape drawing methods have to worry about this too?
    //          Later: Yes they do, see kpPixmapFX::draw_ToQColor().
    //          Why not centralize this in draw()?
    if (!(drawingOnRGBLayer && pack->textStyle.foregroundColor ().isTransparent ()))
    {
        p->setClipRect (pack->wholeAreaRect);
        
        p->setPen (
            kpPixmapFX::draw_ToQColor (pack->textStyle.foregroundColor (),
                drawingOnRGBLayer));
        p->setFont (pack->textStyle.font ());


        // Draw a line at a time instead of using QPainter::drawText(QRect,...).
        // Else, the line heights become >QFontMetrics::height() if you type Chinese
        // characters (!) and then the cursor gets out of sync.
        int baseLine = pack->textAreaRect.y () + fontMetrics.ascent ();
        foreach (const QString &str, pack->textLines)
        {
            // Note: It seems text does not antialias without XRENDER.
            p->drawText (pack->textAreaRect.x (), baseLine, str);

            baseLine += fontMetrics.lineSpacing ();
        }
    }
}

// private
//
// This method uses the text style colors as follows:
//
// A transparent foreground means that text is drawn in the transparent color
// ("setting" document pixels) as opposed to being see-through and not being
// drawn, which is why we need to wrap this with drawText().
//
// However, a transparent background is see-through and permits the text to
// antialias with the document pixels below.
void kpTextSelection::drawTextInternal (QPixmap *destPixmap, const QRect &docRect) const
{
    // Pixels are set to transparent only if the foreground is transparent.
    // In contrast, a transparent background does not set any pixels so we
    // don't include it in here.
    const bool anyColorTransparent =
       (d->textStyle.foregroundColor ().isTransparent ());

    // [Check 1]
    if (anyColorTransparent)
    {
        // draw() normally does this for us but we have "Check 2" just below
        // (but before draw()) that needs to know whether we will have a mask
        // in draw().
        //
        // Currently, this makes no functional difference since "Subcheck 2.1"
        // only passes if "Check 1" fails (us).  But this is good futureproofing
        // in case another subcheck is placed under "Check 2".
        destPixmap->setMask (kpPixmapFX::getNonNullMask (*destPixmap));
    }


    // [Check 2]
    if (kpPixmapFX::hasMask (*destPixmap))
    {
        // [Subcheck 2.1] Are we drawing opaque text over potentially
        //                transparent document pixels?
        if (d->textStyle.foregroundColor ().isOpaque () &&
            d->textStyle.effectiveBackgroundColor ().isTransparent ())
        {
        #if DEBUG_KP_SELECTION
            kDebug () << "\tensuring image's transparent pixels are defined";
        #endif

            // Set the RGB of transparent pixels to the foreground colour to avoid
            // anti-aliasing the foreground colored text with undefined RGBs.
        #if 1
            // TODO: This might not work under Qt4.
            //       See kpPixmapFX::pixmapWithDefinedTransparentPixels() API Doc.
            *destPixmap = kpPixmapFX::pixmapWithDefinedTransparentPixels (*destPixmap,
                d->textStyle.foregroundColor ().toQColor ());
            // This touches fewer pixels and could be more efficient than the
            // above but does not work since setPixmapAt() only copies the RGB
            // data of non-transparent pixels.
        #else
            QRect modifyingRectRelPixmap = modifyingRect;
            modifyingRectRelPixmap.translate (-docRect.x (), -docRect.y ());

            // This does not work since setPixmapAt() only copies the RGB data
            // of non-transparent pixels.
            kpPixmapFX::setPixmapAt (destPixmap,
                modifyingRectRelPixmap,
                kpPixmapFX::pixmapWithDefinedTransparentPixels (
                    kpPixmapFX::getPixmapAt (*destPixmap, modifyingRectRelPixmap),
                    d->textStyle.foregroundColor ().toQColor ()));
        #endif
        }
    }


    DrawTextLinesPackage pack;
    pack.wholeAreaRect = boundingRect ().translated (-docRect.topLeft ());
    pack.textAreaRect = textAreaRect ().translated (-docRect.topLeft ());
    pack.textLines = textLines ();
    pack.textStyle = textStyle ();

    kpPixmapFX::draw (destPixmap, &::DrawTextHelper,
        textStyle ().foregroundColor ().isOpaque () ||
            textStyle ().effectiveBackgroundColor ().isOpaque (),
        anyColorTransparent,
        &pack);


    ::DebugAlpha (*destPixmap);

#if DEBUG_KP_SELECTION
    kDebug ();
#endif
}

// private
//
// This wraps drawTextInternal(), ironing out NOP cases and also ensuring
// that transparent pixels are "painted" on top of the document, not "set".
void kpTextSelection::drawText (QPixmap *destPixmap, const QRect &docRect) const
{
#if DEBUG_KP_SELECTION
    kDebug () << "kpTextSelection::drawText("
              << "docRect=" << docRect << ")"
              << " boundingRect=" << boundingRect () << endl;
#endif

    ::DebugAlpha (*destPixmap);


    // Drawing text is slow so if the text box will be rendered completely
    // outside of <destRect>, don't bother rendering it at all.
    const QRect modifyingRect = docRect.intersect (boundingRect ());
    if (modifyingRect.isEmpty ())
        return;


    // Is the text box completely invisible?
    if (textStyle ().foregroundColor ().isTransparent () &&
        textStyle ().effectiveBackgroundColor ().isTransparent ())
    {
        return;
    }


    // Transparent text on an opaque background?
    if (textStyle ().foregroundColor ().isTransparent () &&
        textStyle ().effectiveBackgroundColor ().isOpaque ())
    {
        kpImage floatImage (modifyingRect.width (), modifyingRect.height ());

        // This draws transparent text by "setting" transparent pixels...
        drawTextInternal (&floatImage, modifyingRect);

        // ... convert that into "painting" transparent pixels on top of
        // the document.
        kpPixmapFX::paintPixmapAt (destPixmap,
            modifyingRect.topLeft () - docRect.topLeft (),
            floatImage);
    }
    // Opaque text on a transparent or opaque background?
    else
    {
        drawTextInternal (destPixmap, docRect);
    }
}


// public virtual [kpAbstractSelection]
void kpTextSelection::paint (QPixmap *destPixmap, const QRect &docRect) const
{
#if DEBUG_KP_SELECTION
    kDebug () << "kpTextSelection::paint() textStyle: fcol="
            << (int *) d->textStyle.foregroundColor ().toQRgb ()
            << " bcol="
            << (int *) d->textStyle.effectiveBackgroundColor ().toQRgb ()
            << endl;
#endif

    // (may have to antialias with background)
    drawText (destPixmap, docRect);
}


// public virtual [kpAbstractSelection]
void kpTextSelection::paintBorder (QPixmap *destPixmap, const QRect &docRect,
        bool selectionFinished) const
{
    paintRectangularBorder (destPixmap, docRect, selectionFinished);
}


// public
kpImage kpTextSelection::approximateImage () const
{
    kpImage retImage (width (), height ());

    // Are we a text box with a see-through background?
    if (d->textStyle.effectiveBackgroundColor ().isTransparent ())
    {
        // Give it a defined background of an arbitrarily neutral color.
        kpPixmapFX::fill (&retImage, kpColor::Transparent);
    }

    paint (&retImage, boundingRect ());

    return retImage;
}
