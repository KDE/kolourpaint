
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


#define DEBUG_KP_COLOR_CELLS 1


#include <kpColorCells.h>

#include <QContextMenuEvent>
#include <QMouseEvent>

#include <kpColorCollection.h>

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
static QColor kpRed (255, 0, 0);
static QColor kpGreen (0, 255, 0);
static QColor kpBlue (0, 0, 255);
static QColor kpBlack (0, 0, 0);
static QColor kpWhite (255, 255, 255);

// intentionally _not_ an HSV darkener
static QColor dark (const QColor &color)
{
    return blend (color, kpBlack);
}

// full-brightness colors
static QColor kpYellow = add (kpRed, kpGreen);
static QColor kpPurple = add (kpRed, kpBlue);
static QColor kpAqua = add (kpGreen, kpBlue);

// mixed colors
static QColor kpGrey = blend (kpBlack, kpWhite);
static QColor kpLightGrey = blend (kpGrey, kpWhite);
static QColor kpOrange = blend (kpRed, kpYellow);

// pastel colors
static QColor kpPink = blend (kpRed, kpWhite);
static QColor kpLightGreen = blend (kpGreen, kpWhite);
static QColor kpLightBlue = blend (kpBlue, kpWhite);
static QColor kpTan = blend (kpYellow, kpWhite);



class kpDefaultColorCollection : public kpColorCollection
{
public:
    kpDefaultColorCollection ();
    ~kpDefaultColorCollection ();
};

kpDefaultColorCollection::kpDefaultColorCollection ()
{
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

    for (int i = 0; i < (int) (sizeof (colors) / sizeof (colors [0])); i++)
    {
        addColor (colors [i]);
    }
}

kpDefaultColorCollection::~kpDefaultColorCollection ()
{
}





/* TODO: clean up this code!!!
 *       (probably when adding palette load/save)
 */

struct kpColorCellsPrivate
{
    Qt::Orientation orientation;
    int mouseButton;

    kpColorCollection colorCol;
    KUrl url;
    bool isModified;
};

static int PaletteColumns (const kpColorCollection &colorCol)
{
    // 11 or smaller.
    return qMin (11, colorCol.count ());
}

static int PaletteRows (const kpColorCollection &colorCol)
{
    const int cols = ::PaletteColumns (colorCol);
    if (cols == 0)
        return 0;

    return (colorCol.count () + (cols - 1)) / cols;
}


static int PaletteCellSize (const kpColorCollection &colorCol)
{
    if (::PaletteRows (colorCol) <= 2)
        return 26;
    else
        return 13;
}


// HITODO: You should not be able to get text focus for this!
kpColorCells::kpColorCells (QWidget *parent,
                            Qt::Orientation o)
    : KColorCells (parent, 1/*rows for now*/, 300/*HACK due to KColorCells bug: cols for now*/),
      d (new kpColorCellsPrivate ())
{
    d->mouseButton = -1;

    setShading (false);  // no 3D look

    // Trap KColorDrag so that kpMainWindow does not trap it.
    // See our impl of dropEvent().
    setAcceptDrops (true);
    setAcceptDrags (true);

    connect (this, SIGNAL (colorDoubleClicked (int, QColor)),
             SLOT (slotColorDoubleClicked (int, QColor)));

    setOrientation (o);


    setColorCollection (kpDefaultColorCollection ());
}

kpColorCells::~kpColorCells ()
{
    delete d;
}

// public static
kpColorCollection kpColorCells::DefaultColorCollection ()
{
    return kpDefaultColorCollection ();
}

Qt::Orientation kpColorCells::orientation () const
{
    return d->orientation;
}

void kpColorCells::setOrientation (Qt::Orientation o)
{
    d->orientation = o;

    makeCellsMatchColorCollection ();
}


// protected
// OPT: Find out why this is being called multiple times on startup.
void kpColorCells::makeCellsMatchColorCollection ()
{
    int c, r;

    if (orientation () == Qt::Horizontal)
    {
        c = ::PaletteColumns (d->colorCol);
        r = ::PaletteRows (d->colorCol);
    }
    else
    {
        c = ::PaletteRows (d->colorCol);
        r = ::PaletteColumns (d->colorCol);;
    }

#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::makeCellsMatchColorCollection():"
              << " r=" << r << " c=" << c << endl;
#endif

    setRowCount (r);
    setColumnCount (c);


    // COMPAT: Get cell dimensions exactly as claimed by these variables.
    //         I suspect frameWidth() isn't right.
    // TODO: It wasn't right in KDE 3.4 either (haven't checked 3.5).
    int CellWidth = ::PaletteCellSize (d->colorCol),
        CellHeight = ::PaletteCellSize (d->colorCol);

    for (int y = 0; y < r; y++)
        setRowHeight (y, CellHeight);
    for (int x = 0; x < c; x++)
        setColumnWidth (x, CellWidth);

    setFixedSize (columnCount () * CellWidth + frameWidth () * 2,
                  rowCount () * CellHeight + frameWidth () * 2);

/*
    kDebug () << "\tlimits: array=" << sizeof (colors) / sizeof (colors [0])
               << " r*c=" << r * c << endl;
    kDebug () << "\tsizeof (colors)=" << sizeof (colors)
               << " sizeof (colors [0])=" << sizeof (colors [0])
               << endl;*/
    for (int i = 0;
         /*i < int (sizeof (colors) / sizeof (colors [0])) &&*/
         i < r * c;
         i++)
    {
        int y, x;
        int pos;

        if (orientation () == Qt::Horizontal)
        {
            y = i / c;
            x = i % c;
            pos = i;
        }
        else
        {
            y = i % r;
            x = i / r;
            // int x = c - 1 - i / r;
            pos = y * c + x;
        }
    #if DEBUG_KP_COLOR_CELLS
        kDebug () << "\tSetting cell " << i << ": y=" << y << " x=" << x
                  << " pos=" << pos << endl;
        kDebug () << "\t\tcolor=" << (int *) d->colorCol.color (i).rgb ();
    #endif
        KColorCells::setColor (pos, d->colorCol.color (i));
        //this->setToolTip( cellGeometry (y, x), colors [i].name ());
    }
}


bool kpColorCells::isModified () const
{
    return d->isModified;
}

// TODO: Use
void kpColorCells::setModified (bool yes)
{
    if (yes != d->isModified)
        return;

    d->isModified = yes;
}


KUrl kpColorCells::url () const
{
    return d->url;
}


void kpColorCells::setColorCollection (const kpColorCollection &colorCol, const KUrl &url)
{
    d->colorCol = colorCol;
    d->url = url;

    d->isModified = false;

    makeCellsMatchColorCollection ();
}


bool kpColorCells::openColorCollection (const KUrl &url)
{
    if (d->colorCol.open (url))
    {
        d->url = url;
        d->isModified = false;

        makeCellsMatchColorCollection ();

        return true;
    }

    return false;  // TODO: error message
}

bool kpColorCells::saveColorCollectionAs (const KUrl &url)
{
    if (d->colorCol.saveAs (url))
    {
        d->url = url;
        d->isModified = false;
        return true;
    }

    return false;  // TODO: error message
}


void kpColorCells::appendRow ()
{
    // TODO: Wrong for 0 columns.
    const int cols = ::PaletteColumns (d->colorCol);
    const int cellArea = ::PaletteRows (d->colorCol) * cols;

    const int totalColors = (cellArea - d->colorCol.count ()) + cols;
    for (int i = 0; i < totalColors; i++)
        d->colorCol.addColor (Qt::white);

    makeCellsMatchColorCollection ();
}

void kpColorCells::deleteLastRow ()
{
    return;  // TODO
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
    //e->accept ();

    // TODO: Remove method.



    KColorCells::dropEvent (e);
}

// virtual protected
void kpColorCells::paintCell (QPainter *painter, int row, int col)
{
#ifdef __GNUC__
#warning "Port to KColorCells API changes"
// All the below code does is gray out all the colors and give each cell
// a 3D look, when the widget is disabled.
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
    d->mouseButton = -1;

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
            d->mouseButton = 0;
        else if (button & Qt::RightButton)
            d->mouseButton = 1;
    }

    // (d->mouseButton will be read in the slot)
    connect (this, SIGNAL (colorSelected (int, QColor)), this, SLOT (slotColorSelected (int)));
    KColorCells::mouseReleaseEvent (e);
    disconnect (this, SIGNAL (colorSelected (int, QColor)), this, SLOT (slotColorSelected (int)));

#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::mouseReleaseEvent() setting d->mouseButton back to -1";
#endif
    d->mouseButton = -1;

    // Deselect the selected cell (selected by above KColorCells::mouseReleaseEvent()).
    // KolourPaint's palette has no concept of a current cell/color: you can
    // pick a color but you can't mark a cell as selected.  In any case, a
    // selected cell would be rendered as violet, which would ruin the cell.
    //
    // setSelectionMode (KColorCells::NoSelection); does not work so we
    // clearSelection().  I think setSelectionMode() concerns when the user
    // directly selects a cell - not when KColorCells::mouseReleaseEvent()
    // selects a cell programmatically.
    clearSelection ();
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
    QColor c = KColorCells::color (cell);
#if DEBUG_KP_COLOR_CELLS
    kDebug () << "kpColorCells::slotColorSelected(cell=" << cell
               << ") mouseButton = " << d->mouseButton
               << " rgb=" << (int *) c.rgb ()
               << endl;
#endif

    if (d->mouseButton == 0)
    {
        emit foregroundColorChanged (kpColor (c.rgb ()));
    }
    else if (d->mouseButton == 1)
    {
        emit backgroundColorChanged (kpColor (c.rgb ()));
    }

    d->mouseButton = -1;  // just in case
}

// protected slot
void kpColorCells::slotColorDoubleClicked (int cell, const QColor &)
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
