
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpColorCells_H
#define kpColorCells_H

#include <QUrl>

#include "lgpl/generic/widgets/kpColorCellsBase.h"

class kpColorCollection;

class kpColor;

// This widget consists of rows of 11 cells of colors.  The cells become
// shorter as soon as there are 3 rows.  After that, a vertical scrollbar
// is usually displayed.
//
// By default, it is set to the DefaultColorCollection(), with 2 rows.
//
//
// Cell widgets might not exist for 2 reasons:
//
// 1. The respective colors in the color collection are invalid.
//    An easy way to create this situation is to appendRow().
//
// 2. The number of colors in the color collection is not divisible by
//    the columnCount() [currently fixed at 11], so some cells in the
//    last row might not be linked to colors in the color collection.
//
// Currently, this class always ensures that there is at least one
// visual/table row (which might contain no cell widgets).
//
//
// To determine where the color collection came from:
//
// 1. If url() is non-empty, it came from a file.
//
// 2. If url() is empty:
//    a) If name() is non-empty, it came from KDE-managed color collection.
//    b) If name() is empty, it came from DefaultColorCollection().
//
//
// See also the API documentation for kpColorCellsBase.
//
// TODO: For now, only horizontal orientation is supported.
class kpColorCells : public kpColorCellsBase
{
    Q_OBJECT

public:
    explicit kpColorCells(QWidget *parent, Qt::Orientation o = Qt::Horizontal);
    ~kpColorCells() override;

    static kpColorCollection DefaultColorCollection();

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation o);

protected:
    void makeCellsMatchColorCollection();

public:
    bool isModified() const;
    // (this emits isModifiedChanged() if the modified state changes)
    void setModified(bool yes);
public Q_SLOTS:
    // (this emits isModifiedChanged() if the modified state changes)
    void setModified();

public:
    // The source URL of the kpColorCollection.  Empty for color
    // collections that did not come from files.
    QUrl url() const;

    // The name of the kpColorCollection.  Empty for color collections
    // managed by KDE.
    QString name() const;

    const kpColorCollection *colorCollection() const;

private:
    // Ensures there's a least one row of cells, to avoid a confusing UI.
    void ensureHaveAtLeastOneRow();

public:
    void setColorCollection(const kpColorCollection &colorCol, const QUrl &url = QUrl());

    bool openColorCollection(const QUrl &url);
    bool saveColorCollectionAs(const QUrl &url);
    bool saveColorCollection();

    // These add and delete visual/table rows, independent of whether the number
    // of colors in the color collection is divisible by the columnCount()
    // [currently fixed at 11].
    //
    // For instance, if you only had 15 colors in the color collection, there are
    // visually 2 rows (22 cells in total):
    //
    // 1. appendRow() will add a visual row so that there will be 3 visual rows
    //    (33 cells in total).  (22 - 15) + 11 invalid colors will be added to
    //    the back of the color collection.  Note that invalid colors are not
    //    saved by kpColorCollection, so those new cells not initialized by the
    //    user will not be saved.
    // 2. deleteRow() will delete a visual row so that there will be 1 visual
    //    row (11 cells in total) remaining.  (15 - 11) colors will be deleted
    //    from the back of the color collection.
    void appendRow();
    void deleteLastRow();

Q_SIGNALS:
    void foregroundColorChanged(const kpColor &color);
    void backgroundColorChanged(const kpColor &color);

    void rowCountChanged(int rowCount);

    void nameChanged(const QString &name);
    void urlChanged(const QUrl &url);

    // Emitted when setModified() is called and the modified state changes.
    // It may be called at other times, even when the modified state did
    // not change.
    void isModifiedChanged(bool isModified);

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

protected Q_SLOTS:
    void slotColorSelected(int cell, const QColor &color, Qt::MouseButton button);
    void slotColorDoubleClicked(int cell, const QColor &color);
    void slotColorChanged(int cell, const QColor &color);

private:
    struct kpColorCellsPrivate *const d;
};

#endif // kpColorCells_H
