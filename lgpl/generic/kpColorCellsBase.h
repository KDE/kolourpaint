/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//----------------------------------------------------------------------
// KDE color selection dialog.

// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>

#ifndef kpColorCellsBase_H
#define kpColorCellsBase_H

#include <kolourpaint_lgpl_export.h>

#include <QtGui/QTableWidget>

/**
* A table of editable color cells.
*
* @author Martin Jones <mjones@kde.org>
*/
class KOLOURPAINT_LGPL_EXPORT kpColorCellsBase : public QTableWidget
{
  Q_OBJECT
public:
  /** 
   * Constructs a new table of color cells, consisting of
   * @p rows * @p columns colors. 
   *
   * @param parent The parent of the new widget
   * @param rows The number of rows in the table
   * @param columns The number of columns in the table
   */
  kpColorCellsBase( QWidget *parent, int rows, int columns );
  ~kpColorCellsBase();

  /** Sets the color in the given index in the table */
  void setColor( int index, const QColor &col );
  /** Returns the color at a given index in the table */
  QColor color( int index ) const;
  /** Returns the total number of color cells in the table */
  int count() const;

  void setShading(bool shade);
  void setAcceptDrags(bool acceptDrags);

  /** Sets the currently selected cell to @p index */
  void setSelected(int index);
  /** Returns the index of the cell which is currently selected */
  int  selectedIndex() const;
  
Q_SIGNALS:
  /** Emitted when a color is selected in the table */
  void colorSelected( int index , const QColor& color );
  /** Emitted when a color in the table is double-clicked */
  void colorDoubleClicked( int index , const QColor& color );

protected:
  // the three methods below are used to ensure equal column widths and row heights
  // for all cells and to update the widths/heights when the widget is resized
  virtual int sizeHintForColumn(int column) const;
  virtual int sizeHintForRow(int column) const;
  virtual void resizeEvent( QResizeEvent* event );

  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent * );
  virtual void dragMoveEvent( QDragMoveEvent * );
  virtual void dropEvent( QDropEvent *);
  virtual void mouseDoubleClickEvent( QMouseEvent * );

  int positionToCell(const QPoint &pos, bool ignoreBorders=false) const;

private:
  class kpColorCellsBasePrivate;
  friend class kpColorCellsBasePrivate;
  kpColorCellsBasePrivate *const d;
  
  Q_DISABLE_COPY(kpColorCellsBase)
};

#endif		// kpColorCellsBase_H
