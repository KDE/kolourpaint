/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2017 Martin Koller <kollix@aon.at>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpToolPen.h"

#include "commands/tools/flow/kpToolFlowCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpImage.h"
#include "imagelib/kpPainter.h"

#include <KLocalizedString>

#include <QPainter>

//---------------------------------------------------------------------

kpToolPen::kpToolPen(kpToolEnvironment *environ, QObject *parent)
    : kpToolFlowBase(i18n("Pen"), i18n("Draws dots and freehand strokes"), Qt::Key_P, environ, parent, QStringLiteral("tool_pen"))
{
}

//---------------------------------------------------------------------

// protected virtual [base kpToolFlowBase]
QString kpToolPen::haventBegunDrawUserMessage() const
{
    return i18n("Click to draw dots or drag to draw strokes.");
}

//---------------------------------------------------------------------

// protected virtual [base kpToolFlowBase]
QRect kpToolPen::drawLine(const QPoint &thisPoint, const QPoint &lastPoint)
{
    QRect docRect = kpPainter::normalizedRect(thisPoint, lastPoint);
    docRect = neededRect(docRect, 1 /*pen width*/);
    kpImage image = document()->getImageAt(docRect);

    const QPoint sp = lastPoint - docRect.topLeft(), ep = thisPoint - docRect.topLeft();

    QPainter painter(&image);

    // never use AA - it does not look good for the usually very short lines
    // painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

    painter.setPen(color(mouseButton()).toQColor());
    painter.drawLine(sp, ep);

    document()->setImageAt(image, docRect.topLeft());
    return docRect;
}

//--------------------------------------------------------------------------------

#include "moc_kpToolPen.cpp"
