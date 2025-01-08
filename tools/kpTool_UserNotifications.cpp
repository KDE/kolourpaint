/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

//
// Tools' statusbar updates.
//

#include "kpToolPrivate.h"
#include "tools/kpTool.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

// public static
QString kpTool::cancelUserMessage(int mouseButton)
{
    if (mouseButton == 0) {
        return i18n("Right click to cancel.");
    }

    return i18n("Left click to cancel.");
}

//---------------------------------------------------------------------

// public
QString kpTool::cancelUserMessage() const
{
    return cancelUserMessage(d->mouseButton);
}

//---------------------------------------------------------------------

// public
QString kpTool::userMessage() const
{
    return d->userMessage;
}

//---------------------------------------------------------------------

// public
void kpTool::setUserMessage(const QString &userMessage)
{
    d->userMessage = userMessage;

    if (d->userMessage.isEmpty()) {
        d->userMessage = text();
    } else {
        d->userMessage.prepend(i18n("%1: ", text()));
    }

    Q_EMIT userMessageChanged(d->userMessage);
}

//---------------------------------------------------------------------

// public
QPoint kpTool::userShapeStartPoint() const
{
    return d->userShapeStartPoint;
}

//---------------------------------------------------------------------

// public
QPoint kpTool::userShapeEndPoint() const
{
    return d->userShapeEndPoint;
}

//---------------------------------------------------------------------

// public
void kpTool::setUserShapePoints(const QPoint &startPoint, const QPoint &endPoint, bool setSize)
{
    d->userShapeStartPoint = startPoint;
    d->userShapeEndPoint = endPoint;
    Q_EMIT userShapePointsChanged(d->userShapeStartPoint, d->userShapeEndPoint);

    if (setSize) {
        if (startPoint != KP_INVALID_POINT && endPoint != KP_INVALID_POINT) {
            setUserShapeSize(calculateLength(startPoint.x(), endPoint.x()), calculateLength(startPoint.y(), endPoint.y()));
        } else {
            setUserShapeSize();
        }
    }
}

//---------------------------------------------------------------------

// public
QSize kpTool::userShapeSize() const
{
    return d->userShapeSize;
}

//---------------------------------------------------------------------

// public
int kpTool::userShapeWidth() const
{
    return d->userShapeSize.width();
}

//---------------------------------------------------------------------

// public
int kpTool::userShapeHeight() const
{
    return d->userShapeSize.height();
}

//---------------------------------------------------------------------

// public
void kpTool::setUserShapeSize(const QSize &size)
{
    d->userShapeSize = size;

    Q_EMIT userShapeSizeChanged(d->userShapeSize);
}

//---------------------------------------------------------------------

// public
void kpTool::setUserShapeSize(int width, int height)
{
    setUserShapeSize(QSize(width, height));
}

//---------------------------------------------------------------------
