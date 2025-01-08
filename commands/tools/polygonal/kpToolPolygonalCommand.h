
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
