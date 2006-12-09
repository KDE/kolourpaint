
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_TOOL_ROTATE 0


#include <kpTransformRotateCommand.h>

#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qmatrix.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kpviewmanager.h>


kpTransformRotateCommand::kpTransformRotateCommand (bool actOnSelection,
                                          double angle,
                                          kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_angle (angle),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor (actOnSelection) : kpColor::Invalid),
      m_losslessRotation (kpPixmapFX::isLosslessRotation (angle))
{
}

kpTransformRotateCommand::~kpTransformRotateCommand ()
{
}


// public virtual [base kpCommand]
QString kpTransformRotateCommand::name () const
{
    QString opName = i18n ("Rotate");

    if (m_actOnSelection)
        return i18n ("Selection: %1", opName);
    else
        return opName;
}


// public virtual [base kpCommand]
int kpTransformRotateCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmap) +
           m_oldSelection.size ();
}


// public virtual [base kpCommand]
void kpTransformRotateCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    if (!m_losslessRotation)
        m_oldPixmap = *doc->pixmap (m_actOnSelection);


    QPixmap newPixmap = kpPixmapFX::rotate (*doc->pixmap (m_actOnSelection),
                                            m_angle,
                                            m_backgroundColor);


    if (m_actOnSelection)
    {
        kpSelection *sel = doc->selection ();

        // Save old selection
        m_oldSelection = *sel;
        m_oldSelection.setPixmap (QPixmap ());


        // Calculate new top left (so selection rotates about center)
        // (the Times2 trickery is used to reduce integer division error without
        //  resorting to the troublesome world of floating point)
        QPoint oldCenterTimes2 (sel->x () * 2 + sel->width (),
                                sel->y () * 2 + sel->height ());
        QPoint newTopLeftTimes2 (oldCenterTimes2 - QPoint (newPixmap.width (), newPixmap.height ()));
        QPoint newTopLeft (newTopLeftTimes2.x () / 2, newTopLeftTimes2.y () / 2);


        // Calculate rotated points
        QPolygon currentPoints = sel->points ();
        currentPoints.translate (-currentPoints.boundingRect ().x (),
                                 -currentPoints.boundingRect ().y ());
        QMatrix rotateMatrix = kpPixmapFX::rotateMatrix (*doc->pixmap (m_actOnSelection), m_angle);
        currentPoints = rotateMatrix.map (currentPoints);
        currentPoints.translate (-currentPoints.boundingRect ().x () + newTopLeft.x (),
                                 -currentPoints.boundingRect ().y () + newTopLeft.y ());


        if (currentPoints.boundingRect ().width () == newPixmap.width () &&
            currentPoints.boundingRect ().height () == newPixmap.height ())
        {
            doc->setSelection (kpSelection (currentPoints, newPixmap,
                                            m_oldSelection.transparency ()));
        }
        else
        {
            // TODO: fix the latter "victim of" problem in kpSelection by
            //       allowing the border width & height != pixmap width & height
            //       Or maybe autocrop?
        #if DEBUG_KP_TOOL_ROTATE
            kDebug () << "kpTransformRotateCommand::execute() currentPoints.boundingRect="
                       << currentPoints.boundingRect ()
                       << " newPixmap: w=" << newPixmap.width ()
                       << " h=" << newPixmap.height ()
                       << " (victim of rounding error and/or rotated-a-(rectangular)-pixmap-that-was-transparent-in-the-corners-making-sel-uselessly-bigger-than-needs-be)"
                       << endl;
        #endif
            doc->setSelection (kpSelection (kpSelection::Rectangle,
                                            QRect (newTopLeft.x (), newTopLeft.y (),
                                                   newPixmap.width (), newPixmap.height ()),
                                            newPixmap,
                                            m_oldSelection.transparency ()));
        }

        Q_ASSERT (m_mainWindow->tool ());
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    else
        doc->setPixmap (newPixmap);


    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpCommand]
void kpTransformRotateCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    QPixmap oldPixmap;

    if (!m_losslessRotation)
    {
        oldPixmap = m_oldPixmap;
        m_oldPixmap = QPixmap();
    }
    else
    {
        oldPixmap = kpPixmapFX::rotate (*doc->pixmap (m_actOnSelection),
                                        360 - m_angle,
                                        m_backgroundColor);
    }


    if (!m_actOnSelection)
        doc->setPixmap (oldPixmap);
    else
    {
        kpSelection oldSelection = m_oldSelection;
        oldSelection.setPixmap (oldPixmap);
        doc->setSelection (oldSelection);

        Q_ASSERT (m_mainWindow->tool ());
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }


    QApplication::restoreOverrideCursor ();
}


#include <kpTransformRotateCommand.moc>
