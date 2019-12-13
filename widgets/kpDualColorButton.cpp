
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


#define DEBUG_KP_DUAL_COLOR_BUTTON 0


#include "kpDualColorButton.h"

#include "views/kpView.h"

#include <KColorMimeData>
#include "kpLogCategories.h"

#include <QApplication>
#include <QBitmap>
#include <QColorDialog>
#include <QDrag>
#include <QMouseEvent>
#include <QPainter>
#include <qdrawutil.h>

//---------------------------------------------------------------------

kpDualColorButton::kpDualColorButton (QWidget *parent)
    : QFrame (parent),
      m_dragStartPoint (KP_INVALID_POINT)
{
    setSizePolicy (QSizePolicy::Fixed/*horizontal*/,
                   QSizePolicy::Fixed/*vertical*/);
    setFrameStyle (QFrame::Panel | QFrame::Sunken);

    m_color [0] = kpColor (0, 0, 0);  // black
    m_color [1] = kpColor (255, 255, 255);  // white

    setAcceptDrops (true);
}

//---------------------------------------------------------------------

kpColor kpDualColorButton::color (int which) const
{
    Q_ASSERT (which == 0 || which == 1);

    return m_color [which];
}

//---------------------------------------------------------------------

kpColor kpDualColorButton::foregroundColor () const
{
    return color (0);
}

//---------------------------------------------------------------------

kpColor kpDualColorButton::backgroundColor () const
{
    return color (1);
}

//---------------------------------------------------------------------

void kpDualColorButton::setColor (int which, const kpColor &color)
{
    Q_ASSERT (which == 0 || which == 1);

    if (m_color [which] == color) {
        return;
    }

    m_oldColor [which] = m_color [which];
    m_color [which] = color;
    update ();

    if (which == 0) {
        emit foregroundColorChanged (color);
    }
    else {
        emit backgroundColorChanged (color);
    }
}

//---------------------------------------------------------------------

void kpDualColorButton::setForegroundColor (const kpColor &color)
{
    setColor (0, color);
}

//---------------------------------------------------------------------

void kpDualColorButton::setBackgroundColor (const kpColor &color)
{
    setColor (1, color);
}

//---------------------------------------------------------------------


// public
kpColor kpDualColorButton::oldForegroundColor () const
{
    return m_oldColor [0];
}

//---------------------------------------------------------------------

// public
kpColor kpDualColorButton::oldBackgroundColor () const
{
    return m_oldColor [1];
}

//---------------------------------------------------------------------


// public virtual [base QWidget]
QSize kpDualColorButton::sizeHint () const
{
    return  {52, 52};
}

//---------------------------------------------------------------------


// protected
QRect kpDualColorButton::swapPixmapRect () const
{
    QPixmap swapPixmap = QStringLiteral(":/icons/colorbutton_swap_16x16");

    return  {contentsRect ().width () - swapPixmap.width (), 0,
                swapPixmap.width (), swapPixmap.height ()};
}

//---------------------------------------------------------------------

// protected
QRect kpDualColorButton::foregroundBackgroundRect () const
{
    QRect cr (contentsRect ());
    return  {cr.width () / 8, cr.height () / 8,
                cr.width () * 6 / 8, cr.height () * 6 / 8};
}

//---------------------------------------------------------------------

// protected
QRect kpDualColorButton::foregroundRect () const
{
    QRect fbr (foregroundBackgroundRect ());
    return  {fbr.x (), fbr.y (),
                fbr.width () * 3 / 4, fbr.height () * 3 / 4};
}

//---------------------------------------------------------------------

// protected
QRect kpDualColorButton::backgroundRect () const
{
    QRect fbr (foregroundBackgroundRect ());
    return  {fbr.x () + fbr.width () / 4, fbr.y () + fbr.height () / 4,
                fbr.width () * 3 / 4, fbr.height () * 3 / 4};
}

//---------------------------------------------------------------------


// protected virtual
void kpDualColorButton::dragEnterEvent (QDragEnterEvent *e)
{
#if DEBUG_KP_DUAL_COLOR_BUTTON
    qCDebug(kpLogWidgets) << "kpDualColorButton::dragEnterEvent() canDecode="
              << KColorMimeData::canDecode (e->mimeData ());
#endif
    e->accept ();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpDualColorButton::dragMoveEvent (QDragMoveEvent *e)
{
#if DEBUG_KP_DUAL_COLOR_BUTTON
    qCDebug(kpLogWidgets) << "kpDualColorButton::dragMoveEvent() canDecode="
              << KColorMimeData::canDecode (e->mimeData ());
#endif
    e->setAccepted (
        (foregroundRect ().contains (e->pos ()) ||
            backgroundRect ().contains (e->pos ())) &&
        KColorMimeData::canDecode (e->mimeData ()));
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpDualColorButton::dropEvent (QDropEvent *e)
{
    QColor col = KColorMimeData::fromMimeData (e->mimeData ());
#if DEBUG_KP_DUAL_COLOR_BUTTON
    qCDebug(kpLogWidgets) << "kpDualColorButton::dropEvent() col="
              << (int *) col.rgba()
              << " (with alpha=" << (int *) col.rgba () << ")";
#endif

    if (col.isValid ())
    {
        if (foregroundRect ().contains (e->pos ())) {
            setForegroundColor (kpColor (col.rgba()));
        }
        else if (backgroundRect ().contains (e->pos ())) {
            setBackgroundColor (kpColor (col.rgba()));
        }
    }
}

//---------------------------------------------------------------------


// protected virtual [base QWidget]
void kpDualColorButton::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KP_DUAL_COLOR_BUTTON
    qCDebug(kpLogWidgets) << "kpDualColorButton::mousePressEvent() pos=" << e->pos ();
#endif

    m_dragStartPoint = KP_INVALID_POINT;

    if (e->button () == Qt::LeftButton) {
        m_dragStartPoint = e->pos ();
    }
}

//---------------------------------------------------------------------

void kpDualColorButton::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_DUAL_COLOR_BUTTON
    qCDebug(kpLogWidgets) << "kpDualColorButton::mouseMoveEvent() pos=" << e->pos ()
              << " buttons=" << e->buttons ()
              << " dragStartPoint=" << m_dragStartPoint << endl;
#endif

    if (m_dragStartPoint == KP_INVALID_POINT) {
        return;
    }

    if (!(e->buttons () & Qt::LeftButton))
    {
        m_dragStartPoint = KP_INVALID_POINT;
        return;
    }

    const int delay = QApplication::startDragDistance ();
    if (e->x () < m_dragStartPoint.x () - delay ||
        e->x () > m_dragStartPoint.x () + delay ||
        e->y () < m_dragStartPoint.y () - delay ||
        e->y () > m_dragStartPoint.y () + delay)
    {
    #if DEBUG_KP_DUAL_COLOR_BUTTON
        qCDebug(kpLogWidgets) << "\tstarting drag as long as it's in a rectangle";
    #endif

        kpColor color;

        if (foregroundRect ().contains (m_dragStartPoint)) {
            color = foregroundColor ();
        }
        else if (backgroundRect ().contains (m_dragStartPoint)) {
            color = backgroundColor ();
        }

    #if DEBUG_KP_DUAL_COLOR_BUTTON
        qCDebug(kpLogWidgets) << "\tcolor.isValid=" << color.isValid ()
                  << " rgb=" << (color.isValid () ? (int *) color.toQRgb () : 0)
                  << endl;
    #endif

        if (color.isValid ())
        {
            if (!color.isTransparent ()) {
                KColorMimeData::createDrag (color.toQColor (), this)->exec ();
            }
        }

        m_dragStartPoint = KP_INVALID_POINT;
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpDualColorButton::mouseReleaseEvent (QMouseEvent *e)
{
    m_dragStartPoint = KP_INVALID_POINT;

    if (swapPixmapRect ().contains (e->pos ()) &&
        m_color [0] != m_color [1])
    {
    #if DEBUG_KP_DUAL_COLOR_BUTTON && 1
        qCDebug(kpLogWidgets) << "kpDualColorButton::mouseReleaseEvent() swap colors:";
    #endif
        m_oldColor [0] = m_color [0];
        m_oldColor [1] = m_color [1];

        kpColor temp = m_color [0];
        m_color [0] = m_color [1];
        m_color [1] = temp;

        update ();

        emit colorsSwapped (m_color [0], m_color [1]);
        emit foregroundColorChanged (m_color [0]);
        emit backgroundColorChanged (m_color [1]);
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpDualColorButton::mouseDoubleClickEvent (QMouseEvent *e)
{
    int whichColor = -1;

    if (foregroundRect ().contains (e->pos ())) {
        whichColor = 0;
    }
    else if (backgroundRect ().contains (e->pos ())) {
        whichColor = 1;
    }

    if (whichColor == 0 || whichColor == 1)
    {
        QColorDialog dialog(this);
        dialog.setCurrentColor(color(whichColor).toQColor());
        dialog.setOptions(QColorDialog::ShowAlphaChannel);
        if ( dialog.exec() == QDialog::Accepted ) {
          setColor(whichColor, kpColor(dialog.currentColor().rgba()));
        }
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpDualColorButton::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_DUAL_COLOR_BUTTON && 1
    qCDebug(kpLogWidgets) << "kpDualColorButton::draw() rect=" << rect ()
               << " contentsRect=" << contentsRect ()
               << endl;
#endif

    // Draw frame first.
    QFrame::paintEvent (e);

    QPainter painter (this);

    // Fill with background.
    if (isEnabled ())
    {
        kpView::drawTransparentBackground (&painter,
            contentsRect ().topLeft ()/*checkerboard top-left*/,
            contentsRect (),
            true/*preview*/);
    }
    else
    {
        // Use default widget background.
    }


    painter.translate (contentsRect ().x (), contentsRect ().y ());


    // Draw "Swap Colours" button (top-right).
    QPixmap swapPixmap = QStringLiteral(":/icons/colorbutton_swap_16x16");
    if (!isEnabled ())
    {
        // Don't let the fill() touch the mask.
        QBitmap swapBitmapMask = swapPixmap.mask ();
        swapPixmap.setMask (QBitmap ());

        // Grey out the opaque parts of "swapPixmap".
        swapPixmap.fill (palette ().color (QPalette::Dark));

        swapPixmap.setMask (swapBitmapMask);
    }
    painter.drawPixmap (swapPixmapRect ().topLeft (), swapPixmap);


    // Draw background colour patch.
    QRect bgRect = backgroundRect ();
    QRect bgRectInside = QRect (bgRect.x () + 2, bgRect.y () + 2,
                                bgRect.width () - 4, bgRect.height () - 4);
    if (isEnabled ())
    {
    #if DEBUG_KP_DUAL_COLOR_BUTTON && 1
        qCDebug(kpLogWidgets) << "\tbackgroundColor=" << (int *) m_color [1].toQRgb ()
                   << endl;
    #endif
        if (m_color [1].isTransparent ()) { // only if fully transparent
            painter.drawPixmap (bgRectInside, QStringLiteral(":/icons/color_transparent_26x26"));
        }
        else {
            painter.fillRect (bgRectInside, m_color [1].toQColor ());
        }
    }
    else {
        painter.fillRect (bgRectInside, palette().color (QPalette::Button));
    }
    qDrawShadePanel (&painter, bgRect, palette(),
                     false/*not sunken*/, 2/*lineWidth*/,
                     nullptr/*never fill*/);



    // Draw foreground colour patch.
    // Must be drawn after background patch since we're on top.
    QRect fgRect = foregroundRect ();
    QRect fgRectInside = QRect (fgRect.x () + 2, fgRect.y () + 2,
                                fgRect.width () - 4, fgRect.height () - 4);
    if (isEnabled ())
    {
    #if DEBUG_KP_DUAL_COLOR_BUTTON && 1
        qCDebug(kpLogWidgets) << "\tforegroundColor=" << (int *) m_color [0].toQRgb ()
                   << endl;
    #endif
        if (m_color [0].isTransparent ()) { // only if fully transparent
            painter.drawPixmap (fgRectInside, QStringLiteral(":/icons/color_transparent_26x26"));
        }
        else {
            painter.fillRect (fgRectInside, m_color [0].toQColor ());
        }
    }
    else {
        painter.fillRect (fgRectInside, palette ().color (QPalette::Button));
    }

    qDrawShadePanel (&painter, fgRect, palette (),
                     false/*not sunken*/, 2/*lineWidth*/,
                     nullptr/*never fill*/);
}


