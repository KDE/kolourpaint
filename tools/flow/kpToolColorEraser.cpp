/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_COLOR_ERASER 0

#include "kpToolColorEraser.h"

#include <QApplication>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "commands/kpCommandHistory.h"
#include "commands/kpMacroCommand.h"
#include "commands/tools/flow/kpToolFlowCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"

//--------------------------------------------------------------------------------

kpToolColorEraser::kpToolColorEraser(kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowBase(i18n("Color Eraser"),
                     i18n("Replaces pixels of the foreground color with the background color"),
                     Qt::Key_O,
                     environ,
                     parent,
                     QStringLiteral("tool_color_eraser"))
{
}

//--------------------------------------------------------------------------------

kpToolColorEraser::~kpToolColorEraser() = default;

//--------------------------------------------------------------------------------
// public virtual [base kpTool]

void kpToolColorEraser::globalDraw()
{
#if DEBUG_KP_TOOL_COLOR_ERASER
    qCDebug(kpLogTools) << "kpToolColorEraser::globalDraw()";
#endif
    if (!drawShouldProceed(QPoint() /*unused*/, QPoint() /*unused*/, QRect() /*unused*/)) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    environ()->flashColorSimilarityToolBarItem();

    kpToolFlowCommand *cmd = new kpToolFlowCommand(i18n("Color Eraser"), environ()->commandEnvironment());

    const QRect dirtyRect = kpPainter::washRect(document()->imagePointer(),
                                                0,
                                                0,
                                                document()->width(),
                                                document()->height(),
                                                backgroundColor() /*color to draw in*/,
                                                foregroundColor() /*color to replace*/,
                                                processedColorSimilarity());

    if (!dirtyRect.isEmpty()) {
        document()->slotContentsChanged(dirtyRect);

        cmd->updateBoundingRect(dirtyRect);
        cmd->finalize();

        commandHistory()->addCommand(cmd, false /* don't exec */);

        // don't delete - it's up to the commandHistory
        cmd = nullptr;
    } else {
#if DEBUG_KP_TOOL_COLOR_ERASER
        qCDebug(kpLogTools) << "\tisNOP";
#endif
        delete cmd;
        cmd = nullptr;
    }

    QApplication::restoreOverrideCursor();
}

//--------------------------------------------------------------------------------

QString kpToolColorEraser::haventBegunDrawUserMessage() const
{
    return i18n("Click or drag to erase pixels of the foreground color.");
}

//--------------------------------------------------------------------------------

bool kpToolColorEraser::drawShouldProceed(const QPoint & /*thisPoint*/, const QPoint & /*lastPoint*/, const QRect & /*normalizedRect*/)
{
    return !(foregroundColor() == backgroundColor() && processedColorSimilarity() == 0);
}

//--------------------------------------------------------------------------------

QRect kpToolColorEraser::drawLine(const QPoint &thisPoint, const QPoint &lastPoint)
{
#if DEBUG_KP_TOOL_COLOR_ERASER
    qCDebug(kpLogTools) << "kpToolColorEraser::drawLine(thisPoint=" << thisPoint << ",lastPoint=" << lastPoint << ")";
#endif

    environ()->flashColorSimilarityToolBarItem();

    const QRect dirtyRect = kpPainter::washLine(document()->imagePointer(),
                                                lastPoint.x(),
                                                lastPoint.y(),
                                                thisPoint.x(),
                                                thisPoint.y(),
                                                color(mouseButton()) /*color to draw in*/,
                                                brushWidth(),
                                                brushHeight(),
                                                color(1 - mouseButton()) /*color to replace*/,
                                                processedColorSimilarity());

#if DEBUG_KP_TOOL_COLOR_ERASER
    qCDebug(kpLogTools) << "\tdirtyRect=" << dirtyRect;
#endif

    if (!dirtyRect.isEmpty()) {
        document()->slotContentsChanged(dirtyRect);
        return dirtyRect;
    }

    return {};
}

//--------------------------------------------------------------------------------

#include "moc_kpToolColorEraser.cpp"
