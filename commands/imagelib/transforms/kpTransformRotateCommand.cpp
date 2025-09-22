
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_ROTATE 0

#include "kpTransformRotateCommand.h"

#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "pixmapfx/kpPixmapFX.h"
#include "views/manager/kpViewManager.h"

#include <QApplication>
#include <QPolygon>
#include <QTransform>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpTransformRotateCommand::kpTransformRotateCommand(bool actOnSelection, double angle, kpCommandEnvironment *environ)
    : kpCommand(environ)
    , m_actOnSelection(actOnSelection)
    , m_angle(angle)
    , m_backgroundColor(environ->backgroundColor(actOnSelection))
    , m_losslessRotation(kpPixmapFX::isLosslessRotation(angle))
    , m_oldSelectionPtr(nullptr)
{
}

kpTransformRotateCommand::~kpTransformRotateCommand()
{
    delete m_oldSelectionPtr;
}

// public virtual [base kpCommand]
QString kpTransformRotateCommand::name() const
{
    QString opName = i18n("Rotate");

    return (m_actOnSelection) ? i18n("Selection: %1", opName) : opName;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpTransformRotateCommand::size() const
{
    return ImageSize(m_oldImage) + SelectionSize(m_oldSelectionPtr);
}

// public virtual [base kpCommand]
void kpTransformRotateCommand::execute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (!m_losslessRotation) {
        m_oldImage = doc->image(m_actOnSelection);
    }

    kpImage newImage = kpPixmapFX::rotate(doc->image(m_actOnSelection), m_angle, m_backgroundColor);

    if (!m_actOnSelection) {
        doc->setImage(newImage);
    } else {
        kpAbstractImageSelection *sel = doc->imageSelection();
        Q_ASSERT(sel);

        // Save old selection
        m_oldSelectionPtr = sel->clone();

        // Conserve memory:
        //
        // 1. If it's a lossless rotation, we don't need to the store old
        //    image anywhere at all, as we can reconstruct it by rotating in
        //    reverse.
        // 2. If it's not a lossless rotation, "m_oldImage" already holds
        //    a copy of the old image.  In this case, we actually save very
        //    little with this line (just, the computed transparency mask) since
        //    kpImage is copy-on-write.
        m_oldSelectionPtr->setBaseImage(kpImage());

        // Calculate new top left (so selection rotates about center)
        // (the Times2 trickery is used to reduce integer division error without
        //  resorting to the troublesome world of floating point)
        QPoint oldCenterTimes2(sel->x() * 2 + sel->width(), sel->y() * 2 + sel->height());
        QPoint newTopLeftTimes2(oldCenterTimes2 - QPoint(newImage.width(), newImage.height()));
        QPoint newTopLeft(newTopLeftTimes2.x() / 2, newTopLeftTimes2.y() / 2);

        // Calculate rotated points
        QPolygon currentPoints = sel->calculatePoints();
        currentPoints.translate(-currentPoints.boundingRect().x(), -currentPoints.boundingRect().y());
        QTransform rotateMatrix = kpPixmapFX::rotateMatrix(doc->image(m_actOnSelection), m_angle);
        currentPoints = rotateMatrix.map(currentPoints);
        currentPoints.translate(-currentPoints.boundingRect().x() + newTopLeft.x(), -currentPoints.boundingRect().y() + newTopLeft.y());

        if (currentPoints.boundingRect().width() == newImage.width() && currentPoints.boundingRect().height() == newImage.height()) {
            doc->setSelection(kpFreeFormImageSelection(currentPoints, newImage, m_oldSelectionPtr->transparency()));
        } else {
            // TODO: fix the latter "victim of" problem in kpAbstractImageSelection by
            //       allowing the border width & height != pixmap width & height
            //       Or maybe autocrop?
#if DEBUG_KP_TOOL_ROTATE
            qCDebug(kpLogCommands) << "kpTransformRotateCommand::execute() currentPoints.boundingRect=" << currentPoints.boundingRect()
                                   << " newPixmap: w=" << newImage.width() << " h=" << newImage.height()
                                   << " (victim of rounding error and/or "
                                      "rotated-a-(rectangular)-pixmap-that-was-transparent-in-the-corners-making-sel-uselessly-bigger-than-needs-be)";
#endif
            doc->setSelection(kpRectangularImageSelection(QRect(newTopLeft.x(), newTopLeft.y(), newImage.width(), newImage.height()),
                                                          newImage,
                                                          m_oldSelectionPtr->transparency()));
        }

        environ()->somethingBelowTheCursorChanged();
    }

    QApplication::restoreOverrideCursor();
}

// public virtual [base kpCommand]
void kpTransformRotateCommand::unexecute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    kpImage oldImage;

    if (!m_losslessRotation) {
        oldImage = m_oldImage;
        m_oldImage = kpImage();
    } else {
        oldImage = kpPixmapFX::rotate(doc->image(m_actOnSelection), 360 - m_angle, m_backgroundColor);
    }

    if (!m_actOnSelection) {
        doc->setImage(oldImage);
    } else {
        m_oldSelectionPtr->setBaseImage(oldImage);
        doc->setSelection(*m_oldSelectionPtr);
        delete m_oldSelectionPtr;
        m_oldSelectionPtr = nullptr;

        environ()->somethingBelowTheCursorChanged();
    }

    QApplication::restoreOverrideCursor();
}
