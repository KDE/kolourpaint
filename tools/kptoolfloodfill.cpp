
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_FLOOD_FILL 0


#include <kptoolfloodfill.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpview.h>
#include <kpviewmanager.h>


/*
 * kpToolFloodFill
 */

kpToolFloodFill::kpToolFloodFill (kpMainWindow *mainWindow)
    : kpTool (i18n ("Flood Fill"), i18n ("Fills regions in the image"),
              Qt::Key_F,
              mainWindow, "tool_flood_fill"),
      m_currentCommand (0)
{
}

kpToolFloodFill::~kpToolFloodFill ()
{
}

QString kpToolFloodFill::haventBegunDrawUserMessage () const
{
    return i18n ("Click to fill a region.");
}

void kpToolFloodFill::begin ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolFloodFill::beginDraw ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    kdDebug () << "kpToolFloodFill::beginDraw()" << endl;
#endif

    QApplication::setOverrideCursor (Qt::waitCursor);

    // Flood Fill is an expensive CPU operation so we only fill at a
    // mouse click (beginDraw ()), not on mouse move (virtually draw())
    m_currentCommand = new kpToolFloodFillCommand (m_currentPoint.x (), m_currentPoint.y (),
                                                   color (m_mouseButton), processedColorSimilarity (),
                                                   mainWindow ());

    if (m_currentCommand->prepareColorToChange ())
    {
    #if DEBUG_KP_TOOL_FLOOD_FILL && 1
        kdDebug () << "\tperforming new-doc-corner-case check" << endl;
    #endif
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

    setUserMessage (cancelUserMessage ());
}

// virtual
void kpToolFloodFill::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    setUserShapePoints (thisPoint);
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

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolFloodFill::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolFloodFill::endDraw (const QPoint &, const QRect &)
{
    mainWindow ()->commandHistory ()->addCommand (m_currentCommand,
        false /* no exec - we already did it up there */);

    // don't delete
    m_currentCommand = 0;
    setUserMessage (haventBegunDrawUserMessage ());
}


/*
 * kpToolFloodFillCommand
 */

kpToolFloodFillCommand::kpToolFloodFillCommand (int x, int y,
                                                const kpColor &color, int processedColorSimilarity,
                                                kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      kpFloodFill (document ()->pixmap (), x, y, color, processedColorSimilarity),
      m_fillEntirePixmap (false)
{
}

kpToolFloodFillCommand::~kpToolFloodFillCommand ()
{
}


// public virtual [base kpCommand]
QString kpToolFloodFillCommand::name () const
{
    return i18n ("Flood Fill");
}

// public virtual [base kpCommand]
int kpToolFloodFillCommand::size () const
{
    return kpFloodFill::size () + kpPixmapFX::pixmapSize (m_oldPixmap);
}


void kpToolFloodFillCommand::setFillEntirePixmap (bool yes)
{
    m_fillEntirePixmap = yes;
}


// virtual
void kpToolFloodFillCommand::execute ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    kdDebug () << "kpToolFloodFillCommand::execute() m_fillEntirePixmap=" << m_fillEntirePixmap << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;


    if (m_fillEntirePixmap)
    {
        doc->fill (kpFloodFill::color ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            QApplication::setOverrideCursor (QCursor::waitCursor);

            m_oldPixmap = doc->getPixmapAt (rect);

            kpFloodFill::fill ();
            doc->slotContentsChanged (rect);

            QApplication::restoreOverrideCursor ();
        }
        else
        {
        #if DEBUG_KP_TOOL_FLOOD_FILL && 1
            kdDebug () << "\tinvalid boundingRect - must be NOP case" << endl;
        #endif
        }
    }
}

// virtual
void kpToolFloodFillCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;


    if (m_fillEntirePixmap)
    {
        doc->fill (kpFloodFill::colorToChange ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            doc->setPixmapAt (m_oldPixmap, rect.topLeft ());

            m_oldPixmap.resize (0, 0);

            doc->slotContentsChanged (rect);
        }
    }
}

#include <kptoolfloodfill.moc>
