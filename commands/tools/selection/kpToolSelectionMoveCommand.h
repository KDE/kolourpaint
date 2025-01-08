
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolSelectionMoveCommand_H
#define kpToolSelectionMoveCommand_H

#include <QPoint>
#include <QPolygon>
#include <QRect>

#include "commands/kpNamedCommand.h"
#include "imagelib/kpImage.h"

class kpAbstractSelection;

class kpToolSelectionMoveCommand : public kpNamedCommand
{
public:
    kpToolSelectionMoveCommand(const QString &name, kpCommandEnvironment *environ);
    ~kpToolSelectionMoveCommand() override;

    kpAbstractSelection *originalSelectionClone() const;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

    void moveTo(const QPoint &point, bool moveLater = false);
    void moveTo(int x, int y, bool moveLater = false);
    void copyOntoDocument();
    void finalize();

private:
    QPoint m_startPoint, m_endPoint;

    kpImage m_oldDocumentImage;

    // area of document affected (not the bounding rect of the sel)
    QRect m_documentBoundingRect;

    QPolygon m_copyOntoDocumentPoints;
};

#endif // kpToolSelectionMoveCommand_H
