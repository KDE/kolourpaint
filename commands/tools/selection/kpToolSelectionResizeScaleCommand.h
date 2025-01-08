
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolSelectionResizeScaleCommand_H
#define kpToolSelectionResizeScaleCommand_H

#include <QObject>
#include <QPoint>

#include "commands/kpNamedCommand.h"

class QTimer;

class kpAbstractSelection;

// You could subclass kpToolResizeScaleCommand and/or
// kpToolSelectionMoveCommand instead if you want a disaster.
// This is different to kpToolResizeScaleCommand in that:
//
// 1. This only works for selections.
// 2. This is designed for the size and position to change several times
//    before execute().
//
// REFACTOR: Later: I take that all back.  We should merge with
//           kpToolResizeScaleCommand to reduce code duplication.
class kpToolSelectionResizeScaleCommand : public QObject, public kpNamedCommand
{
    Q_OBJECT

public:
    explicit kpToolSelectionResizeScaleCommand(kpCommandEnvironment *environ);
    ~kpToolSelectionResizeScaleCommand() override;

    kpCommandSize::SizeType size() const override;

public:
    const kpAbstractSelection *originalSelection() const;

    QPoint topLeft() const;
    void moveTo(const QPoint &point);

    int width() const;
    int height() const;
    void resize(int width, int height, bool delayed = false);

    // (equivalent to resize() followed by moveTo() but faster)
    void resizeAndMoveTo(int width, int height, const QPoint &point, bool delayed = false);

protected:
    void killSmoothScaleTimer();

    // If <delayed>, does a fast, low-quality scale and then calls itself
    // with <delayed> unset for a smooth scale, a short time later.
    // If acting on a text box, <delayed> is ignored.
    void resizeScaleAndMove(bool delayed = false);

public:
    void finalize();

public:
    void execute() override;
    void unexecute() override;

protected:
    kpAbstractSelection *m_originalSelectionPtr;

    QPoint m_newTopLeft;
    int m_newWidth, m_newHeight;

    QTimer *m_smoothScaleTimer;
};

#endif // kpToolSelectionResizeScaleCommand_H
