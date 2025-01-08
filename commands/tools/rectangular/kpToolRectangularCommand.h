
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_RECTANGULAR_COMMAND_H
#define KP_TOOL_RECTANGULAR_COMMAND_H

#include "commands/kpNamedCommand.h"
#include "tools/rectangular/kpToolRectangularBase.h"

class kpColor;

class kpToolRectangularCommand : public kpNamedCommand
{
public:
    kpToolRectangularCommand(const QString &name,
                             kpToolRectangularBase::DrawShapeFunc drawShapeFunc,
                             const QRect &rect,
                             const kpColor &fcolor,
                             int penWidth,
                             const kpColor &bcolor,
                             kpCommandEnvironment *environ);
    ~kpToolRectangularCommand() override;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    struct kpToolRectangularCommandPrivate *const d;
    kpToolRectangularCommand &operator=(const kpToolRectangularCommand &) const;
};

#endif // KP_TOOL_RECTANGULAR_COMMAND_H
