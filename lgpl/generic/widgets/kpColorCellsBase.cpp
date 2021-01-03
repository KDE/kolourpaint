
/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 2007 Roberto Raggi (roberto@kdevelop.org)
    Copyright (C) 2007 Clarence Dang (dang@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------

#define DEBUG_KP_COLOR_CELLS_BASE 0

#include "kpColorCellsBase.h"

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QItemDelegate>
#include <QMouseEvent>
#include <QPainter>
#include <QImage>
#include "kpLogCategories.h"

#include <KColorMimeData>


class kpColorCellsBase::kpColorCellsBasePrivate
{
public:
    kpColorCellsBasePrivate(kpColorCellsBase *q): q(q)
    {
        colors = nullptr;
        inMouse = false;
        selected = -1;
        shade = false;
        acceptDrags = false;
        cellsResizable = true;
    }

    kpColorCellsBase *q;

    // Note: This is a good thing and is _not_ data duplication with the
    //       colors of QTableWidget cells, for the following reasons:
    //
    //       1. QColor in Qt4 is full-quality RGB.  However, QTableWidget
    //          cells are lossy as their colors may be dithered on the screen.
    //
    //          Eventually, this field will be changed to a kpColor.
    //
    //       2. We change the QTableWidget cells' colors when the widget is
    //          disabled (see changeEvent()).
    //
    //       Therefore, do not remove this field without better reasons.
    QColor *colors;

    QPoint mousePos;
    int	selected;
    bool shade;
    bool acceptDrags;
    bool cellsResizable;
    bool inMouse;
};

kpColorCellsBase::kpColorCellsBase( QWidget *parent, int rows, int cols )
    : QTableWidget( parent ), d(new kpColorCellsBasePrivate(this))
{
    setItemDelegate(new QItemDelegate(this));

    setFrameShape(QFrame::NoFrame);
    d->shade = true;
    setRowCount( rows );
    setColumnCount( cols );

    verticalHeader()->setMinimumSectionSize(16);
    verticalHeader()->hide();
    horizontalHeader()->setMinimumSectionSize(16);
    horizontalHeader()->hide();

    d->colors = new QColor [ rows * cols ];

    d->selected = 0;
    d->inMouse = false;

    // Drag'n'Drop
    setAcceptDrops( true);

    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    viewport()->setBackgroundRole( QPalette::Window );
    setBackgroundRole( QPalette::Window );
}

kpColorCellsBase::~kpColorCellsBase()
{
    delete [] d->colors;

    delete d;
}

void kpColorCellsBase::invalidateAllColors ()
{
    for (int r = 0; r < rowCount (); r++)
        for (int c = 0; c < columnCount (); c++)
            d->colors [r * columnCount () + c] = QColor ();
}

void kpColorCellsBase::clear()
{
    invalidateAllColors ();
    QTableWidget::clear ();
}

void kpColorCellsBase::clearContents()
{
    invalidateAllColors ();
    QTableWidget::clearContents ();
}

void kpColorCellsBase::setRowColumnCounts (int rows, int columns)
{
    const int oldRows = rowCount (), oldCols = columnCount ();
    const int newRows = rows, newCols = columns;
#if DEBUG_KP_COLOR_CELLS_BASE
    qCDebug(kpLogColorCollection) << "oldRows=" << oldRows << "oldCols=" << oldCols
        << "newRows=" << newRows << "newCols=" << newCols;
#endif

    if (oldRows == newRows && oldCols == newCols)
        return;

    QTableWidget::setColumnCount (newCols);
    QTableWidget::setRowCount (newRows);

    QColor *oldColors = d->colors;
    d->colors = new QColor [newRows * newCols];

    for (int r = 0; r < qMin (oldRows, newRows); r++)
        for (int c = 0; c < qMin (oldCols, newCols); c++)
            d->colors [r * newCols + c] = oldColors [r * oldCols + c];

    delete [] oldColors;
}

void kpColorCellsBase::setColumnCount (int newColumns)
{
    setRowColumnCounts (rowCount (), newColumns);
}

void kpColorCellsBase::setRowCount (int newRows)
{
    setRowColumnCounts (newRows, columnCount ());
}

QColor kpColorCellsBase::color(int index) const
{
    return d->colors[index];
}

int kpColorCellsBase::count() const
{
    return rowCount() * columnCount();
}

void kpColorCellsBase::setShading(bool _shade)
{
    d->shade = _shade;
}

void kpColorCellsBase::setAcceptDrags(bool _acceptDrags)
{
    d->acceptDrags = _acceptDrags;
}

void kpColorCellsBase::setCellsResizable(bool yes)
{
    d->cellsResizable = yes;
}

void kpColorCellsBase::setSelected(int index)
{
    Q_ASSERT( index >= 0 && index < count() );

    d->selected = index;
}

int kpColorCellsBase::selectedIndex() const
{
    return d->selected;
}

//---------------------------------------------------------------------

static void TableWidgetItemSetColor (QTableWidgetItem *tableItem, const QColor &color)
{
    Q_ASSERT (tableItem);
    QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    const int StippleSize = 4;
    QColor useColor;

    for (int dy = 0; dy < 16; dy += StippleSize)
    {
        for (int dx = 0; dx < 16; dx += StippleSize)
        {
            const bool parity = ((dy + dx) / StippleSize) % 2;

            if (!parity)
                useColor = Qt::white;
            else
                useColor = Qt::lightGray;

            painter.fillRect(dx, dy, StippleSize, StippleSize, useColor);
        }
    }

    painter.fillRect(image.rect(), color);
    painter.end();

    tableItem->setData(Qt::BackgroundRole , QBrush(image));
}

//---------------------------------------------------------------------

void kpColorCellsBase::setColor( int column, const QColor &colorIn )
{
    const int tableRow = column / columnCount();
    const int tableColumn = column % columnCount();

    Q_ASSERT( tableRow >= 0 && tableRow < rowCount() );
    Q_ASSERT( tableColumn >= 0 && tableColumn < columnCount() );

    QColor color = colorIn;

    d->colors[column] = color;

    QTableWidgetItem* tableItem = item(tableRow,tableColumn);

    if (color.isValid ())
    {
        if ( tableItem == nullptr ) {
            tableItem = new QTableWidgetItem();
            setItem(tableRow,tableColumn,tableItem);
        }

        if (isEnabled ())
            ::TableWidgetItemSetColor (tableItem, color);
    }
    else
    {
        delete tableItem;
    }

    emit colorChanged (column, color);
}

void kpColorCellsBase::changeEvent( QEvent* event )
{
    QTableWidget::changeEvent (event);

    if (event->type () != QEvent::EnabledChange)
        return;

    for (int r = 0; r < rowCount (); r++)
    {
        for (int c = 0; c < columnCount (); c++)
        {
            const int index = r * columnCount () + c;

            QTableWidgetItem* tableItem = item(r, c);

            // See API Doc for this invariant.
            Q_ASSERT (!!tableItem == d->colors [index].isValid ());

            if (!tableItem)
                continue;


            QColor color;
            if (isEnabled ())
                color = d->colors [index];
            else
                color = palette ().color (backgroundRole ());

            ::TableWidgetItemSetColor (tableItem, color);
        }
    }
}

/*void kpColorCellsBase::paintCell( QPainter *painter, int row, int col )
{
    painter->setRenderHint( QPainter::Antialiasing , true );

	QBrush brush;
	int w = 1;

	if (shade)
	{
		qDrawShadePanel( painter, 1, 1, cellWidth()-2,
						 cellHeight()-2, palette(), true, 1, &brush );
		w = 2;
	}
	QColor color = colors[ row * numCols() + col ];
	if (!color.isValid())
	{
		if (!shade) return;
		color = palette().color(backgroundRole());
	}

	const QRect colorRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );
	painter->fillRect( colorRect, color );

	if ( row * numCols() + col == selected ) {
		painter->setPen( qGray(color.rgb())>=127 ? Qt::black : Qt::white );
		painter->drawLine( colorRect.topLeft(), colorRect.bottomRight() );
		painter->drawLine( colorRect.topRight(), colorRect.bottomLeft() );
	}
}*/

void kpColorCellsBase::resizeEvent( QResizeEvent* e )
{
    if (d->cellsResizable)
    {
        // According to the Qt doc:
        //   If you need to set the width of a given column to a fixed value, call
        //   QHeaderView::resizeSection() on the table's {horizontal,vertical}
        //   header.
        // Therefore we iterate over each row and column and set the header section
        // size, as the sizeHint does indeed appear to be ignored in favor of a
        // minimum size that is larger than what we want.
        for ( int index = 0 ; index < columnCount() ; index++ )
            horizontalHeader()->resizeSection( index, sizeHintForColumn(index) );
        for ( int index = 0 ; index < rowCount() ; index++ )
            verticalHeader()->resizeSection( index, sizeHintForRow(index) );
    }
    else
    {
        // Update scrollbars if they're forced on by a subclass.
        // TODO: Should the d->cellsResizable path (from kdelibs) do this as well?
        QTableWidget::resizeEvent (e);
    }
}

int kpColorCellsBase::sizeHintForColumn(int /*column*/) const
{
    // TODO: Should it be "(width() - frameWidth() * 2) / columnCount()"?
    return width() / columnCount() ;
}

int kpColorCellsBase::sizeHintForRow(int /*row*/) const
{
    // TODO: Should be "(height() - frameWidth() * 2) / rowCount()"?
    return height() / rowCount() ;
}

void kpColorCellsBase::mousePressEvent( QMouseEvent *e )
{
    d->inMouse = true;
    d->mousePos = e->pos();
}


int kpColorCellsBase::positionToCell(const QPoint &pos, bool ignoreBorders,
        bool allowEmptyCell) const
{
    //TODO ignoreBorders not yet handled
    Q_UNUSED( ignoreBorders )

    const int r = indexAt (pos).row (), c = indexAt (pos).column ();
#if DEBUG_KP_COLOR_CELLS_BASE
    qCDebug(kpLogColorCollection) << "r=" << r << "c=" << c;
#endif

    if (r == -1 || c == -1)
       return -1;

    if (!allowEmptyCell && !itemAt(pos))
        return -1;

    const int cell = r * columnCount() + c;

   /*if (!ignoreBorders)
   {
      int border = 2;
      int x = pos.x() - col * cellWidth();
      int y = pos.y() - row * cellHeight();
      if ( (x < border) || (x > cellWidth()-border) ||
           (y < border) || (y > cellHeight()-border))
         return -1;
   }*/

    return cell;
}


void kpColorCellsBase::mouseMoveEvent( QMouseEvent *e )
{
    if( !(e->buttons() & Qt::LeftButton)) return;

    if(d->inMouse) {
        int delay = QApplication::startDragDistance();
        if(e->x() > d->mousePos.x()+delay || e->x() < d->mousePos.x()-delay ||
           e->y() > d->mousePos.y()+delay || e->y() < d->mousePos.y()-delay){
            // Drag color object
            int cell = positionToCell(d->mousePos);
            if (cell != -1)
            {
            #if DEBUG_KP_COLOR_CELLS_BASE
               qCDebug(kpLogColorCollection) << "beginning drag from cell=" << cell
                         << "color: isValid=" << d->colors [cell].isValid ()
                         << " rgba=" << (int *) d->colors [cell].rgba();
            #endif
               Q_ASSERT (d->colors[cell].isValid());
               KColorMimeData::createDrag(d->colors[cell], this)->exec(Qt::CopyAction | Qt::MoveAction);
            #if DEBUG_KP_COLOR_CELLS_BASE
               qCDebug(kpLogColorCollection) << "finished drag";
            #endif
            }
        }
    }
}


// LOTODO: I'm not quite clear on how the drop actions logic is supposed
//         to be done e.g.:
//
//         1. Who is supposed to call setDropAction().
//         2. Which variant of accept(), setAccepted(), acceptProposedAction() etc.
//            is supposed to be called to accept a move -- rather than copy --
//            action.
//
//         Nevertheless, it appears to work -- probably because we restrict
//         the non-Qt-default move/swap action to be intrawidget.
static void SetDropAction (QWidget *self, QDropEvent *event)
{
     // TODO: Would be nice to default to CopyAction if the destination cell
     //       is null.
     if (event->source () == self && (event->keyboardModifiers () & Qt::ControlModifier) == 0)
         event->setDropAction(Qt::MoveAction);
     else
         event->setDropAction(Qt::CopyAction);
}

void kpColorCellsBase::dragEnterEvent( QDragEnterEvent *event)
{
#if DEBUG_KP_COLOR_CELLS_BASE
     qCDebug(kpLogColorCollection) << "kpColorCellsBase::dragEnterEvent() acceptDrags="
               << d->acceptDrags
               << " canDecode=" << KColorMimeData::canDecode(event->mimeData());
#endif
     event->setAccepted( d->acceptDrags && KColorMimeData::canDecode( event->mimeData()));
     if (event->isAccepted ())
         ::SetDropAction (this, event);
}

// Reimplemented to override QTableWidget's override.  Else dropping doesn't work.
void kpColorCellsBase::dragMoveEvent (QDragMoveEvent *event)
{
#if DEBUG_KP_COLOR_CELLS_BASE
     qCDebug(kpLogColorCollection) << "kpColorCellsBase::dragMoveEvent() acceptDrags="
               << d->acceptDrags
               << " canDecode=" << KColorMimeData::canDecode(event->mimeData());
#endif
     // TODO: Disallow drag that isn't onto a cell.
     event->setAccepted( d->acceptDrags && KColorMimeData::canDecode( event->mimeData()));
     if (event->isAccepted ())
         ::SetDropAction (this, event);
}

void kpColorCellsBase::dropEvent( QDropEvent *event)
{
     QColor c=KColorMimeData::fromMimeData(event->mimeData());

     const int dragSourceCell = event->source () == this ?
         positionToCell (d->mousePos, true) :
         -1;
#if DEBUG_KP_COLOR_CELLS_BASE
     qCDebug(kpLogColorCollection) << "kpColorCellsBase::dropEvent()"
               << "color: rgba=" << (const int *) c.rgba () << "isValid=" << c.isValid()
               << "source=" << event->source () << "dragSourceCell=" << dragSourceCell;
#endif
     if( c.isValid()) {
          ::SetDropAction (this, event);

          int cell = positionToCell(event->pos(), true, true/*allow empty cell*/);
     #if DEBUG_KP_COLOR_CELLS_BASE
          qCDebug(kpLogColorCollection) << "\tcell=" << cell;
     #endif
          // TODO: I believe kdelibs forgets to do this.
          if (cell == -1)
              return;

          // Avoid NOP.
          if (cell == dragSourceCell)
              return;

          QColor destOldColor = d->colors [cell];
	  setColor(cell,c);

    #if DEBUG_KP_COLOR_CELLS_BASE
          qCDebug(kpLogColorCollection) << "\tdropAction=" << event->dropAction ()
                    << "destOldColor.rgba=" << (const int *) destOldColor.rgba ();
    #endif
          if (event->dropAction () == Qt::MoveAction && dragSourceCell != -1) {
              setColor(dragSourceCell, destOldColor);
          }
     }
}

void kpColorCellsBase::mouseReleaseEvent( QMouseEvent *e )
{
	int cell = positionToCell(d->mousePos);
    int currentCell = positionToCell(e->pos());

        // If we release the mouse in another cell and we don't have
        // a drag we should ignore this event.
        if (currentCell != cell)
           cell = -1;

	if ( (cell != -1) && (d->selected != cell) )
	{
		d->selected = cell;

        const int newRow = cell/columnCount();
        const int newColumn = cell%columnCount();

        clearSelection(); // we do not want old violet selected cells

        item(newRow,newColumn)->setSelected(true);
    }

    d->inMouse = false;
    if (cell != -1)
    {
        emit colorSelected( cell , color(cell) );
        emit colorSelectedWhitButton( cell , color(cell), e->button() );
    }
}

void kpColorCellsBase::mouseDoubleClickEvent( QMouseEvent * /*e*/ )
{
  int cell = positionToCell(d->mousePos, false, true/*allow empty cell*/);

  if (cell != -1)
    emit colorDoubleClicked( cell , color(cell) );
}


