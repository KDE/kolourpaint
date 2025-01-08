
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolPolygonalCommand_H
#define kpToolPolygonalCommand_H

#include "commands/kpNamedCommand.h"
#include "tools/polygonal/kpToolPolygonalBase.h"

class QPolygon;
class QRect;

class kpColor;

// TODO: merge with kpToolRectangularCommand due to code duplication.
class kpToolPolygonalCommand : public kpNamedCommand
{
public:
    // <boundingRect> = the bounding rectangle for <points> including <penWidth>.
    kpToolPolygonalCommand(const QString &name,
                           kpToolPolygonalBase::DrawShapeFunc drawShapeFunc,
                           const QPolygon &points,
                           const QRect &boundingRect,
                           const kpColor &fcolor,
                           int penWidth,
                           const kpColor &bcolor,
                           kpCommandEnvironment *environ);
    ~kpToolPolygonalCommand() override;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    struct kpToolPolygonalCommandPrivate *const d;
    kpToolPolygonalCommand &operator=(const kpToolPolygonalCommand &) const;
};

#endif // kpToolPolygonalCommand_H
