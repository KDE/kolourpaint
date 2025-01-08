/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_SELECTION_DRAG_H
#define KP_SELECTION_DRAG_H

#include <QMimeData>

class kpAbstractImageSelection;

class kpSelectionDrag : public QMimeData
{
    Q_OBJECT

public:
    static const char *const SelectionMimeType;

    // ASSUMPTION: <sel> has content (is not just a border).
    explicit kpSelectionDrag(const kpAbstractImageSelection &sel);

public:
    static bool canDecode(const QMimeData *mimeData);
    static kpAbstractImageSelection *decode(const QMimeData *mimeData);
};

#endif // KP_SELECTION_DRAG_H
