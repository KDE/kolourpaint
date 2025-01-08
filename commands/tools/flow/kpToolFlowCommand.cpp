
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_FLOW_COMMAND 0

#include "kpToolFlowCommand.h"

#include "document/kpDocument.h"
#include "imagelib/kpImage.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"
#include "views/manager/kpViewManager.h"

#include <QRect>

struct kpToolFlowCommandPrivate {
    kpImage image;
    QRect boundingRect;
};

kpToolFlowCommand::kpToolFlowCommand(const QString &name, kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , d(new kpToolFlowCommandPrivate())
{
    d->image = document()->image();
}

kpToolFlowCommand::~kpToolFlowCommand()
{
    delete d;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolFlowCommand::size() const
{
    return ImageSize(d->image);
}

// public virtual [base kpCommand]
void kpToolFlowCommand::execute()
{
    swapOldAndNew();
}

// public virtual [base kpCommand]
void kpToolFlowCommand::unexecute()
{
    swapOldAndNew();
}

// private
void kpToolFlowCommand::swapOldAndNew()
{
    if (d->boundingRect.isValid()) {
        const kpImage oldImage = document()->getImageAt(d->boundingRect);

        document()->setImageAt(d->image, d->boundingRect.topLeft());

        d->image = oldImage;
    }
}

// public
void kpToolFlowCommand::updateBoundingRect(const QPoint &point)
{
    updateBoundingRect(QRect(point, point));
}

// public
void kpToolFlowCommand::updateBoundingRect(const QRect &rect)
{
#if DEBUG_KP_TOOL_FLOW_COMMAND & 0
    qCDebug(kpLogCommands) << "kpToolFlowCommand::updateBoundingRect()  existing=" << d->boundingRect << " plus=" << rect;
#endif
    d->boundingRect = d->boundingRect.united(rect);
#if DEBUG_KP_TOOL_FLOW_COMMAND & 0
    qCDebug(kpLogCommands) << "\tresult=" << d->boundingRect;
#endif
}

// public
void kpToolFlowCommand::finalize()
{
    if (d->boundingRect.isValid()) {
        // Store only the needed part of doc image.
        d->image = kpTool::neededPixmap(d->image, d->boundingRect);
    } else {
        d->image = kpImage();
    }
}

// public
void kpToolFlowCommand::cancel()
{
    if (d->boundingRect.isValid()) {
        viewManager()->setFastUpdates();
        document()->setImageAt(d->image, d->boundingRect.topLeft());
        viewManager()->restoreFastUpdates();
    }
}
