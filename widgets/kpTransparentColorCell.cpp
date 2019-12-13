
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


#define DEBUG_KP_TRANSPARENT_COLOR_CELL 0


#include "kpTransparentColorCell.h"

#include "imagelib/kpColor.h"

#include <KLocalizedString>

#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QPainter>

//---------------------------------------------------------------------

kpTransparentColorCell::kpTransparentColorCell (QWidget *parent)
    : QFrame (parent)
{
    setSizePolicy (QSizePolicy::Fixed/*horizontal*/,
                   QSizePolicy::Fixed/*vertical*/);
    setFrameStyle (QFrame::Panel | QFrame::Sunken);

    m_pixmap = QStringLiteral(":/icons/color_transparent_26x26");

    this->setToolTip( i18n ("Transparent"));
}

//---------------------------------------------------------------------

// public virtual [base QWidget]
QSize kpTransparentColorCell::sizeHint () const
{
    return  {m_pixmap.width () + frameWidth () * 2,
                m_pixmap.height () + frameWidth () * 2};
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::mousePressEvent (QMouseEvent * /*e*/)
{
    // Eat press so that we own the mouseReleaseEvent().
    // [https://www.qt.io/blog/2006/05/27/mouse-event-propagation]
    //
    // However, contrary to that blog, it doesn't seem to be needed?
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::contextMenuEvent (QContextMenuEvent *e)
{
    // Eat right-mouse press to prevent it from getting to the toolbar.
    e->accept ();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::mouseReleaseEvent (QMouseEvent *e)
{
    if (rect ().contains (e->pos ()))
    {
        if (e->button () == Qt::LeftButton)
        {
            emit transparentColorSelected (0);
            emit foregroundColorChanged (kpColor::Transparent);
        }
        else if (e->button () == Qt::RightButton)
        {
            emit transparentColorSelected (1);
            emit backgroundColorChanged (kpColor::Transparent);
        }
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::paintEvent (QPaintEvent *e)
{
    // Draw frame first.
    QFrame::paintEvent (e);

    if (isEnabled ())
    {
    #if DEBUG_KP_TRANSPARENT_COLOR_CELL
        qCDebug(kpLogWidgets) << "kpTransparentColorCell::paintEvent() contentsRect="
                   << contentsRect ()
                   << endl;
    #endif
        QPainter p (this);
        p.drawPixmap (contentsRect (), m_pixmap);
    }
}

//---------------------------------------------------------------------


