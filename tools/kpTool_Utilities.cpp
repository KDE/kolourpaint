
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

//
// Tool utility methods - mainly for subclasses' convenience.
//


#define DEBUG_KP_TOOL 0


#include "tools/kpTool.h"
#include "kpToolPrivate.h"

#include <QCursor>
#include <QPixmap>

#include "kpLogCategories.h"
#include <KMessageBox>

#include "commands/kpCommandSize.h"
#include "kpDefs.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "views/kpView.h"

//---------------------------------------------------------------------

// static
QRect kpTool::neededRect (const QRect &rect, int lineWidth)
{
    int x1, y1, x2, y2;
    rect.getCoords (&x1, &y1, &x2, &y2);

    if (lineWidth < 1) {
        lineWidth = 1;
    }

    // TODO: why not divide by 2?
    return QRect (QPoint (x1 - lineWidth + 1, y1 - lineWidth + 1),
                  QPoint (x2 + lineWidth - 1, y2 + lineWidth - 1));
}

//---------------------------------------------------------------------

// static
QImage kpTool::neededPixmap (const QImage &image, const QRect &boundingRect)
{
    return kpPixmapFX::getPixmapAt (image, boundingRect);
}

//---------------------------------------------------------------------

// public
bool kpTool::hasCurrentPoint () const
{
    return (viewUnderStartPoint () || viewUnderCursor ());
}

//---------------------------------------------------------------------

// public
QPoint kpTool::calculateCurrentPoint (bool zoomToDoc) const
{
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "kpTool::currentPoint(zoomToDoc=" << zoomToDoc << ")";
    qCDebug(kpLogTools) << "\tviewUnderStartPoint="
               << (viewUnderStartPoint () ? viewUnderStartPoint ()->objectName () : "(none)")
               << " viewUnderCursor="
               << (viewUnderCursor () ? viewUnderCursor ()->objectName () : "(none)")
               << endl;
#endif

    kpView *v = viewUnderStartPoint ();
    if (!v)
    {
        v = viewUnderCursor ();
        if (!v)
        {
        #if DEBUG_KP_TOOL && 0
            qCDebug(kpLogTools) << "\tno view - returning sentinel";
        #endif
            return KP_INVALID_POINT;
        }
    }


    const QPoint globalPos = QCursor::pos ();
    const QPoint viewPos = v->mapFromGlobal (globalPos);
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "\tglobalPos=" << globalPos
               << " viewPos=" << viewPos;
#endif
    if (!zoomToDoc) {
        return viewPos;
    }


    const QPoint docPos = v->transformViewToDoc (viewPos);
#if DEBUG_KP_TOOL && 0
    qCDebug(kpLogTools) << "\tdocPos=" << docPos;
#endif
    return docPos;
}

//---------------------------------------------------------------------

// public slot
void kpTool::somethingBelowTheCursorChanged ()
{
    somethingBelowTheCursorChanged (calculateCurrentPoint (),
        calculateCurrentPoint (false/*view point*/));
}

//---------------------------------------------------------------------

// private
// TODO: don't dup code from mouseMoveEvent()
void kpTool::somethingBelowTheCursorChanged (const QPoint &currentPoint_,
        const QPoint &currentViewPoint_)
{
#if DEBUG_KP_TOOL && 1
    qCDebug(kpLogTools) << "kpTool::somethingBelowTheCursorChanged(docPoint="
               << currentPoint_
               << " viewPoint="
               << currentViewPoint_
               << ")" << endl;
    qCDebug(kpLogTools) << "\tviewUnderStartPoint="
               << (viewUnderStartPoint () ? viewUnderStartPoint ()->objectName () : "(none)")
               << " viewUnderCursor="
               << (viewUnderCursor () ? viewUnderCursor ()->objectName () : "(none)")
               << endl;
    qCDebug(kpLogTools) << "\tbegan draw=" << d->beganDraw;
#endif

    d->currentPoint = currentPoint_;
    d->currentViewPoint = currentViewPoint_;

    if (d->beganDraw)
    {
        if (d->currentPoint != KP_INVALID_POINT)
        {
            draw (d->currentPoint, d->lastPoint, normalizedRect ());
            d->lastPoint = d->currentPoint;
        }
    }
    else
    {
        hover (d->currentPoint);
    }
}

//---------------------------------------------------------------------

bool kpTool::currentPointNextToLast () const
{
    if (d->lastPoint == QPoint (-1, -1)) {
        return true;
    }

    int dx = qAbs (d->currentPoint.x () - d->lastPoint.x ());
    int dy = qAbs (d->currentPoint.y () - d->lastPoint.y ());

    return (dx <= 1 && dy <= 1);
}

//---------------------------------------------------------------------

bool kpTool::currentPointCardinallyNextToLast () const
{
    if (d->lastPoint == QPoint (-1, -1)) {
        return true;
    }

    return (d->currentPoint == d->lastPoint ||
            kpPainter::pointsAreCardinallyAdjacent (d->currentPoint, d->lastPoint));
}

//---------------------------------------------------------------------

// static
// TODO: we don't handle Qt::XButton1 and Qt::XButton2 at the moment.
int kpTool::mouseButton (Qt::MouseButtons mouseButtons)
{
    // we have nothing to do with mid-buttons
    if (mouseButtons & Qt::MiddleButton) {
        return -1;
    }

    // both left & right together is quite meaningless...
    const Qt::MouseButtons bothButtons = (Qt::LeftButton | Qt::RightButton);
    if ((mouseButtons & bothButtons) == bothButtons) {
        return -1;
    }

    if (mouseButtons & Qt::LeftButton) {
        return 0;
    }
    if (mouseButtons & Qt::RightButton) {
        return 1;
    }

    return -1;
}

//---------------------------------------------------------------------

// public static
int kpTool::calculateLength (int start, int end)
{
    if (start <= end)
    {
        return end - start + 1;
    }

    return end - start - 1;
}

//---------------------------------------------------------------------

// public static
bool kpTool::warnIfBigImageSize (int oldWidth, int oldHeight,
                                 int newWidth, int newHeight,
                                 const QString &text,
                                 const QString &caption,
                                 const QString &continueButtonText,
                                 QWidget *parent)
{
#if DEBUG_KP_TOOL
    qCDebug(kpLogTools) << "kpTool::warnIfBigImageSize()"
               << " old: w=" << oldWidth << " h=" << oldWidth
               << " new: w=" << newWidth << " h=" << newHeight
               << " pixmapSize="
               << kpPixmapFX::pixmapSize (newWidth,
                                          newHeight,
                                          QPixmap::defaultDepth ())
               << " vs BigImageSize=" << KP_BIG_IMAGE_SIZE
               << endl;
#endif

    // Only got smaller or unchanged - don't complain
    if (!(newWidth > oldWidth || newHeight > oldHeight))
    {
        return true;
    }

    // Was already large - user was warned before, don't annoy him/her again
    if (kpCommandSize::PixmapSize (oldWidth, oldHeight, QPixmap::defaultDepth ()) >=
        KP_BIG_IMAGE_SIZE)
    {
        return true;
    }

    if (kpCommandSize::PixmapSize (newWidth, newHeight, QPixmap::defaultDepth ()) >=
        KP_BIG_IMAGE_SIZE)
    {
        int accept = KMessageBox::warningContinueCancel (parent,
            text,
            caption,
            KGuiItem (continueButtonText),
            KStandardGuiItem::cancel(),
            QStringLiteral ("BigImageDontAskAgain"));

        return (accept == KMessageBox::Continue);
    }

    return true;
}

//---------------------------------------------------------------------
