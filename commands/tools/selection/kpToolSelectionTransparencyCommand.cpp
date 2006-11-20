
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


#include <kpToolSelectionTransparencyCommand.h>

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
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpToolSelectionTransparencyCommand::kpToolSelectionTransparencyCommand (
        const QString &name,
        const kpSelectionTransparency &st,
        const kpSelectionTransparency &oldST,
        kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      m_st (st),
      m_oldST (oldST)
{
}

kpToolSelectionTransparencyCommand::~kpToolSelectionTransparencyCommand ()
{
}


// public virtual [base kpCommand]
int kpToolSelectionTransparencyCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolSelectionTransparencyCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionTransparencyCommand::execute()" << endl;
#endif
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    QApplication::setOverrideCursor (Qt::WaitCursor);
    {
        m_mainWindow->setSelectionTransparency (m_st, true/*force colour change*/);
    
        if (doc->selection ())
            doc->selection ()->setTransparency (m_st);
    }
    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpCommand]
void kpToolSelectionTransparencyCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionTransparencyCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    QApplication::setOverrideCursor (Qt::WaitCursor);
    {
        m_mainWindow->setSelectionTransparency (m_oldST, true/*force colour change*/);
    
        if (doc->selection ())
            doc->selection ()->setTransparency (m_oldST);
    }
    QApplication::restoreOverrideCursor ();
}


#include <kpToolSelectionTransparencyCommand.moc>
