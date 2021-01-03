
// REFACTOR: Move into kpPainter

/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2010 Tasuku Suzuki <stasuku@gmail.com>
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


#include "kpTextSelection.h"
#include "kpTextSelectionPrivate.h"

#include "kpTextStyle.h"
#include "kpPreeditText.h"
#include "pixmapfx/kpPixmapFX.h"

#include "kpLogCategories.h"

#include <QBitmap>
#include <QFont>
#include <QList>
#include <QPainter>
#include <QTextCharFormat>

//---------------------------------------------------------------------

void kpTextSelection::drawPreeditString(QPainter &painter, int &x, int y, const kpPreeditText &preeditText) const
{
    int i = 0;
    const QString& preeditString = preeditText.preeditString ();
    QString str;
    for (const auto &attr : preeditText.textFormatList ())
    {
        int start = attr.start;
        int length = attr.length;
        QTextCharFormat format = qvariant_cast<QTextFormat> (attr.value).toCharFormat ();

        if (i > start)
        {
            length = length - i + start;
            start = i;
        }
        if (length <= 0) {
            continue;
        }

        if (i < start)
        {
            str = preeditString.mid (i, start - i);
            painter.drawText (x, y, str);
            x += painter.fontMetrics().horizontalAdvance(str);
        }

        painter.save();
        str = preeditString.mid (start, length);
        int width = painter.fontMetrics().horizontalAdvance(str);
        if (format.background ().color () != Qt::black)
        {
            painter.save ();
            painter.setPen (format.background ().color ());
            painter.setBrush (format.background());
            painter.drawRect (x, y - painter.fontMetrics ().ascent (), width, painter.fontMetrics ().height ());
            painter.restore ();
        }
        if (format.foreground ().color () != Qt::black)
        {
            painter.setBrush (format.foreground ());
            painter.setPen (format.foreground ().color ());
        }
        if (format.underlineStyle ())
        {
            painter.drawLine (x, y + painter.fontMetrics ().descent (), x + width, y + painter.fontMetrics ().descent ());
        }
        painter.drawText (x, y, str);

        x += width;
        painter.restore ();

        i = start + length;
    }
    if (i < preeditString.length ())
    {
        str = preeditString.mid (i);
        painter.drawText (x, y, str);
        x += painter.fontMetrics().horizontalAdvance(str);
    }
}

//---------------------------------------------------------------------

// public virtual [kpAbstractSelection]
void kpTextSelection::paint(QImage *destPixmap, const QRect &docRect) const
{
#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "kpTextSelection::paint() textStyle: fcol="
            << (int *) d->textStyle.foregroundColor ().toQRgb ()
            << " bcol="
            << (int *) d->textStyle.backgroundColor ().toQRgb ();
#endif

    // Drawing text is slow so if the text box will be rendered completely
    // outside of <destRect>, don't bother rendering it at all.
    const QRect modifyingRect = docRect.intersected (boundingRect ());
    if (modifyingRect.isEmpty ()) {
        return;
    }


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

    QList <QString> theTextLines = textLines();
    kpTextStyle theTextStyle = textStyle();

    const QFontMetrics fontMetrics (theTextStyle.font ());

#if DEBUG_KP_SELECTION
    qCDebug(kpLogLayers) << "kpTextSelection_Paint.cpp:DrawTextHelper";
    qCDebug(kpLogLayers) << "\theight=" << fontMetrics.height ()
               << " leading=" << fontMetrics.leading ()
               << " ascent=" << fontMetrics.ascent ()
               << " descent=" << fontMetrics.descent ()
               << " lineSpacing=" << fontMetrics.lineSpacing ();
#endif

    QPainter painter(&floatImage);

    // Fill in the background using the transparent/opaque tool setting
    if ( theTextStyle.isBackgroundTransparent() ) {
      painter.fillRect(theWholeAreaRect, Qt::transparent);
    }
    else {
      painter.fillRect(theWholeAreaRect, theTextStyle.backgroundColor().toQColor());
    }

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
      for (const auto &str : theTextLines)
      {
          painter.drawText (theTextAreaRect.x (), baseLine, str);
          baseLine += fontMetrics.lineSpacing ();

          // if the next textline would already be below the visible text area, stop drawing
          if ( (baseLine - fontMetrics.ascent()) > (theTextAreaRect.y() + theTextAreaRect.height()) ) {
            break;
          }
      }
      // the next text drawing will now blend the text foreground color with
      // what is really below the text background
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // Draw a line at a time instead of using QPainter::drawText(QRect,...).
    // Else, the line heights become >QFontMetrics::height() if you type Chinese
    // characters (!) and then the cursor gets out of sync.
    int baseLine = theTextAreaRect.y () + fontMetrics.ascent ();

    kpPreeditText thePreeditText = preeditText();

    if ( theTextLines.isEmpty() )
    {
        if ( ! thePreeditText.isEmpty() )
        {
            int x = theTextAreaRect.x();
            drawPreeditString(painter, x, baseLine, thePreeditText);
        }
    }
    else
    {
        int i = 0;
        int row = thePreeditText.position().y();
        int col = thePreeditText.position().x();
        for (const auto &str : theTextLines)
        {
            if (row == i && !thePreeditText.isEmpty())
            {
                QString left = str.left(col);
                QString right = str.mid(col);
                int x = theTextAreaRect.x();
                painter.drawText(x, baseLine, left);
                x += fontMetrics.horizontalAdvance(left);
                drawPreeditString(painter, x, baseLine, thePreeditText);

                painter.drawText(x, baseLine, right);
            }
            else
            {
                painter.drawText(theTextAreaRect.x (), baseLine, str);
            }
            baseLine += fontMetrics.lineSpacing();
            i++;

            // if the next textline would already be below the visible text area, stop drawing
            if ( (baseLine - fontMetrics.ascent()) > (theTextAreaRect.y() + theTextAreaRect.height()) ) {
              break;
            }
        }
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
