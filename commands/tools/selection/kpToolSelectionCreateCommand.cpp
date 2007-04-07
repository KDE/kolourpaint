
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


#define DEBUG_KP_TOOL_SELECTION 0


#include <kpToolSelectionCreateCommand.h>

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

#include <kpBug.h>
#include <kpCommandHistory.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpMainWindow.h>
#include <kpSelection.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


kpToolSelectionCreateCommand::kpToolSelectionCreateCommand (const QString &name,
                                                            const kpSelection &fromSelection,
                                                            kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_fromSelection (0),
      m_textRow (0), m_textCol (0)
{
    setFromSelection (fromSelection);
}

kpToolSelectionCreateCommand::~kpToolSelectionCreateCommand ()
{
    delete m_fromSelection;
}


// public virtual [base kpCommand]
int kpToolSelectionCreateCommand::size () const
{
    return kpPixmapFX::selectionSize (m_fromSelection);
}


// public static
bool kpToolSelectionCreateCommand::nextUndoCommandIsCreateBorder (
    kpCommandHistory *commandHistory)
{
    if (!commandHistory)
        return false;

    kpCommand *cmd = commandHistory->nextUndoCommand ();
    if (!cmd)
        return false;

    kpToolSelectionCreateCommand *c = dynamic_cast <kpToolSelectionCreateCommand *> (cmd);
    if (!c)
        return false;

    const kpSelection *sel = c->fromSelection ();
    if (!sel)
        return false;

    return (!sel->pixmap ());
}


// public
const kpSelection *kpToolSelectionCreateCommand::fromSelection () const
{
    return m_fromSelection;
}

// public
void kpToolSelectionCreateCommand::setFromSelection (const kpSelection &fromSelection)
{
    delete m_fromSelection;
    m_fromSelection = new kpSelection (fromSelection);
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionCreateCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    if (m_fromSelection)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\tusing fromSelection" << endl;
        kDebug () << "\t\thave sel=" << doc->selection ()
                   << " pixmap=" << (doc->selection () ? doc->selection ()->pixmap () : 0)
                   << endl;
    #endif
        if (!m_fromSelection->isText ())
        {
            if (m_fromSelection->transparency () != m_mainWindow->selectionTransparency ())
                m_mainWindow->setSelectionTransparency (m_fromSelection->transparency ());
        }
        else
        {
            if (m_fromSelection->textStyle () != m_mainWindow->textStyle ())
                m_mainWindow->setTextStyle (m_fromSelection->textStyle ());
        }

        m_mainWindow->viewManager ()->setTextCursorPosition (m_textRow, m_textCol);
        doc->setSelection (*m_fromSelection);

        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    if (!doc->selection ())
    {
        // Was just a border that got deselected?
        if (m_fromSelection && !m_fromSelection->pixmap ())
            return;

        Q_ASSERT (!"kpToolSelectionCreateCommand::unexecute() without sel region");
        return;
    }

    m_textRow = m_mainWindow->viewManager ()->textCursorRow ();
    m_textCol = m_mainWindow->viewManager ()->textCursorCol ();

    doc->selectionDelete ();

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
}


#include <kpToolSelectionCreateCommand.moc>
