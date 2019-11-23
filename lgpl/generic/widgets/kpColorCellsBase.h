
// SYNC: Periodically merge in changes from:
//
//           trunk/KDE/kdelibs/kdeui/colors/kcolordialog.{h,cpp}
//
//       which this is a fork of.
//
//       Our changes can be merged back into KDE (grep for "Added for KolourPaint" and similar).

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

#include <QTableWidget>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(kpLogColorCollection)

/**
* A table of editable color cells.
*
* @author Martin Jones <mjones@kde.org>
*
* Added for KolourPaint:
*
* If you have not called setColor() for a cell, its widget will not exist.
* So it is possible to have "holes" in this rectangular table of cells.
* You can delete a cell widget by calling setColor() with an invalid QColor.
*
* An invariant is that color() returns an invalid color iff the cells' widget
* does not exist.  Note that:
*
* 1. You can double click on cells that don't contain a widget
* 2. You can drop onto -- but not drag from -- a cell that doesn't contain a
*    widget
*
* If a color is dragged and dropped to-and-from the same instance of this
* widget, then the colors in the source and destination cells are swapped
* (this is a "move action").
*
* If CTRL is held or they are not from the same instance, then the source
* cell's color is copied into the destination cell, without any change to
* the source cell (this is a "copy action").
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
   *
   * Specifying <rows> and <columns> was made optional for KolourPaint.
   */
  kpColorCellsBase( QWidget *parent, int rows = 0, int columns = 0 );
  ~kpColorCellsBase() override;

private:
  /** Added for KolourPaint. */
  void invalidateAllColors ();

public:
  /** Added for KolourPaint.
      WARNING: These are not virtual in QTableWidget.
  */
  void clear ();
  void clearContents ();

  /** Added for KolourPaint. */
  void setRowColumnCounts (int rows, int columns);

  /** Added for KolourPaint.
      WARNING: These are not virtual in QTableWidget.
  */
  void setColumnCount (int columns);
  void setRowCount (int rows);

  /** Sets the color in the given index in the table.

      The following behavior change was added for KolourPaint:

          If <col> is not valid, the cell widget at <index> is deleted.
  */
  void setColor( int index, const QColor &col );
  /** Returns the color at a given index in the table.
      If a cell widget does not exist at <index>, the invalid color is
      returned.
   */
  QColor color( int index ) const;
  /** Returns the total number of color cells in the table */
  int count() const;

  void setShading(bool shade);
  void setAcceptDrags(bool acceptDrags);

  /** Whether component cells should resize with the entire widget.
      Default is true.

      Added for KolourPaint.
  */
  void setCellsResizable(bool yes);

  /** Sets the currently selected cell to @p index */
  void setSelected(int index);
  /** Returns the index of the cell which is currently selected */
  int  selectedIndex() const;

Q_SIGNALS:
  /** Emitted when a color is selected in the table */
  void colorSelected( int index , const QColor& color );
  /** Emitted with the above.

      Added for KolourPaint.
  */
  void colorSelectedWhitButton( int index , const QColor& color, Qt::MouseButton button );

  /** Emitted when a color in the table is double-clicked */
  void colorDoubleClicked( int index , const QColor& color );

  /** Emitted when setColor() is called.
      This includes when a color is dropped onto the table, via drag-and-drop.

      Added for KolourPaint.
  */
  void colorChanged( int index , const QColor& color );

protected:
  /** Grays out the cells, when the object is disabled.
      Added for KolourPaint.
  */
  void changeEvent( QEvent* event ) override;

  // the three methods below are used to ensure equal column widths and row heights
  // for all cells and to update the widths/heights when the widget is resized
  int sizeHintForColumn(int column) const override;
  int sizeHintForRow(int column) const override;
  void resizeEvent( QResizeEvent* event ) override;

  void mouseReleaseEvent( QMouseEvent * ) override;
  void mousePressEvent( QMouseEvent * ) override;
  void mouseMoveEvent( QMouseEvent * ) override;
  void dragEnterEvent( QDragEnterEvent * ) override;
  void dragMoveEvent( QDragMoveEvent * ) override;
  void dropEvent( QDropEvent *) override;
  void mouseDoubleClickEvent( QMouseEvent * ) override;

  /** <allowEmptyCell> was added for KolourPaint. */
  int positionToCell(const QPoint &pos, bool ignoreBorders=false,
    bool allowEmptyCell=false) const;

private:
  class kpColorCellsBasePrivate;
  friend class kpColorCellsBasePrivate;
  kpColorCellsBasePrivate *const d;

  Q_DISABLE_COPY(kpColorCellsBase)
};

#endif		// kpColorCellsBase_H
