
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


#define DEBUG_KP_TOOL_RECTANGULAR_BASE 0


#include "tools/rectangular/kpToolRectangularBase.h"

#include <QCursor>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "imagelib/kpColor.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "layers/tempImage/kpTempImage.h"
#include "environments/tools/kpToolEnvironment.h"
#include "commands/tools/rectangular/kpToolRectangularCommand.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetFillStyle.h"
#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"


//---------------------------------------------------------------------

struct kpToolRectangularBasePrivate
{
    kpToolRectangularBase::DrawShapeFunc drawShapeFunc{};

    kpToolWidgetLineWidth *toolWidgetLineWidth{};
    kpToolWidgetFillStyle *toolWidgetFillStyle{};

    QRect toolRectangleRect;
};

//---------------------------------------------------------------------

kpToolRectangularBase::kpToolRectangularBase (
        const QString &text,
        const QString &description,
        DrawShapeFunc drawShapeFunc,
        int key,
        kpToolEnvironment *environ, QObject *parent,
        const QString &name)

    : kpTool (text, description, key, environ, parent, name),
      d (new kpToolRectangularBasePrivate ())
{
    d->drawShapeFunc = drawShapeFunc;

    d->toolWidgetLineWidth = nullptr;
    d->toolWidgetFillStyle = nullptr;
}

//---------------------------------------------------------------------

kpToolRectangularBase::~kpToolRectangularBase ()
{
    delete d;
}

//---------------------------------------------------------------------


// private slot virtual
void kpToolRectangularBase::slotLineWidthChanged ()
{
    if (hasBegunDraw ()) {
        updateShape ();
    }
}

//---------------------------------------------------------------------

// private slot virtual
void kpToolRectangularBase::slotFillStyleChanged ()
{
    if (hasBegunDraw ()) {
        updateShape ();
    }
}

//---------------------------------------------------------------------

// private
QString kpToolRectangularBase::haventBegunDrawUserMessage () const
{
    return i18n ("Drag to draw.");
}

//---------------------------------------------------------------------

// virtual
void kpToolRectangularBase::begin ()
{
#if DEBUG_KP_TOOL_RECTANGULAR_BASE
    qCDebug(kpLogTools) << "kpToolRectangularBase::begin ()";
#endif

    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

#if DEBUG_KP_TOOL_RECTANGULAR_BASE
    qCDebug(kpLogTools) << "\ttoolToolBar=" << tb;
#endif

    d->toolWidgetLineWidth = tb->toolWidgetLineWidth ();
    connect (d->toolWidgetLineWidth, &kpToolWidgetLineWidth::lineWidthChanged,
             this, &kpToolRectangularBase::slotLineWidthChanged);
    d->toolWidgetLineWidth->show ();

    d->toolWidgetFillStyle = tb->toolWidgetFillStyle ();
    connect (d->toolWidgetFillStyle, &kpToolWidgetFillStyle::fillStyleChanged,
             this, &kpToolRectangularBase::slotFillStyleChanged);
    d->toolWidgetFillStyle->show ();

    viewManager ()->setCursor (QCursor (Qt::ArrowCursor));

    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// virtual
void kpToolRectangularBase::end ()
{
#if DEBUG_KP_TOOL_RECTANGULAR_BASE
    qCDebug(kpLogTools) << "kpToolRectangularBase::end ()";
#endif

    if (d->toolWidgetLineWidth)
    {
        disconnect (d->toolWidgetLineWidth, &kpToolWidgetLineWidth::lineWidthChanged,
                 this, &kpToolRectangularBase::slotLineWidthChanged);
        d->toolWidgetLineWidth = nullptr;
    }

    if (d->toolWidgetFillStyle)
    {
        disconnect (d->toolWidgetFillStyle, &kpToolWidgetFillStyle::fillStyleChanged,
                 this, &kpToolRectangularBase::slotFillStyleChanged);
        d->toolWidgetFillStyle = nullptr;
    }

    viewManager ()->unsetCursor ();
}

//---------------------------------------------------------------------

void kpToolRectangularBase::applyModifiers ()
{
    QRect rect = normalizedRect ();

#if DEBUG_KP_TOOL_RECTANGULAR_BASE
    qCDebug(kpLogTools) << "kpToolRectangularBase::applyModifiers(" << rect
               << ") shift=" << shiftPressed ()
               << " ctrl=" << controlPressed ()
               << endl;
#endif

    // user wants to startPoint () == center
    if (controlPressed ())
    {
        int xdiff = qAbs (startPoint ().x () - currentPoint ().x ());
        int ydiff = qAbs (startPoint ().y () - currentPoint ().y ());
        rect = QRect (startPoint ().x () - xdiff, startPoint ().y () - ydiff,
                      xdiff * 2 + 1, ydiff * 2 + 1);
    }

    // user wants major axis == minor axis:
    //   rectangle --> square
    //   rounded rectangle --> rounded square
    //   ellipse --> circle
    if (shiftPressed ())
    {
        if (!controlPressed ())
        {
            if (rect.width () < rect.height ())
            {
                if (startPoint ().y () == rect.y ()) {
                    rect.setHeight (rect.width ());
                }
                else {
                    rect.setY (rect.bottom () - rect.width () + 1);
                }
            }
            else
            {
                if (startPoint ().x () == rect.x ()) {
                    rect.setWidth (rect.height ());
                }
                else {
                    rect.setX (rect.right () - rect.height () + 1);
                }
            }
        }
        // have to maintain the center
        else
        {
            if (rect.width () < rect.height ())
            {
                QPoint center = rect.center ();
                rect.setHeight (rect.width ());
                rect.moveCenter (center);
            }
            else
            {
                QPoint center = rect.center ();
                rect.setWidth (rect.height ());
                rect.moveCenter (center);
            }
        }
    }

    d->toolRectangleRect = rect;
}

//---------------------------------------------------------------------

void kpToolRectangularBase::beginDraw ()
{
    setUserMessage (cancelUserMessage ());
}

//---------------------------------------------------------------------


// private
kpColor kpToolRectangularBase::drawingForegroundColor () const
{
    return color (mouseButton ());
}

//---------------------------------------------------------------------

// private
kpColor kpToolRectangularBase::drawingBackgroundColor () const
{
    const kpColor foregroundColor = color (mouseButton ());
    const kpColor backgroundColor = color (1 - mouseButton ());

    return d->toolWidgetFillStyle->drawingBackgroundColor (
        foregroundColor, backgroundColor);
}

//---------------------------------------------------------------------

// private
void kpToolRectangularBase::updateShape ()
{
    kpImage image = document ()->getImageAt (d->toolRectangleRect);

    // Invoke shape drawing function passed in ctor.
    (*d->drawShapeFunc) (&image,
        0, 0, d->toolRectangleRect.width (), d->toolRectangleRect.height (),
        drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
        drawingBackgroundColor ());

    kpTempImage newTempImage (false/*always display*/,
                                kpTempImage::SetImage/*render mode*/,
                                d->toolRectangleRect.topLeft (),
                                image);

    viewManager ()->setFastUpdates ();
    viewManager ()->setTempImage (newTempImage);
    viewManager ()->restoreFastUpdates ();
}

//---------------------------------------------------------------------

void kpToolRectangularBase::draw (const QPoint &, const QPoint &, const QRect &)
{
    applyModifiers ();


    updateShape ();


    // Recover the start and end points from the transformed & normalized d->toolRectangleRect

    // S. or S or SC or S == C
    // .C    C
    if (currentPoint ().x () >= startPoint ().x () &&
        currentPoint ().y () >= startPoint ().y ())
    {
        setUserShapePoints (d->toolRectangleRect.topLeft (),
                            d->toolRectangleRect.bottomRight ());
    }
    // .C or C
    // S.    S
    else if (currentPoint ().x () >= startPoint ().x () &&
             currentPoint ().y () < startPoint ().y ())
    {
        setUserShapePoints (d->toolRectangleRect.bottomLeft (),
                            d->toolRectangleRect.topRight ());
    }
    // .S or CS
    // C.
    else if (currentPoint ().x () < startPoint ().x () &&
             currentPoint ().y () >= startPoint ().y ())
    {
        setUserShapePoints (d->toolRectangleRect.topRight (),
                            d->toolRectangleRect.bottomLeft ());
    }
    // C.
    // .S
    else
    {
        setUserShapePoints (d->toolRectangleRect.bottomRight (),
                            d->toolRectangleRect.topLeft ());
    }
}

//---------------------------------------------------------------------

void kpToolRectangularBase::cancelShape ()
{
    viewManager ()->invalidateTempImage ();

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

//---------------------------------------------------------------------

void kpToolRectangularBase::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

void kpToolRectangularBase::endDraw (const QPoint &, const QRect &)
{
    applyModifiers ();

    // TODO: flicker
    // Later: So why can't we use kpViewManager::setQueueUpdates()?  Check SVN
    //        log to see if this method was not available at the time of the
    //        TODO, hence justifying the TODO.
    // Later2: kpToolPolygonalBase, and perhaps, other shapes will have the
    //         same problem.
    viewManager ()->invalidateTempImage ();

    environ ()->commandHistory ()->addCommand (
        new kpToolRectangularCommand (
            text (),
            d->drawShapeFunc, d->toolRectangleRect,
            drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
            drawingBackgroundColor (),
            environ ()->commandEnvironment ()));

    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------


