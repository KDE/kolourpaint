
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_FLOW_COMMAND_H
#define KP_TOOL_FLOW_COMMAND_H

#include "commands/kpNamedCommand.h"

class QPoint;
class QRect;

class kpToolFlowCommand : public kpNamedCommand
{
public:
    kpToolFlowCommand(const QString &name, kpCommandEnvironment *environ);
    ~kpToolFlowCommand() override;

    kpCommandSize::SizeType size() const override;

    void execute() override;
    void unexecute() override;

    // interface for kpToolFlowBase
    void updateBoundingRect(const QPoint &point);
    void updateBoundingRect(const QRect &rect);
    void finalize();
    void cancel();

private:
    void swapOldAndNew();

    struct kpToolFlowCommandPrivate *const d;
};

#endif // KP_TOOL_FLOW_COMMAND_H
