
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


#define DEBUG_KP_COLOR_CELLS 0


#include <kpColorCells.h>

#include <QContextMenuEvent>
#include <QMouseEvent>

#include <kpColor.h>


static inline int roundUp2 (int val)
{
    return val % 2 ? val + 1 : val;
}

static inline int btwn0_255 (int val)
{
    if (val < 0)
        return 0;
    else if (val > 255)
        return 255;
    else
        return val;
}

enum
{
    blendDark = 25,
    blendNormal = 50,
    blendLight = 75,
    blendAdd = 100
};

static QColor blend (const QColor &a, const QColor &b, int percent = blendNormal)
{
    return QColor (btwn0_255 (roundUp2 (a.red () + b.red ()) * percent / 100),
                   btwn0_255 (roundUp2 (a.green () + b.green ()) * percent / 100),
                   btwn0_255 (roundUp2 (a.blue () + b.blue ()) * percent / 100));
}

static QColor add (const QColor &a, const QColor &b)
{
    return blend (a, b, blendAdd);
}





//
// make our own colors in case weird ones like "Qt::cyan"
// (turquoise) get changed by Qt
//

// primary colors + B&W
static QColor kpRed;
static QColor kpGreen;
static QColor kpBlue;
static QColor kpBlack;
static QColor kpWhite;

// intentionally _not_ an HSV darkener
static QColor dark (const QColor &color)
{
    return blend (color, kpBlack);
}

// full-brightness colors
static QColor kpYellow;
static QColor kpPurple;
static QColor kpAqua;

// mixed colors
static QColor kpGrey;
static QColor kpLightGrey;
static QColor kpOrange;

// pastel colors
static QColor kpPink;
static QColor kpLightGreen;
static QColor kpLightBlue;
static QColor kpTan;

static bool ownColorsInitialised = false;

/* TODO: clean up this code!!!
 *       (probably when adding palette load/save)
 */
#define rows 2
#define cols 11
kpColorCells::kpColorCells (QWidget *parent,
                            Qt::Orientation o)
    : KColorCells (parent, rows, cols),
      m_mouseButton (-1)
{
    setShading (false);  // no 3D look

    // Trap KColorDrag so that kpMainWindow does not trap it.
    // See our impl of dropEvent().
    setAcceptDrops (true);
    setAcceptDrags (true);

    connect (this, SIGNAL (colorDoubleClicked (int, QColor)),
             SLOT (slotColorDoubleClicked (int, QColor)));

    if (!ownColorsInitialised)
    {
        // Don't initialise globally when we probably don't have a colour
        // allocation context.  This way, the colours aren't sometimes
        // invalid (e.g. at 8-bit).

        kpRed = QColor (255, 0, 0);
        kpGreen = QColor (0, 255, 0);
        kpBlue = QColor (0, 0, 255);
        kpBlack = QColor (0, 0, 0);
        kpWhite = QColor (255, 255, 255);

        kpYellow = add (kpRed, kpGreen);
        kpPurple = add (kpRed, kpBlue);
        kpAqua = add (kpGreen, kpBlue);

        kpGrey = blend (kpBlack, kpWhite);
        kpLightGrey = blend (kpGrey, kpWhite);
        kpOrange = blend (kpRed, kpYellow);

        kpPink = blend (kpRed, kpWhite);
        kpLightGreen = blend (kpGreen, kpWhite);
        kpLightBlue = blend (kpBlue, kpWhite);
        kpTan = blend (kpYellow, kpWhite);

        ownColorsInitialised = true;
    }

    setOrientation (o);
}

kpColorCells::~kpColorCells ()
{
}

Qt::Orientation kpColorCells::orientation () const
{
    return m_orientation;
}

void kpColorCells::setOrientation (Qt::Orientation o)
{
    int c, r;

    if (o == Qt::Horizontal)
    {
        c = cols;
        r = rows;
    }
    else
    {
        c = rows;
        r = cols;
    }

#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::setOrientation(): r=" << r << " c=" << c << endl;
#endif

    setRowCount(r);
    setColumnCount(c);

#ifdef __GNUC__
#warning "Port to KColorCells API changes"
#endif
#if 0
    setCellWidth (26);
    setCellHeight (26);
#endif

    setFixedSize (columnCount () * columnWidth (0) + frameWidth () * 2,
                  rowCount  () * rowHeight (0) + frameWidth () * 2);

/*
    kDebug () << "\tlimits: array=" << sizeof (colors) / sizeof (colors [0])
               << " r*c=" << r * c << endl;
    kDebug () << "\tsizeof (colors)=" << sizeof (colors)
               << " sizeof (colors [0])=" << sizeof (colors [0])
               << endl;*/
    QColor colors [] =
    {
        kpBlack,
        kpGrey,
        kpRed,
        kpOrange,
        kpYellow,
        kpGreen,
        kpAqua,
        kpBlue,
        kpPurple,
        kpPink,
        kpLightGreen,

        kpWhite,
        kpLightGrey,
        dark (kpRed),
        dark (kpOrange)/*brown*/,
        dark (kpYellow),
        dark (kpGreen),
        dark (kpAqua),
        dark (kpBlue),
        dark (kpPurple),
        kpLightBlue,
        kpTan
    };

    for (int i = 0;
         /*i < int (sizeof (colors) / sizeof (colors [0])) &&*/
         i < r * c;
         i++)
    {
        int y, x;
        int pos;

        if (o == Qt::Horizontal)
        {
            y = i / cols;
            x = i % cols;
            pos = i;
        }
        else
        {
            y = i % cols;
            x = i / cols;
            // int x = rows - 1 - i / cols;
            pos = y * rows + x;
        }

        KColorCells::setColor (pos, colors [i]);
        //this->setToolTip( cellGeometry (y, x), colors [i].name ());
    }

    m_orientation = o;
}

// virtual protected [base KColorCells]
void kpColorCells::dropEvent (QDropEvent *e)
{
    // Eat event so that:
    //
    // 1. User doesn't clobber the palette (until we support reconfigurable
    //    palettes)
    // 2. kpMainWindow::dropEvent() doesn't try to paste colour code as text
    //    (when the user slips and drags colour cell a little instead of clicking)
    e->accept ();
}

// virtual protected
void kpColorCells::paintCell (QPainter *painter, int row, int col)
{
#ifdef __GNUC__
#warning "Port to KColorCells API changes"
#endif
#if 0
    QColor oldColor;
    int cellNo = -1;

    if (!isEnabled ())
    {
        cellNo = row * columnCount () + col;

        // make all cells 3D (so that disabled palette doesn't look flat)
        setShading (true);

        oldColor = KColorCells::color (cellNo);
        KColorCells::colors [cellNo] = palette ().color (backgroundRole ());
    }


    // no focus rect as it doesn't make sense
    // since 2 colors (foreground & background) can be selected
    KColorCells::selected = -1;
    KColorCells::paintCell (painter, row, col);


    if (!isEnabled ())
    {
        KColorCells::colors [cellNo] = oldColor;
        setShading (false);
    }
#endif
}

// protected virtual [base QWidget]
void kpColorCells::contextMenuEvent (QContextMenuEvent *e)
{
    // Eat right-mouse press to prevent it from getting to the toolbar.
    e->accept ();
}

// virtual protected
void kpColorCells::mouseReleaseEvent (QMouseEvent *e)
{
    m_mouseButton = -1;

    Qt::ButtonState button = e->button ();
#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::mouseReleaseEvent(left="
               << (button & Qt::LeftButton)
               << ",right="
               << (button & Qt::RightButton)
               << ")"
               << endl;
#endif
    if (!((button & Qt::LeftButton) && (button & Qt::RightButton)))
    {
        if (button & Qt::LeftButton)
            m_mouseButton = 0;
        else if (button & Qt::RightButton)
            m_mouseButton = 1;
    }

    connect (this, SIGNAL (colorSelected (int, QColor)), this, SLOT (slotColorSelected (int)));
    KColorCells::mouseReleaseEvent (e);
    disconnect (this, SIGNAL (colorSelected (int, QColor)), this, SLOT (slotColorSelected (int)));

#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::mouseReleaseEvent() setting m_mouseButton back to -1" << endl;
#endif
    m_mouseButton = -1;
}

// protected virtual [base KColorCells]
void kpColorCells::resizeEvent (QResizeEvent *e)
{
    // KColorCells::resizeEvent() tries to adjust the cellWidth and cellHeight
    // to the current dimensions but doesn't take into account
    // frame{Width,Height}().
    //
    // In any case, we already set the cell{Width,Height} and a fixed
    // widget size and don't want any of it changed.  Eat the resize event.
    (void) e;
}

// protected slot
void kpColorCells::slotColorSelected (int cell)
{
#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::slotColorSelected(cell=" << cell
               << ") mouseButton = " << m_mouseButton << endl;
#endif
    QColor c = KColorCells::color (cell);

    if (m_mouseButton == 0)
    {
        emit foregroundColorChanged (c);
        emit foregroundColorChanged (kpColor (c.rgb ()));
    }
    else if (m_mouseButton == 1)
    {
        emit backgroundColorChanged (c);
        emit backgroundColorChanged (kpColor (c.rgb ()));
    }

    m_mouseButton = -1;  // just in case
}

// protected slot
void kpColorCells::slotColorDoubleClicked (int cell,const QColor &)
{
#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::slotColorDoubleClicked(cell="
               << cell << ")" << endl;
#endif

    QColor color = KColorCells::color (cell);

    // TODO: parent
    if (KColorDialog::getColor (color/*ref*/))
        KColorCells::setColor (cell, color);
}


#include <kpColorCells.moc>
