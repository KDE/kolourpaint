
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


//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
void kpTextSelection::paint (QImage *destPixmap, const QRect &docRect) const
{
#if DEBUG_KP_SELECTION
    kDebug () << "kpTextSelection::paint() textStyle: fcol="
            << (int *) d->textStyle.foregroundColor ().toQRgb ()
            << " bcol="
            << (int *) d->textStyle.backgroundColor ().toQRgb ()
            << endl;
#endif

    // Drawing text is slow so if the text box will be rendered completely
    // outside of <destRect>, don't bother rendering it at all.
    const QRect modifyingRect = docRect.intersect (boundingRect ());
    if (modifyingRect.isEmpty ())
        return;


    // Is the text box completely invisible?
    if (textStyle ().foregroundColor ().isTransparent () &&
        textStyle ().backgroundColor ().isTransparent ())
    {
        return;
    }

    kpImage floatImage(modifyingRect.size(), QImage::Format_ARGB32_Premultiplied);
    floatImage.fill(0);

    QRect theWholeAreaRect, theTextAreaRect;
    theWholeAreaRect = boundingRect ().translated (-modifyingRect.topLeft ());
    theTextAreaRect = textAreaRect ().translated (-modifyingRect.topLeft ());

    QList <QString> theTextLines;
    kpTextStyle theTextStyle;
    theTextLines = textLines ();
    theTextStyle = textStyle ();

    const QFontMetrics fontMetrics (theTextStyle.font ());

#if DEBUG_KP_SELECTION
    kDebug () << "kpTextSelection_Paint.cpp:DrawTextHelper";
    kDebug () << "\theight=" << fontMetrics.height ()
               << " leading=" << fontMetrics.leading ()
               << " ascent=" << fontMetrics.ascent ()
               << " descent=" << fontMetrics.descent ()
               << " lineSpacing=" << fontMetrics.lineSpacing ()
               << endl;
#endif

    const kpColor backColor = theTextStyle.backgroundColor ();

    QPainter painter(&floatImage);
    // Fill in the background.
    painter.fillRect (theWholeAreaRect, backColor.toQColor());

    painter.setClipRect(theWholeAreaRect);
    painter.setPen(theTextStyle.foregroundColor().toQColor());
    painter.setFont(theTextStyle.font());

    if ( theTextStyle.foregroundColor().toQColor().alpha() < 255 )
    {
      // if the foreground color has an alpha channel, we want to
      // see through the background, so we first need to punch holes
      // into the background where the text is
      painter.setCompositionMode(QPainter::CompositionMode_Clear);

      int baseLine = theTextAreaRect.y () + fontMetrics.ascent ();
      foreach (const QString &str, theTextLines)
      {
          painter.drawText (theTextAreaRect.x (), baseLine, str);
          baseLine += fontMetrics.lineSpacing ();
      }
      // the next text drawing will now blend the text foreground color with
      // what is really below the text background
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // Draw a line at a time instead of using QPainter::drawText(QRect,...).
    // Else, the line heights become >QFontMetrics::height() if you type Chinese
    // characters (!) and then the cursor gets out of sync.
    int baseLine = theTextAreaRect.y () + fontMetrics.ascent ();
    foreach (const QString &str, theTextLines)
    {
        painter.drawText (theTextAreaRect.x (), baseLine, str);
        baseLine += fontMetrics.lineSpacing ();
    }

    // ... convert that into "painting" transparent pixels on top of
    // the document.
    kpPixmapFX::paintPixmapAt (destPixmap,
        modifyingRect.topLeft () - docRect.topLeft (),
        floatImage);
}

//---------------------------------------------------------------------


// public virtual [kpAbstractSelection]
void kpTextSelection::paintBorder (QImage *destPixmap, const QRect &docRect,
        bool selectionFinished) const
{
    paintRectangularBorder (destPixmap, docRect, selectionFinished);
}

//---------------------------------------------------------------------


// public
kpImage kpTextSelection::approximateImage () const
{
    kpImage retImage (width (), height (), QImage::Format_ARGB32_Premultiplied);
    retImage.fill(0);
    paint (&retImage, boundingRect ());
    return retImage;
}

//---------------------------------------------------------------------
