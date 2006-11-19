
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


#define DEBUG_KP_TOOL_SELECTION 0


#include <kpToolSelectionDestroyCommand.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpToolSelectionDestroyCommand::kpToolSelectionDestroyCommand (const QString &name,
                                                              bool pushOntoDocument,
                                                              kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_pushOntoDocument (pushOntoDocument),
      m_oldSelection (0)
{
}

kpToolSelectionDestroyCommand::~kpToolSelectionDestroyCommand ()
{
    delete m_oldSelection;
}


// public virtual [base kpCommand]
int kpToolSelectionDestroyCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldDocPixmap) +
           kpPixmapFX::selectionSize (m_oldSelection);
}


// public virtual [base kpCommand]
void kpToolSelectionDestroyCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionDestroyCommand::execute () CALLED" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kError () << "kpToolSelectionDestroyCommand::execute() without doc" << endl;
        return;
    }

    if (!doc->selection ())
    {
        kError () << "kpToolSelectionDestroyCommand::execute() without sel region" << endl;
        return;
    }

    m_textRow = m_mainWindow->viewManager ()->textCursorRow ();
    m_textCol = m_mainWindow->viewManager ()->textCursorCol ();

    m_oldSelection = new kpSelection (*doc->selection ());
    if (m_pushOntoDocument)
    {
        m_oldDocPixmap = doc->getPixmapAt (doc->selection ()->boundingRect ());
        doc->selectionPushOntoDocument ();
    }
    else
        doc->selectionDelete ();

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
}

// public virtual [base kpCommand]
void kpToolSelectionDestroyCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionDestroyCommand::unexecute () CALLED" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kError () << "kpToolSelectionDestroyCommand::unexecute() without doc" << endl;
        return;
    }

    if (doc->selection ())
    {
        // not error because it's possible that the user dragged out a new
        // region (without pulling pixmap), and then CTRL+Z
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "kpToolSelectionDestroyCommand::unexecute() already has sel region" << endl;
    #endif

        if (doc->selection ()->pixmap ())
        {
            kError () << "kpToolSelectionDestroyCommand::unexecute() already has sel pixmap" << endl;
            return;
        }
    }

    if (!m_oldSelection)
    {
        kError () << "kpToolSelectionDestroyCommand::unexecute() without old sel" << endl;
        return;
    }

    if (m_pushOntoDocument)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\tunpush oldDocPixmap onto doc first" << endl;
    #endif
        doc->setPixmapAt (m_oldDocPixmap, m_oldSelection->topLeft ());
    }

#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "\tsetting selection to: rect=" << m_oldSelection->boundingRect ()
               << " pixmap=" << m_oldSelection->pixmap ()
               << " pixmap.isNull()=" << (m_oldSelection->pixmap ()
                                              ?
                                          m_oldSelection->pixmap ()->isNull ()
                                              :
                                          true)
               << endl;
#endif
    if (!m_oldSelection->isText ())
    {
        if (m_oldSelection->transparency () != m_mainWindow->selectionTransparency ())
            m_mainWindow->setSelectionTransparency (m_oldSelection->transparency ());
    }
    else
    {
        if (m_oldSelection->textStyle () != m_mainWindow->textStyle ())
            m_mainWindow->setTextStyle (m_oldSelection->textStyle ());
    }

    m_mainWindow->viewManager ()->setTextCursorPosition (m_textRow, m_textCol);
    doc->setSelection (*m_oldSelection);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    delete m_oldSelection;
    m_oldSelection = 0;
}


#include <kpToolSelectionDestroyCommand.moc>
