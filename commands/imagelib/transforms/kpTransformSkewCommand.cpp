
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


#include <kpTransformSkewCommand.h>

#include <qapplication.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmatrix.h>
#include <qpixmap.h>
#include <qpolygon.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpMainWindow.h>
#include <kpPixmapFX.h>
#include <kpSelection.h>
#include <kpTool.h>

// TODO: nasty, should avoid using GUI class in this command class
#include <kpTransformSkewDialog.h>


kpTransformSkewCommand::kpTransformSkewCommand (bool actOnSelection,
                                      int hangle, int vangle,
                                      kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_hangle (hangle), m_vangle (vangle),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor (actOnSelection) : kpColor::Invalid),
      m_oldPixmapPtr (0)
{
}

kpTransformSkewCommand::~kpTransformSkewCommand ()
{
    delete m_oldPixmapPtr;
}


// public virtual [base kpCommand]
QString kpTransformSkewCommand::name () const
{
    QString opName = i18n ("Skew");

    if (m_actOnSelection)
        return i18n ("Selection: %1", opName);
    else
        return opName;
}


// public virtual [base kpCommand]
int kpTransformSkewCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmapPtr) +
           m_oldSelection.size ();
}


// public virtual [base kpCommand]
void kpTransformSkewCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *doc->pixmap (m_actOnSelection);


    QPixmap newPixmap = kpPixmapFX::skew (*doc->pixmap (m_actOnSelection),
                                          kpTransformSkewDialog::horizontalAngleForPixmapFX (m_hangle),
                                          kpTransformSkewDialog::verticalAngleForPixmapFX (m_vangle),
                                          m_backgroundColor);

    if (m_actOnSelection)
    {
        kpSelection *sel = doc->selection ();

        // Save old selection
        m_oldSelection = *sel;


        // Calculate skewed points
        QPolygon currentPoints = sel->points ();
        currentPoints.translate (-currentPoints.boundingRect ().x (),
                                 -currentPoints.boundingRect ().y ());
        QMatrix skewMatrix = kpPixmapFX::skewMatrix (
            *doc->pixmap (m_actOnSelection),
            kpTransformSkewDialog::horizontalAngleForPixmapFX (m_hangle),
            kpTransformSkewDialog::verticalAngleForPixmapFX (m_vangle));
        currentPoints = skewMatrix.map (currentPoints);
        currentPoints.translate (-currentPoints.boundingRect ().x () + m_oldSelection.x (),
                                 -currentPoints.boundingRect ().y () + m_oldSelection.y ());


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
        #if DEBUG_KP_TOOL_SKEW
            kDebug () << "kpTransformSkewCommand::execute() currentPoints.boundingRect="
                       << currentPoints.boundingRect ()
                       << " newPixmap: w=" << newPixmap.width ()
                       << " h=" << newPixmap.height ()
                       << " (victim of rounding error and/or skewed-a-(rectangular)-pixmap-that-was-transparent-in-the-corners-making-sel-uselessly-bigger-than-needs-be))"
                       << endl;
        #endif
            doc->setSelection (kpSelection (kpSelection::Rectangle,
                                            QRect (currentPoints.boundingRect ().x (),
                                                   currentPoints.boundingRect ().y (),
                                                   newPixmap.width (),
                                                   newPixmap.height ()),
                                            newPixmap,
                                            m_oldSelection.transparency ()));
        }

        Q_ASSERT (m_mainWindow->tool ());
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    else
    {
        doc->setPixmap (newPixmap);
    }


    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpCommand]
void kpTransformSkewCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    QPixmap oldPixmap = *m_oldPixmapPtr;
    delete m_oldPixmapPtr; m_oldPixmapPtr = 0;


    if (!m_actOnSelection)
        doc->setPixmap (oldPixmap);
    else
    {
        kpSelection oldSelection = m_oldSelection;
        doc->setSelection (oldSelection);

        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }


    QApplication::restoreOverrideCursor ();
}


#include <kpTransformSkewCommand.moc>
