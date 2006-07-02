
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_RECTANGLE 0


#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppainter.h>
#include <kppixmapfx.h>
#include <kptemppixmap.h>
#include <kptoolrectangle.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetfillstyle.h>
#include <kptoolwidgetlinewidth.h>
#include <kpview.h>
#include <kpviewmanager.h>


static kpImage image (const kpToolRectangle::Mode mode,
                       kpDocument *document, const QRect &rect,
                       const kpColor &fcolor, int penWidth, const kpColor &bcolor)
{
#if DEBUG_KP_TOOL_RECTANGLE && 1
    kDebug () << "image: rect=" << rect
               << endl;
    kDebug () << "\tfcolor=" << fcolor.toQRgb ()
              << " penWidth=" << penWidth
              << " bcolor=" << (bcolor.isValid () ? fcolor.toQRgb () : 0xabadcafe)
              << endl;
#endif

    kpImage image = document->getPixmapAt (rect);

    switch (mode)
    {
    case kpToolRectangle::Rectangle:
        kpPainter::drawRect (&image,
            0, 0, rect.width (), rect.height (),
            fcolor, penWidth, bcolor);
        break;

    case kpToolRectangle::RoundedRectangle:
        kpPainter::drawRoundedRect (&image,
            0, 0, rect.width (), rect.height (),
            fcolor, penWidth, bcolor);
        break;

    case kpToolRectangle::Ellipse:
        kpPainter::drawEllipse (&image,
            0, 0, rect.width (), rect.height (),
            fcolor, penWidth, bcolor);
        break;

    default:
        kError () << "kptoolrectangle.cpp::image() passed unknown mode: "
                    << int (mode) << endl;
        break;
    }

    return image;
}


//
// kpToolRectangle
//

struct kpToolRectanglePrivate
{
    kpToolRectangle::Mode mode;

    kpToolWidgetLineWidth *toolWidgetLineWidth;
    kpToolWidgetFillStyle *toolWidgetFillStyle;

    QRect toolRectangleRect;
};


kpToolRectangle::kpToolRectangle (Mode mode,
                                  const QString &text,
                                  const QString &description,
                                  int key,
                                  kpMainWindow *mainWindow,
                                  const char *name)
    : kpTool (text, description, key, mainWindow, name),
      d (new kpToolRectanglePrivate ())
{
    d->mode = mode;
    d->toolWidgetLineWidth = 0, d->toolWidgetFillStyle = 0;
}

kpToolRectangle::kpToolRectangle (kpMainWindow *mainWindow)
    : kpTool (i18n ("Rectangle"), i18n ("Draws rectangles and squares"),
              Qt::Key_R,
              mainWindow, "tool_rectangle"),
      d (new kpToolRectanglePrivate ())
{
    d->mode = Rectangle;
    d->toolWidgetLineWidth = 0, d->toolWidgetFillStyle = 0;
}

kpToolRectangle::~kpToolRectangle ()
{
    delete d;
}

void kpToolRectangle::setMode (Mode mode)
{
    d->mode = mode;
}


// private slot virtual
void kpToolRectangle::slotLineWidthChanged ()
{
    if (hasBegunDraw ())
        updateShape ();
}

// private slot virtual
void kpToolRectangle::slotFillStyleChanged ()
{
    if (hasBegunDraw ())
        updateShape ();
}


// private
QString kpToolRectangle::haventBegunDrawUserMessage () const
{
    return i18n ("Drag to draw.");
}

// virtual
void kpToolRectangle::begin ()
{
#if DEBUG_KP_TOOL_RECTANGLE
    kDebug () << "kpToolRectangle::begin ()" << endl;
#endif

    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

#if DEBUG_KP_TOOL_RECTANGLE
    kDebug () << "\ttoolToolBar=" << tb << endl;
#endif

    d->toolWidgetLineWidth = tb->toolWidgetLineWidth ();
    connect (d->toolWidgetLineWidth,
        SIGNAL (lineWidthChanged (int)),
        this,
        SLOT (slotLineWidthChanged ()));
    d->toolWidgetLineWidth->show ();

    d->toolWidgetFillStyle = tb->toolWidgetFillStyle ();
    connect (d->toolWidgetFillStyle,
        SIGNAL (fillStyleChanged (kpToolWidgetFillStyle::FillStyle)),
        this,
        SLOT (slotFillStyleChanged ()));
    d->toolWidgetFillStyle->show ();

    viewManager ()->setCursor (QCursor (Qt::CrossCursor));

    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolRectangle::end ()
{
#if DEBUG_KP_TOOL_RECTANGLE
    kDebug () << "kpToolRectangle::end ()" << endl;
#endif

    if (d->toolWidgetLineWidth)
    {
        disconnect (d->toolWidgetLineWidth,
            SIGNAL (lineWidthChanged (int)),
            this,
            SLOT (slotLineWidthChanged ()));
        d->toolWidgetLineWidth = 0;
    }

    if (d->toolWidgetFillStyle)
    {
        disconnect (d->toolWidgetFillStyle,
            SIGNAL (fillStyleChanged (kpToolWidgetFillStyle::FillStyle)),
            this,
            SLOT (slotFillStyleChanged ()));
        d->toolWidgetFillStyle = 0;
    }

    viewManager ()->unsetCursor ();
}

void kpToolRectangle::applyModifiers ()
{
    QRect rect = kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint));

#if DEBUG_KP_TOOL_RECTANGLE
    kDebug () << "kpToolRectangle::applyModifiers(" << rect
               << ") shift=" << m_shiftPressed
               << " ctrl=" << m_controlPressed
               << endl;
#endif

    // user wants to m_startPoint == centre
    if (m_controlPressed)
    {
        int xdiff = qAbs (m_startPoint.x () - m_currentPoint.x ());
        int ydiff = qAbs (m_startPoint.y () - m_currentPoint.y ());
        rect = QRect (m_startPoint.x () - xdiff, m_startPoint.y () - ydiff,
                      xdiff * 2 + 1, ydiff * 2 + 1);
    }

    // user wants major axis == minor axis:
    //   rectangle --> square
    //   rounded rectangle --> rounded square
    //   ellipse --> circle
    if (m_shiftPressed)
    {
        if (!m_controlPressed)
        {
            if (rect.width () < rect.height ())
            {
                if (m_startPoint.y () == rect.y ())
                    rect.setHeight (rect.width ());
                else
                    rect.setY (rect.bottom () - rect.width () + 1);
            }
            else
            {
                if (m_startPoint.x () == rect.x ())
                    rect.setWidth (rect.height ());
                else
                    rect.setX (rect.right () - rect.height () + 1);
            }
        }
        // have to maintain the centre
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

void kpToolRectangle::beginDraw ()
{
    setUserMessage (cancelUserMessage ());
}


// private
kpColor kpToolRectangle::drawingForegroundColor () const
{
    return color (m_mouseButton);
}

// private
kpColor kpToolRectangle::drawingBackgroundColor () const
{
    const kpColor foregroundColor = color (m_mouseButton);
    const kpColor backgroundColor = color (1 - m_mouseButton);

    return d->toolWidgetFillStyle->drawingBackgroundColor (
        foregroundColor, backgroundColor);
}

// private
void kpToolRectangle::updateShape ()
{
    viewManager ()->setFastUpdates ();

    QPixmap newPixmap = ::image (d->mode, document (), d->toolRectangleRect,
        drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
        drawingBackgroundColor ());
    kpTempPixmap newTempPixmap (false/*always display*/,
                                kpTempPixmap::SetPixmap/*render mode*/,
                                d->toolRectangleRect.topLeft (),
                                newPixmap);
    viewManager ()->setTempPixmap (newTempPixmap);

    viewManager ()->restoreFastUpdates ();
}


void kpToolRectangle::draw (const QPoint &, const QPoint &, const QRect &)
{
    applyModifiers ();


    updateShape ();


    // Recover the start and end points from the transformed & normalized d->toolRectangleRect

    // S. or S or SC or S == C
    // .C    C
    if (m_currentPoint.x () >= m_startPoint.x () &&
        m_currentPoint.y () >= m_startPoint.y ())
    {
        setUserShapePoints (d->toolRectangleRect.topLeft (),
                            d->toolRectangleRect.bottomRight ());
    }
    // .C or C
    // S.    S
    else if (m_currentPoint.x () >= m_startPoint.x () &&
             m_currentPoint.y () < m_startPoint.y ())
    {
        setUserShapePoints (d->toolRectangleRect.bottomLeft (),
                            d->toolRectangleRect.topRight ());
    }
    // .S or CS
    // C.
    else if (m_currentPoint.x () < m_startPoint.x () &&
             m_currentPoint.y () >= m_startPoint.y ())
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

void kpToolRectangle::cancelShape ()
{
    viewManager ()->invalidateTempPixmap ();

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolRectangle::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

void kpToolRectangle::endDraw (const QPoint &, const QRect &)
{
    applyModifiers ();

    // TODO: flicker
    viewManager ()->invalidateTempPixmap ();

    mainWindow ()->commandHistory ()->addCommand (
        new kpToolRectangleCommand (d->mode, d->toolRectangleRect,
            drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
            drawingBackgroundColor (),
            mainWindow ()));

    setUserMessage (haventBegunDrawUserMessage ());
}


//
// kpToolRectangleCommand
//

struct kpToolRectangleCommandPrivate
{
    kpToolRectangle::Mode mode;
    QRect rect;
    kpColor fcolor;
    int penWidth;
    kpColor bcolor;
    QPixmap *oldPixmapPtr;
};

kpToolRectangleCommand::kpToolRectangleCommand (kpToolRectangle::Mode mode,
        const QRect &rect,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      d (new kpToolRectangleCommandPrivate ())
{
    d->mode = mode;
    d->rect = rect;
    d->fcolor = fcolor;
    d->penWidth = penWidth;
    d->bcolor = bcolor;
    d->oldPixmapPtr = 0;
}

kpToolRectangleCommand::~kpToolRectangleCommand ()
{
    delete d->oldPixmapPtr;
    delete d;
}


// public virtual [base kpCommand]
QString kpToolRectangleCommand::name () const
{
    switch (d->mode)
    {
    case kpToolRectangle::Rectangle:
        return i18n ("Rectangle");
    case kpToolRectangle::RoundedRectangle:
        return i18n ("Rounded Rectangle");
    case kpToolRectangle::Ellipse:
        return i18n ("Ellipse");
    default:
        kError () << "kpToolRectangleCommand::name() passed unknown mode: "
            << int (d->mode) << endl;
        return QString::null;
    }
}


// public virtual [base kpCommand]
int kpToolRectangleCommand::size () const
{
    return kpPixmapFX::pixmapSize (d->oldPixmapPtr);
}


// public virtual [base kpCommand]
void kpToolRectangleCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    // store Undo info
    if (!d->oldPixmapPtr)
    {
        // OPT: I can do better with no brush
        d->oldPixmapPtr = new QPixmap ();
        *d->oldPixmapPtr = doc->getPixmapAt (d->rect);
    }
    else
        kError () << "kpToolRectangleCommand::execute() d->oldPixmapPtr not null" << endl;

    doc->setPixmapAt (
        ::image (d->mode, doc, d->rect, d->fcolor, d->penWidth, d->bcolor),
        d->rect.topLeft ());
}

// public virtual [base kpCommand]
void kpToolRectangleCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    if (d->oldPixmapPtr)
    {
        doc->setPixmapAt (*d->oldPixmapPtr, d->rect.topLeft ());

        delete d->oldPixmapPtr;
        d->oldPixmapPtr = 0;
    }
    else
        kError () << "kpToolRectangleCommand::unexecute() m_oldPixmapPtr null" << endl;
}


#include <kptoolrectangle.moc>
