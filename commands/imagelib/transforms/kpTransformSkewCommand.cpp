
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


#define DEBUG_KP_TOOL_SKEW 0
#define DEBUG_KP_TOOL_SKEW_DIALOG 0


#include "kpTransformSkewCommand.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"
#include "pixmapfx/kpPixmapFX.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "kpLogCategories.h"

#include <QApplication>
#include <QTransform>
#include <QPolygon>

// TODO: nasty, should avoid using GUI class in this command class
#include "dialogs/imagelib/transforms/kpTransformSkewDialog.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpTransformSkewCommand::kpTransformSkewCommand (bool actOnSelection,
        int hangle, int vangle,
        kpCommandEnvironment *environ)
    : kpCommand (environ),
      m_actOnSelection (actOnSelection),
      m_hangle (hangle), m_vangle (vangle),
      m_backgroundColor (environ->backgroundColor (actOnSelection)),
      m_oldSelectionPtr (nullptr)
{
}

kpTransformSkewCommand::~kpTransformSkewCommand ()
{
    delete m_oldSelectionPtr;
}


// public virtual [base kpCommand]
QString kpTransformSkewCommand::name () const
{
    QString opName = i18n ("Skew");

    return (m_actOnSelection) ? i18n ("Selection: %1", opName) : opName;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpTransformSkewCommand::size () const
{
    return ImageSize (m_oldImage) +
           SelectionSize (m_oldSelectionPtr);
}


// public virtual [base kpCommand]
void kpTransformSkewCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    kpImage newImage = kpPixmapFX::skew (doc->image (m_actOnSelection),
                                          kpTransformSkewDialog::horizontalAngleForPixmapFX (m_hangle),
                                          kpTransformSkewDialog::verticalAngleForPixmapFX (m_vangle),
                                          m_backgroundColor);

    if (!m_actOnSelection)
    {
        m_oldImage = doc->image (m_actOnSelection);

        doc->setImage (newImage);
    }
    else
    {
        kpAbstractImageSelection *sel = doc->imageSelection ();
        Q_ASSERT (sel);

        // Save old selection
        m_oldSelectionPtr = sel->clone ();


        // Calculate skewed points
        QPolygon currentPoints = sel->calculatePoints ();
        currentPoints.translate (-currentPoints.boundingRect ().x (),
                                 -currentPoints.boundingRect ().y ());
        QTransform skewMatrix = kpPixmapFX::skewMatrix (
            doc->image (m_actOnSelection),
            kpTransformSkewDialog::horizontalAngleForPixmapFX (m_hangle),
            kpTransformSkewDialog::verticalAngleForPixmapFX (m_vangle));
        currentPoints = skewMatrix.map (currentPoints);
        currentPoints.translate (-currentPoints.boundingRect ().x () + m_oldSelectionPtr->x (),
                                 -currentPoints.boundingRect ().y () + m_oldSelectionPtr->y ());


        if (currentPoints.boundingRect ().width () == newImage.width () &&
            currentPoints.boundingRect ().height () == newImage.height ())
        {
            doc->setSelection (
                kpFreeFormImageSelection (
                    currentPoints, newImage,
                    m_oldSelectionPtr->transparency ()));
        }
        else
        {
            // TODO: fix the latter "victim of" problem in kpAbstractImageSelection by
            //       allowing the border width & height != pixmap width & height
            //       Or maybe autocrop?
        #if DEBUG_KP_TOOL_SKEW
            qCDebug(kpLogCommands) << "kpTransformSkewCommand::execute() currentPoints.boundingRect="
                       << currentPoints.boundingRect ()
                       << " newPixmap: w=" << newImage.width ()
                       << " h=" << newImage.height ()
                       << " (victim of rounding error and/or skewed-a-(rectangular)-pixmap-that-was-transparent-in-the-corners-making-sel-uselessly-bigger-than-needs-be))";
        #endif
            doc->setSelection (
                kpRectangularImageSelection (
                    QRect (currentPoints.boundingRect ().x (),
                        currentPoints.boundingRect ().y (),
                        newImage.width (),
                        newImage.height ()),
                    newImage,
                    m_oldSelectionPtr->transparency ()));
        }

        environ ()->somethingBelowTheCursorChanged ();
    }


    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpCommand]
void kpTransformSkewCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    if (!m_actOnSelection)
    {
        doc->setImage (m_oldImage);
        m_oldImage = kpImage ();
    }
    else
    {
        doc->setSelection (*m_oldSelectionPtr);
        delete m_oldSelectionPtr; m_oldSelectionPtr = nullptr;

        environ ()->somethingBelowTheCursorChanged ();
    }


    QApplication::restoreOverrideCursor ();
}

