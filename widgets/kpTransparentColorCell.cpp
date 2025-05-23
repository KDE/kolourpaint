
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TRANSPARENT_COLOR_CELL 0

#include "kpTransparentColorCell.h"

#include "imagelib/kpColor.h"

#include <KLocalizedString>

#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QPainter>

//---------------------------------------------------------------------

kpTransparentColorCell::kpTransparentColorCell(QWidget *parent)
    : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Fixed /*horizontal*/, QSizePolicy::Fixed /*vertical*/);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);

    m_pixmap = QStringLiteral(":/icons/color_transparent_26x26");

    this->setToolTip(i18n("Transparent"));
}

//---------------------------------------------------------------------

// public virtual [base QWidget]
QSize kpTransparentColorCell::sizeHint() const
{
    return {m_pixmap.width() + frameWidth() * 2, m_pixmap.height() + frameWidth() * 2};
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::mousePressEvent(QMouseEvent * /*e*/)
{
    // Eat press so that we own the mouseReleaseEvent().
    // [https://www.qt.io/blog/2006/05/27/mouse-event-propagation]
    //
    // However, contrary to that blog, it doesn't seem to be needed?
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::contextMenuEvent(QContextMenuEvent *e)
{
    // Eat right-mouse press to prevent it from getting to the toolbar.
    e->accept();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::mouseReleaseEvent(QMouseEvent *e)
{
    if (rect().contains(e->pos())) {
        if (e->button() == Qt::LeftButton) {
            Q_EMIT transparentColorSelected(0);
            Q_EMIT foregroundColorChanged(kpColor::Transparent);
        } else if (e->button() == Qt::RightButton) {
            Q_EMIT transparentColorSelected(1);
            Q_EMIT backgroundColorChanged(kpColor::Transparent);
        }
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpTransparentColorCell::paintEvent(QPaintEvent *e)
{
    // Draw frame first.
    QFrame::paintEvent(e);

    if (isEnabled()) {
#if DEBUG_KP_TRANSPARENT_COLOR_CELL
        qCDebug(kpLogWidgets) << "kpTransparentColorCell::paintEvent() contentsRect=" << contentsRect() << endl;
#endif
        QPainter p(this);
        p.drawPixmap(contentsRect(), m_pixmap);
    }
}

//---------------------------------------------------------------------

#include "moc_kpTransparentColorCell.cpp"
