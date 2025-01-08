
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpAbstractSelectionToolPrivate_H
#define kpAbstractSelectionToolPrivate_H

#include "kpAbstractSelectionTool.h"

#include <QPoint>

class QTimer;

class kpAbstractSelectionContentCommand;
class kpToolSelectionMoveCommand;
class kpToolSelectionResizeScaleCommand;
class kpToolWidgetOpaqueOrTransparent;

struct kpAbstractSelectionToolPrivate {
    kpAbstractSelectionTool::DrawType drawType;
    kpAbstractSelectionContentCommand *currentSelContentCommand;

    // Whether the drag has been substantial enough to be considered as a
    // non-NOP drag.  The "substantial enough" criteria is draw-type
    // dependent and is usually based on how far the mouse has been
    // dragged.  See kpAbstractSelectionTool's API Doc for details.
    bool dragAccepted;

    bool hadSelectionBeforeDraw;

    bool cancelledShapeButStillHoldingButtons;

    kpToolWidgetOpaqueOrTransparent *toolWidgetOpaqueOrTransparent;

    //
    // Create
    //

    QTimer *createNOPTimer;

    //
    // Move
    //

    kpToolSelectionMoveCommand *currentMoveCommand;
    bool currentMoveCommandIsSmear;

    QPoint startMoveDragFromSelectionTopLeft;

    QTimer *RMBMoveUpdateGUITimer;

    //
    // Resize / Scale
    //

    kpToolSelectionResizeScaleCommand *currentResizeScaleCommand;
    int resizeScaleType;
};

#endif // kpAbstractSelectionToolPrivate_H
