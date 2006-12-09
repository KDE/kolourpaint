
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


#define DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND 0
#define DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG 0


#include <kpTransformResizeScaleCommand.h>

#include <math.h>

#include <q3accel.h>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qsize.h>
#include <qtoolbutton.h>
#include <qmatrix.h>


#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <khbox.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>


kpTransformResizeScaleCommand::kpTransformResizeScaleCommand (bool actOnSelection,
                                                    int newWidth, int newHeight,
                                                    Type type,
                                                    kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_type (type),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : kpColor::Invalid),
      m_oldSelection (0)
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    m_oldWidth = doc->width (m_actOnSelection);
    m_oldHeight = doc->height (m_actOnSelection);

    m_actOnTextSelection = (m_actOnSelection &&
                            doc->selection () &&
                            doc->selection ()->isText ());

    resize (newWidth, newHeight);

    // If we have a selection _border_ (but not a floating selection),
    // then scale the selection with the document
    m_scaleSelectionWithImage = (!m_actOnSelection &&
                                 (m_type == Scale || m_type == SmoothScale) &&
                                 document ()->selection () &&
                                 !document ()->selection ()->pixmap ());
}

kpTransformResizeScaleCommand::~kpTransformResizeScaleCommand ()
{
    delete m_oldSelection;
}


// public virtual [base kpCommand]
QString kpTransformResizeScaleCommand::name () const
{
    if (m_actOnSelection)
    {
        if (m_actOnTextSelection)
        {
            if (m_type == Resize)
                return i18n ("Text: Resize Box");
        }
        else
        {
            if (m_type == Scale)
                return i18n ("Selection: Scale");
            else if (m_type == SmoothScale)
                return i18n ("Selection: Smooth Scale");
        }
    }
    else
    {
        switch (m_type)
        {
        case Resize:
            return i18n ("Resize");
        case Scale:
            return i18n ("Scale");
        case SmoothScale:
            return i18n ("Smooth Scale");
        }
    }

    return QString::null;
}

// public virtual [base kpCommand]
int kpTransformResizeScaleCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmap) +
           kpPixmapFX::pixmapSize (m_oldRightPixmap) +
           kpPixmapFX::pixmapSize (m_oldBottomPixmap) +
           (m_oldSelection ? m_oldSelection->size () : 0);
}


// public
int kpTransformResizeScaleCommand::newWidth () const
{
    return m_newWidth;
}

// public
void kpTransformResizeScaleCommand::setNewWidth (int width)
{
    resize (width, newHeight ());
}


// public
int kpTransformResizeScaleCommand::newHeight () const
{
    return m_newHeight;
}

// public
void kpTransformResizeScaleCommand::setNewHeight (int height)
{
    resize (newWidth (), height);
}


// public
QSize kpTransformResizeScaleCommand::newSize () const
{
    return QSize (newWidth (), newHeight ());
}

// public virtual
void kpTransformResizeScaleCommand::resize (int width, int height)
{
    m_newWidth = width;
    m_newHeight = height;

    m_isLosslessScale = ((m_type == Scale) &&
                         (m_newWidth / m_oldWidth * m_oldWidth == m_newWidth) &&
                         (m_newHeight / m_oldHeight * m_oldHeight == m_newHeight));
}


// public
bool kpTransformResizeScaleCommand::scaleSelectionWithImage () const
{
    return m_scaleSelectionWithImage;
}


// private
void kpTransformResizeScaleCommand::scaleSelectionRegionWithDocument ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    kDebug () << "kpTransformResizeScaleCommand::scaleSelectionRegionWithDocument"
               << endl;
#endif

    Q_ASSERT (m_oldSelection);
    Q_ASSERT (!m_oldSelection->pixmap ());


    const double horizScale = double (m_newWidth) / double (m_oldWidth);
    const double vertScale = double (m_newHeight) / double (m_oldHeight);

    const int newX = (int) (m_oldSelection->x () * horizScale);
    const int newY = (int) (m_oldSelection->y () * vertScale);


    QPolygon currentPoints = m_oldSelection->points ();
    currentPoints.translate (-currentPoints.boundingRect ().x (),
                             -currentPoints.boundingRect ().y ());

    // TODO: refactor into kpPixmapFX
    QMatrix scaleMatrix;
    scaleMatrix.scale (horizScale, vertScale);
    currentPoints = scaleMatrix.map (currentPoints);

    currentPoints.translate (
        -currentPoints.boundingRect ().x () + newX,
        -currentPoints.boundingRect ().y () + newY);

    document ()->setSelection (kpSelection (currentPoints, QPixmap (),
                                            m_oldSelection->transparency ()));


    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
}


// public virtual [base kpCommand]
void kpTransformResizeScaleCommand::execute ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    kDebug () << "kpTransformResizeScaleCommand::execute() type="
               << (int) m_type
               << " oldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight
               << " newWidth=" << m_newWidth
               << " newHeight=" << m_newHeight
               << endl;
#endif

    if (m_oldWidth == m_newWidth && m_oldHeight == m_newHeight)
        return;

    if (m_type == Resize)
    {
        if (m_actOnSelection)
        {
            if (!m_actOnTextSelection)
            {
                Q_ASSERT (!"kpTransformResizeScaleCommand::execute() resizing sel doesn't make sense");
                return;
            }
            else
            {
                QApplication::setOverrideCursor (Qt::WaitCursor);
                document ()->selection ()->textResize (m_newWidth, m_newHeight);

                Q_ASSERT (m_mainWindow->tool ());
                m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

                QApplication::restoreOverrideCursor ();
            }
        }
        else
        {
            QApplication::setOverrideCursor (Qt::WaitCursor);


            if (m_newWidth < m_oldWidth)
            {
                m_oldRightPixmap = kpPixmapFX::getPixmapAt (
                    *document ()->pixmap (),
                    QRect (m_newWidth, 0,
                        m_oldWidth - m_newWidth, m_oldHeight));
            }

            if (m_newHeight < m_oldHeight)
            {
                m_oldBottomPixmap = kpPixmapFX::getPixmapAt (
                    *document ()->pixmap (),
                    QRect (0, m_newHeight,
                        m_newWidth, m_oldHeight - m_newHeight));
            }

            document ()->resize (m_newWidth, m_newHeight, m_backgroundColor);


            QApplication::restoreOverrideCursor ();
        }
    }
    else
    {
        QApplication::setOverrideCursor (Qt::WaitCursor);


        QPixmap oldPixmap = *document ()->pixmap (m_actOnSelection);

        if (!m_isLosslessScale)
            m_oldPixmap = oldPixmap;

        QPixmap newPixmap = kpPixmapFX::scale (oldPixmap, m_newWidth, m_newHeight,
                                               m_type == SmoothScale);


        if (!m_oldSelection && document ()->selection ())
        {
            // Save sel border
            m_oldSelection = new kpSelection (*document ()->selection ());
            m_oldSelection->setPixmap (QPixmap ());
        }

        if (m_actOnSelection)
        {
            Q_ASSERT (m_oldSelection);
            QRect newRect = QRect (m_oldSelection->x (), m_oldSelection->y (),
                                   newPixmap.width (), newPixmap.height ());

            // Not possible to retain non-rectangular selection borders on scale
            // (think about e.g. a 45 deg line as part of the border & 2x scale)
            document ()->setSelection (
                kpSelection (kpSelection::Rectangle, newRect, newPixmap,
                             m_oldSelection->transparency ()));

            Q_ASSERT (m_mainWindow->tool ());
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
        }
        else
        {
            document ()->setPixmap (newPixmap);

            if (m_scaleSelectionWithImage)
            {
                scaleSelectionRegionWithDocument ();
            }
        }


        QApplication::restoreOverrideCursor ();
    }
}

// public virtual [base kpCommand]
void kpTransformResizeScaleCommand::unexecute ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    kDebug () << "kpTransformResizeScaleCommand::unexecute() type="
               << m_type << endl;
#endif

    if (m_oldWidth == m_newWidth && m_oldHeight == m_newHeight)
        return;

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    if (m_type == Resize)
    {
        if (m_actOnSelection)
        {
            if (!m_actOnTextSelection)
            {
                Q_ASSERT (!"kpTransformResizeScaleCommand::unexecute() resizing sel doesn't make sense");
                return;
            }
            else
            {
                QApplication::setOverrideCursor (Qt::WaitCursor);
                doc->selection ()->textResize (m_oldWidth, m_oldHeight);

                Q_ASSERT (m_mainWindow->tool ());
                m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

                QApplication::restoreOverrideCursor ();
            }
        }
        else
        {
            QApplication::setOverrideCursor (Qt::WaitCursor);


            QPixmap newPixmap (m_oldWidth, m_oldHeight);

            kpPixmapFX::setPixmapAt (&newPixmap, QPoint (0, 0),
                                    *doc->pixmap ());

            if (m_newWidth < m_oldWidth)
            {
                kpPixmapFX::setPixmapAt (&newPixmap,
                                        QPoint (m_newWidth, 0),
                                        m_oldRightPixmap);
            }

            if (m_newHeight < m_oldHeight)
            {
                kpPixmapFX::setPixmapAt (&newPixmap,
                                        QPoint (0, m_newHeight),
                                        m_oldBottomPixmap);
            }

            doc->setPixmap (newPixmap);


            QApplication::restoreOverrideCursor ();
        }
    }
    else
    {
        QApplication::setOverrideCursor (Qt::WaitCursor);


        QPixmap oldPixmap;

        if (!m_isLosslessScale)
            oldPixmap = m_oldPixmap;
        else
            oldPixmap = kpPixmapFX::scale (*doc->pixmap (m_actOnSelection),
                                           m_oldWidth, m_oldHeight);


        if (m_actOnSelection)
        {
            kpSelection oldSelection = *m_oldSelection;
            oldSelection.setPixmap (oldPixmap);
            doc->setSelection (oldSelection);

            Q_ASSERT (m_mainWindow->tool ());
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
        }
        else
        {
            doc->setPixmap (oldPixmap);

            if (m_scaleSelectionWithImage)
            {
                doc->setSelection (*m_oldSelection);

                Q_ASSERT (m_mainWindow->tool ());
                m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
            }
        }


        QApplication::restoreOverrideCursor ();
    }
}


#include <kpTransformResizeScaleCommand.moc>
