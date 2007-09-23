
// SYNC: Periodically merge in changes from:
//
//           trunk/KDE/kdelibs/kdeui/colors/kcolordialog.{h,cpp}
//
//       which this is a fork of.
//
//       Our changes can be merged back into KDE (grep for "Added for KolourPaint" and similar).

/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 2007 Roberto Raggi (roberto@kdevelop.org)

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

#include <kpColorCellsBase.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QMouseEvent>

#include <KColorMimeData>
#include <KDebug>
#include <KGlobalSettings>


class kpColorCellsBase::kpColorCellsBasePrivate
{
public:
    kpColorCellsBasePrivate(kpColorCellsBase *q): q(q)
    {
        colors = 0;
        inMouse = false;
        selected = -1;
        shade = false;
        acceptDrags = false;
    }

    kpColorCellsBase *q;
    QColor *colors;
    QPoint mousePos;
    int	selected;
    bool shade;
    bool acceptDrags;
    bool inMouse;
};

kpColorCellsBase::kpColorCellsBase( QWidget *parent, int rows, int cols )
    : QTableWidget( parent ), d(new kpColorCellsBasePrivate(this))
{
    setFrameShape(QFrame::NoFrame);
    d->shade = true;
    setRowCount( rows );
    setColumnCount( cols );

    verticalHeader()->hide();
    horizontalHeader()->hide();

    d->colors = new QColor [ rows * cols ];

    for ( int i = 0; i < rows * cols; i++ )
        d->colors[i] = QColor();

    d->selected = 0;
    d->inMouse = false;

    // Drag'n'Drop
    setAcceptDrops( true);

    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    viewport()->setBackgroundRole( QPalette::Background );
    setBackgroundRole( QPalette::Background );
}

kpColorCellsBase::~kpColorCellsBase()
{
    delete [] d->colors;

    delete d;
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

void kpColorCellsBase::setSelected(int index)
{
    Q_ASSERT( index >= 0 && index < count() );

    d->selected = index;
}

int kpColorCellsBase::selectedIndex() const
{
    return d->selected;
}

void kpColorCellsBase::setColor( int column, const QColor &color )
{
    const int tableRow = column / columnCount();
    const int tableColumn = column % columnCount();

    Q_ASSERT( tableRow >= 0 && tableRow < rowCount() );
    Q_ASSERT( tableColumn >= 0 && tableColumn < columnCount() );

    QTableWidgetItem* tableItem = item(tableRow,tableColumn);

    if ( tableItem == 0 ) {
        tableItem = new QTableWidgetItem();
        setItem(tableRow,tableColumn,tableItem);
    }

	d->colors[column] = color;
    tableItem->setData(Qt::BackgroundRole , QBrush(color));
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

void kpColorCellsBase::resizeEvent( QResizeEvent* )
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

int kpColorCellsBase::sizeHintForColumn(int /*column*/) const
{
    return width() / columnCount() ;
}

int kpColorCellsBase::sizeHintForRow(int /*row*/) const
{
    return height() / rowCount() ;
}

void kpColorCellsBase::mousePressEvent( QMouseEvent *e )
{
    d->inMouse = true;
    d->mousePos = e->pos();
}


int kpColorCellsBase::positionToCell(const QPoint &pos, bool ignoreBorders) const
{
    //TODO ignoreBorders not yet handled
    Q_UNUSED( ignoreBorders )

   QTableWidgetItem* tableItem = itemAt(pos);

   if (!tableItem)
        return -1;

   const int itemRow = row(tableItem);
   const int itemColumn = column(tableItem);
   int cell = itemRow * columnCount() + itemColumn;

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
        int delay = KGlobalSettings::dndEventDelay();
        if(e->x() > d->mousePos.x()+delay || e->x() < d->mousePos.x()-delay ||
           e->y() > d->mousePos.y()+delay || e->y() < d->mousePos.y()-delay){
            // Drag color object
            int cell = positionToCell(d->mousePos);
            if ((cell != -1) && d->colors[cell].isValid())
            {
               KColorMimeData::createDrag( d->colors[cell], this)->start();
            }
        }
    }
}

void kpColorCellsBase::dragEnterEvent( QDragEnterEvent *event)
{
     kDebug () << "kpColorCellsBase::dragEnterEvent() acceptDrags="
               << d->acceptDrags
               << " canDecode=" << KColorMimeData::canDecode(event->mimeData())
               << endl;
     event->setAccepted( d->acceptDrags && KColorMimeData::canDecode( event->mimeData()));
}

// Reimplemented to override QTableWidget's override.  Else dropping doesn't work.
void kpColorCellsBase::dragMoveEvent (QDragMoveEvent *event)
{
     kDebug () << "kpColorCellsBase::dragMoveEvent() acceptDrags="
               << d->acceptDrags
               << " canDecode=" << KColorMimeData::canDecode(event->mimeData())
               << endl;
     event->setAccepted( d->acceptDrags && KColorMimeData::canDecode( event->mimeData()));
}

void kpColorCellsBase::dropEvent( QDropEvent *event)
{
     QColor c=KColorMimeData::fromMimeData(event->mimeData());
     kDebug () << "kpColorCellsBase::dropEvent() color.isValid=" << c.isValid();
     if( c.isValid()) {
          int cell = positionToCell(event->pos(), true);
	  setColor(cell,c);
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
	emit colorSelected( cell , color(cell) );
}

void kpColorCellsBase::mouseDoubleClickEvent( QMouseEvent * /*e*/ )
{
  int cell = positionToCell(d->mousePos);

  if (cell != -1)
    emit colorDoubleClicked( cell , color(cell) );
}


#include "kpColorCellsBase.moc"
