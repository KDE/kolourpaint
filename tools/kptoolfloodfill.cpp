
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptoolfloodfill.h>
#include <kpview.h>
#include <kpviewmanager.h>


/*
 * kpToolFloodFill
 */

kpToolFloodFill::kpToolFloodFill (kpMainWindow *mainWindow)
    : kpTool (i18n ("Flood Fill"), i18n ("Fills regions in the image"),
              mainWindow, "tool_flood_fill"),
      m_currentCommand (0)
{
}

kpToolFloodFill::~kpToolFloodFill ()
{
}

// virtual
void kpToolFloodFill::beginDraw ()
{
    kdDebug () << "kpToolFloodFill::beginDraw()" << endl;

    QApplication::setOverrideCursor (Qt::waitCursor);

    // Flood Fill is an expensive CPU operation so we only fill at a
    // mouse click (beginDraw ()), not on mouse move (virtually draw())
    m_currentCommand = new kpToolFloodFillCommand (document (), viewManager (),
                                                   m_currentPoint.x (), m_currentPoint.y (),
                                                   color (m_mouseButton));

    if (m_currentCommand->prepareColorToChange ())
    {
        kdDebug () << "\tperforming new-doc-corner-case check" << endl;
        if (document ()->url ().isEmpty () && !document ()->isModified ())
        {
            m_currentCommand->setFillEntirePixmap ();
            m_currentCommand->execute ();
        }
        else if (m_currentCommand->prepare ())
        {
            m_currentCommand->execute ();
        }
        else
        {
            kdError () << "kpToolFloodFill::beginDraw() could not fill!" << endl;
        }
    }
    else
    {
        kdError () << "kpToolFloodFill::beginDraw() could not prepareColorToChange!" << endl;
    }

    QApplication::restoreOverrideCursor ();
}

// virtual
void kpToolFloodFill::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    emit mouseMoved (thisPoint);
}

// virtual
void kpToolFloodFill::cancelShape ()
{
#if 0
    endDraw (QPoint (), QRect ());
    mainWindow ()->commandHistory ()->undo ();
#else
    m_currentCommand->unexecute ();

    delete m_currentCommand;
    m_currentCommand = 0;
#endif
}

// virtual
void kpToolFloodFill::endDraw (const QPoint &, const QRect &)
{
    mainWindow ()->commandHistory ()->addCommand (m_currentCommand,
        false /* no exec - we already did it up there */);

    // don't delete
    m_currentCommand = 0;
}


/*
 * kpToolFloodFillCommand
 */

kpToolFloodFillCommand::kpToolFloodFillCommand (kpDocument *document, kpViewManager *viewManager,
                                                int x, int y,
                                                const QColor &color)
    : kpFloodFill (document->pixmap (), x, y, color),
      m_document (document), m_viewManager (viewManager),
      m_fillEntirePixmap (false)
{
}

// virtual
QString kpToolFloodFillCommand::name () const
{
    return i18n ("Flood Fill");
}

kpToolFloodFillCommand::~kpToolFloodFillCommand ()
{
}

void kpToolFloodFillCommand::setFillEntirePixmap (bool yes)
{
    m_fillEntirePixmap = yes;
}

// virtual
void kpToolFloodFillCommand::execute ()
{
    kdDebug () << "kpToolFloodFillCommand::execute() m_fillEntirePixmap=" << m_fillEntirePixmap << endl;

    if (m_fillEntirePixmap)
    {
        m_document->fill (kpFloodFill::color ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            QApplication::setOverrideCursor (QCursor::waitCursor);

            m_oldPixmap = m_document->getPixmapAt (rect);

            kpFloodFill::fill ();
            m_document->slotContentsChanged (rect);

            QApplication::restoreOverrideCursor ();
        }
        else
        {
            kdDebug () << "\tinvalid boundingRect - must be NOP case" << endl;
        }
    }
}

// virtual
void kpToolFloodFillCommand::unexecute ()
{
    if (m_fillEntirePixmap)
    {
        m_document->fill (kpFloodFill::colorToChange ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            m_document->setPixmapAt (m_oldPixmap, rect.topLeft ());

            m_oldPixmap.resize (0, 0);

            m_document->slotContentsChanged (rect);
        }
    }
}

#include <kptoolfloodfill.moc>
