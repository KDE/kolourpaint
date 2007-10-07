
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptemppixmap.h>
#include <kptoolrectangle.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetfillstyle.h>
#include <kptoolwidgetlinewidth.h>
#include <kpview.h>
#include <kpviewmanager.h>

static QPixmap pixmap (const kpToolRectangle::Mode mode,
                       kpDocument *document, const QRect &rect,
                       const QPoint &startPoint, const QPoint &endPoint,
                       const QPen &pen, const QPen &maskPen,
                       const QBrush &brush, const QBrush &maskBrush)
{
    QPixmap pixmap = document->getPixmapAt (rect);
    QBitmap maskBitmap;

    QPainter painter, maskPainter;

#if DEBUG_KP_TOOL_RECTANGLE && 1
    kdDebug () << "pixmap: rect=" << rect
               << " startPoint=" << startPoint
               << " endPoint=" << endPoint
               << endl;
    kdDebug () << "\tm: p=" << (maskPen.style () != Qt::NoPen)
               << " b=" << (maskBrush.style () != Qt::NoBrush)
               << " o: p=" << (pen.style () != Qt::NoPen)
               << " b=" << (brush.style () != Qt::NoBrush)
               << endl;
    kdDebug () << "\tmaskPen.color()=" << (int *) maskPen.color ().rgb ()
               << " transparent=" << (int *) Qt::color0.rgb ()/*transparent*/
               << endl;
#endif

    if (pixmap.mask () ||
        (maskPen.style () != Qt::NoPen &&
         maskPen.color () ==  Qt::color0/*transparent*/) ||
        (maskBrush.style () != Qt::NoBrush &&
         maskBrush.color () == Qt::color0/*transparent*/))
    {
        maskBitmap = kpPixmapFX::getNonNullMask (pixmap);
        maskPainter.begin (&maskBitmap);
        maskPainter.setPen (maskPen);
        maskPainter.setBrush (maskBrush);
    }

    if (pen.style () != Qt::NoPen ||
        brush.style () != Qt::NoBrush)
    {
        painter.begin (&pixmap);
        painter.setPen (pen);
        painter.setBrush (brush);
    }

#define PAINTER_CALL(cmd)         \
{                                 \
    if (painter.isActive ())      \
        painter . cmd ;           \
                                  \
    if (maskPainter.isActive ())  \
        maskPainter . cmd ;       \
}

    if (startPoint != endPoint)
    {
    #if DEBUG_KP_TOOL_RECTANGLE && 1
        kdDebug () << "\tdraw shape" << endl;
    #endif

        // TODO: Rectangle of pen width 1, height 1 and width X is rendered
        //       as width X - 1.
        switch (mode)
        {
        case kpToolRectangle::Rectangle:
            PAINTER_CALL (drawRect (QRect (startPoint - rect.topLeft (), endPoint - rect.topLeft ())));
            break;
        case kpToolRectangle::RoundedRectangle:
            PAINTER_CALL (drawRoundRect (QRect (startPoint - rect.topLeft (), endPoint - rect.topLeft ())));
            break;
        case kpToolRectangle::Ellipse:
            PAINTER_CALL (drawEllipse (QRect (startPoint - rect.topLeft (), endPoint - rect.topLeft ())));
            break;
        default:
            kdError () << "kptoolrectangle.cpp::pixmap() passed unknown mode: " << int (mode) << endl;
            break;
        }
    }
    else
    {
    #if DEBUG_KP_TOOL_RECTANGLE && 1
        kdDebug () << "\tstartPoint == endPoint" << endl;
    #endif
        // SYNC: Work around Qt bug: can't draw 1x1 rectangle
        // Not strictly correct for border width > 1
        // but better than not drawing at all
        PAINTER_CALL (drawPoint (startPoint - rect.topLeft ()));
    }
#undef PAINTER_CALL

    if (painter.isActive ())
        painter.end ();

    if (maskPainter.isActive ())
        maskPainter.end ();

    if (!maskBitmap.isNull ())
        pixmap.setMask (maskBitmap);

    return pixmap;
}


/*
 * kpToolRectangle
 */

kpToolRectangle::kpToolRectangle (Mode mode,
                                  const QString &text,
                                  const QString &description,
                                  int key,
                                  kpMainWindow *mainWindow,
                                  const char *name)
    : kpTool (text, description, key, mainWindow, name),
      m_mode (mode),
      m_toolWidgetLineWidth (0),
      m_toolWidgetFillStyle (0)
{
}

kpToolRectangle::kpToolRectangle (kpMainWindow *mainWindow)
    : kpTool (i18n ("Rectangle"), i18n ("Draws rectangles and squares"),
              Qt::Key_R,
              mainWindow, "tool_rectangle"),
      m_mode (Rectangle),
      m_toolWidgetLineWidth (0),
      m_toolWidgetFillStyle (0)
{
}

kpToolRectangle::~kpToolRectangle ()
{
}

void kpToolRectangle::setMode (Mode mode)
{
    m_mode = mode;
}


// private
void kpToolRectangle::updatePens ()
{
    for (int i = 0; i < 2; i++)
        updatePen (i);
}

// private
void kpToolRectangle::updateBrushes ()
{
    for (int i = 0; i < 2; i++)
        updateBrush (i);
}

// virtual private slot
void kpToolRectangle::slotForegroundColorChanged (const kpColor &)
{
#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "kpToolRectangle::slotForegroundColorChanged()" << endl;
#endif
    updatePen (0);
    updateBrush (0);  // brush may be in foreground color
    updateBrush (1);
}

// virtual private slot
void kpToolRectangle::slotBackgroundColorChanged (const kpColor &)
{
#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "kpToolRectangle::slotBackgroundColorChanged()" << endl;
    kdDebug () << "\tm_toolWidgetFillStyle=" << m_toolWidgetFillStyle << endl;
#endif
    updatePen (1);
    updateBrush (0);
    updateBrush (1);  // brush may be in background color
}

// private
void kpToolRectangle::updatePen (int mouseButton)
{
    QColor maskPenColor = color (mouseButton).maskColor ();

    if (!m_toolWidgetLineWidth)
    {
        if (color (mouseButton).isOpaque ())
            m_pen [mouseButton] = QPen (color (mouseButton).toQColor ());
        else
            m_pen [mouseButton] = Qt::NoPen;
        m_maskPen [mouseButton] = QPen (maskPenColor);
    }
    else
    {
        if (color (mouseButton).isOpaque ())
        {
            m_pen [mouseButton] = QPen (color (mouseButton).toQColor (),
                                        m_toolWidgetLineWidth->lineWidth (),
                                        Qt::SolidLine);
        }
        else
            m_pen [mouseButton] = Qt::NoPen;
        m_maskPen [mouseButton] = QPen (maskPenColor,
                                        m_toolWidgetLineWidth->lineWidth (),
                                        Qt::SolidLine);
    }
}

void kpToolRectangle::updateBrush (int mouseButton)
{
#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "kpToolRectangle::brush ()  mouseButton=" << mouseButton
               << " m_toolWidgetFillStyle=" << m_toolWidgetFillStyle
               << endl;
#endif
    if (m_toolWidgetFillStyle)
    {
        m_brush [mouseButton] = m_toolWidgetFillStyle->brush (
            color (mouseButton)/*foreground colour*/,
            color (1 - mouseButton)/*background colour*/);

        m_maskBrush [mouseButton] = m_toolWidgetFillStyle->maskBrush (
            color (mouseButton)/*foreground colour*/,
            color (1 - mouseButton)/*background colour*/);
    }
    else
    {
        m_brush [mouseButton] = Qt::NoBrush;
        m_maskBrush [mouseButton] = Qt::NoBrush;
    }
}


// private slot virtual
void kpToolRectangle::slotLineWidthChanged ()
{
    updatePens ();

    if (hasBegunDraw ())
        updateShape ();
}

// private slot virtual
void kpToolRectangle::slotFillStyleChanged ()
{
    updateBrushes ();

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
    kdDebug () << "kpToolRectangle::begin ()" << endl;
#endif

    kpToolToolBar *tb = toolToolBar ();

#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "\ttoolToolBar=" << tb << endl;
#endif

    if (tb)
    {
        m_toolWidgetLineWidth = tb->toolWidgetLineWidth ();
        connect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                 this, SLOT (slotLineWidthChanged ()));
        m_toolWidgetLineWidth->show ();

        updatePens ();


        m_toolWidgetFillStyle = tb->toolWidgetFillStyle ();
        connect (m_toolWidgetFillStyle, SIGNAL (fillStyleChanged (kpToolWidgetFillStyle::FillStyle)),
                 this, SLOT (slotFillStyleChanged ()));
        m_toolWidgetFillStyle->show ();

        updateBrushes ();
    }

#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "\t\tm_toolWidgetFillStyle=" << m_toolWidgetFillStyle << endl;
#endif

    viewManager ()->setCursor (QCursor (CrossCursor));

    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolRectangle::end ()
{
#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "kpToolRectangle::end ()" << endl;
#endif

    if (m_toolWidgetLineWidth)
    {
        disconnect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                    this, SLOT (slotLineWidthChanged ()));
        m_toolWidgetLineWidth = 0;
    }

    if (m_toolWidgetFillStyle)
    {
        disconnect (m_toolWidgetFillStyle, SIGNAL (fillStyleChanged (kpToolWidgetFillStyle::FillStyle)),
                   this, SLOT (slotFillStyleChanged ()));
        m_toolWidgetFillStyle = 0;
    }

    viewManager ()->unsetCursor ();
}

void kpToolRectangle::applyModifiers ()
{
    QRect rect = QRect (m_startPoint, m_currentPoint).normalize ();

#if DEBUG_KP_TOOL_RECTANGLE
    kdDebug () << "kpToolRectangle::applyModifiers(" << rect
               << ") shift=" << m_shiftPressed
               << " ctrl=" << m_controlPressed
               << endl;
#endif

    // user wants to m_startPoint == centre
    if (m_controlPressed)
    {
        int xdiff = kAbs (m_startPoint.x () - m_currentPoint.x ());
        int ydiff = kAbs (m_startPoint.y () - m_currentPoint.y ());
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

    m_toolRectangleStartPoint = rect.topLeft ();
    m_toolRectangleEndPoint = rect.bottomRight ();

    m_toolRectangleRectWithoutLineWidth = rect;
    m_toolRectangleRect = kpTool::neededRect (rect, QMAX (m_pen [m_mouseButton].width (),
                                                          m_maskPen [m_mouseButton].width ()));
}

void kpToolRectangle::beginDraw ()
{
    setUserMessage (cancelUserMessage ());
}

void kpToolRectangle::updateShape ()
{
    viewManager ()->setFastUpdates ();

    QPixmap newPixmap = pixmap (m_mode, document (), m_toolRectangleRect,
                                m_toolRectangleStartPoint, m_toolRectangleEndPoint,
                                m_pen [m_mouseButton], m_maskPen [m_mouseButton],
                                m_brush [m_mouseButton], m_maskBrush [m_mouseButton]);
    kpTempPixmap newTempPixmap (false/*always display*/,
                                kpTempPixmap::SetPixmap/*render mode*/,
                                m_toolRectangleRect.topLeft (),
                                newPixmap);
    viewManager ()->setTempPixmap (newTempPixmap);

    viewManager ()->restoreFastUpdates ();
}

void kpToolRectangle::draw (const QPoint &, const QPoint &, const QRect &)
{
    applyModifiers ();


    updateShape ();


    // Recover the start and end points from the transformed & normalized m_toolRectangleRect

    // S. or S or SC or S == C
    // .C    C
    if (m_currentPoint.x () >= m_startPoint.x () &&
        m_currentPoint.y () >= m_startPoint.y ())
    {
        setUserShapePoints (m_toolRectangleRectWithoutLineWidth.topLeft (),
                            m_toolRectangleRectWithoutLineWidth.bottomRight ());
    }
    // .C or C
    // S.    S
    else if (m_currentPoint.x () >= m_startPoint.x () &&
             m_currentPoint.y () < m_startPoint.y ())
    {
        setUserShapePoints (m_toolRectangleRectWithoutLineWidth.bottomLeft (),
                            m_toolRectangleRectWithoutLineWidth.topRight ());
    }
    // .S or CS
    // C.
    else if (m_currentPoint.x () < m_startPoint.x () &&
             m_currentPoint.y () >= m_startPoint.y ())
    {
        setUserShapePoints (m_toolRectangleRectWithoutLineWidth.topRight (),
                            m_toolRectangleRectWithoutLineWidth.bottomLeft ());
    }
    // C.
    // .S
    else
    {
        setUserShapePoints (m_toolRectangleRectWithoutLineWidth.bottomRight (),
                            m_toolRectangleRectWithoutLineWidth.topLeft ());
    }
}

void kpToolRectangle::cancelShape ()
{
#if 0
    endDraw (m_currentPoint, QRect (m_startPoint, m_currentPoint).normalize ());
    mainWindow ()->commandHistory ()->undo ();
#else
    viewManager ()->invalidateTempPixmap ();
#endif

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
        new kpToolRectangleCommand
            (m_mode,
             m_pen [m_mouseButton], m_maskPen [m_mouseButton],
             m_brush [m_mouseButton], m_maskBrush [m_mouseButton],
             m_toolRectangleRect, m_toolRectangleStartPoint, m_toolRectangleEndPoint,
             mainWindow ()));

    setUserMessage (haventBegunDrawUserMessage ());
}


/*
 * kpToolRectangleCommand
 */

kpToolRectangleCommand::kpToolRectangleCommand (kpToolRectangle::Mode mode,
                                                const QPen &pen, const QPen &maskPen,
                                                const QBrush &brush, const QBrush &maskBrush,
                                                const QRect &rect,
                                                const QPoint &startPoint, const QPoint &endPoint,
                                                kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_mode (mode),
      m_pen (pen), m_maskPen (maskPen),
      m_brush (brush), m_maskBrush (maskBrush),
      m_rect (rect),
      m_startPoint (startPoint),
      m_endPoint (endPoint),
      m_oldPixmapPtr (0)
{
}

kpToolRectangleCommand::~kpToolRectangleCommand ()
{
    delete m_oldPixmapPtr;
}


// public virtual [base kpCommand]
QString kpToolRectangleCommand::name () const
{
    switch (m_mode)
    {
    case kpToolRectangle::Rectangle:
        return i18n ("Rectangle");
    case kpToolRectangle::RoundedRectangle:
        return i18n ("Rounded Rectangle");
    case kpToolRectangle::Ellipse:
        return i18n ("Ellipse");
    default:
        kdError () << "kpToolRectangleCommand::name() passed unknown mode: " << int (m_mode) << endl;
        return QString::null;
    }
}


// public virtual [base kpCommand]
int kpToolRectangleCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmapPtr);
}


// public virtual [base kpCommand]
void kpToolRectangleCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    // store Undo info
    if (!m_oldPixmapPtr)
    {
        // OPT: I can do better with no brush
        m_oldPixmapPtr = new QPixmap ();
        *m_oldPixmapPtr = doc->getPixmapAt (m_rect);
    }
    else
        kdError () << "kpToolRectangleCommand::execute() m_oldPixmapPtr not null" << endl;

    doc->setPixmapAt (pixmap (m_mode, doc,
                              m_rect, m_startPoint, m_endPoint,
                              m_pen, m_maskPen,
                              m_brush, m_maskBrush),
                      m_rect.topLeft ());
}

// public virtual [base kpCommand]
void kpToolRectangleCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    if (m_oldPixmapPtr)
    {
        doc->setPixmapAt (*m_oldPixmapPtr, m_rect.topLeft ());

        delete m_oldPixmapPtr;
        m_oldPixmapPtr = 0;
    }
    else
        kdError () << "kpToolRectangleCommand::unexecute() m_oldPixmapPtr null" << endl;
}

#include <kptoolrectangle.moc>
