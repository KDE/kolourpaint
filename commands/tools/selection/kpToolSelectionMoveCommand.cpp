
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


#include <kpToolSelectionMoveCommand.h>

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


kpToolSelectionMoveCommand::kpToolSelectionMoveCommand (const QString &name,
                                                        kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow)
{
    kpDocument *doc = document ();
    if (doc && doc->selection ())
    {
        m_startPoint = m_endPoint = doc->selection ()->topLeft ();
    }
}

kpToolSelectionMoveCommand::~kpToolSelectionMoveCommand ()
{
}


// public
kpSelection kpToolSelectionMoveCommand::originalSelection () const
{
    kpDocument *doc = document ();
    if (!doc || !doc->selection ())
    {
        kError () << "kpToolSelectionMoveCommand::originalSelection() doc="
                   << doc
                   << " sel="
                   << (doc ? doc->selection () : 0)
                   << endl;
        return kpSelection (kpSelection::Rectangle, QRect ());
    }

    kpSelection selection = *doc->selection();
    selection.moveTo (m_startPoint);

    return selection;
}


// public virtual [base kpComand]
int kpToolSelectionMoveCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldDocumentPixmap) +
           kpPixmapFX::pointArraySize (m_copyOntoDocumentPoints);
}


// public virtual [base kpCommand]
void kpToolSelectionMoveCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionMoveCommand::execute()" << endl;
#endif

    Q_ASSERT (m_mainWindow);

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel || !sel->pixmap ())
    {
        kError () << "kpToolSelectionMoveCommand::execute() but haven't pulled pixmap yet: "
                   << "sel=" << sel << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
                   << endl;
        return;
    }

    kpViewManager *vm = m_mainWindow->viewManager ();
    Q_ASSERT (vm);

    vm->setQueueUpdates ();

    QPolygon::ConstIterator copyOntoDocumentPointsEnd = m_copyOntoDocumentPoints.end ();
    for (QPolygon::ConstIterator it = m_copyOntoDocumentPoints.begin ();
         it != copyOntoDocumentPointsEnd;
         it++)
    {
        sel->moveTo (*it);
        doc->selectionCopyOntoDocument ();
    }

    sel->moveTo (m_endPoint);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    vm->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpToolSelectionMoveCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionMoveCommand::unexecute()" << endl;
#endif

    Q_ASSERT (m_mainWindow);

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel || !sel->pixmap ())
    {
        kError () << "kpToolSelectionMoveCommand::unexecute() but haven't pulled pixmap yet: "
                   << "sel=" << sel << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
                   << endl;
        return;
    }

    kpViewManager *vm = m_mainWindow->viewManager ();
    Q_ASSERT (vm);

    vm->setQueueUpdates ();

    if (!m_oldDocumentPixmap.isNull ())
        doc->setPixmapAt (m_oldDocumentPixmap, m_documentBoundingRect.topLeft ());
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "\tmove to startPoint=" << m_startPoint << endl;
#endif
    sel->moveTo (m_startPoint);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    vm->restoreQueueUpdates ();
}

// public
void kpToolSelectionMoveCommand::moveTo (const QPoint &point, bool moveLater)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    kDebug () << "kpToolSelectionMoveCommand::moveTo" << point
               << " moveLater=" << moveLater
               <<endl;
#endif

    if (!moveLater)
    {
        kpDocument *doc = document ();
        if (!doc)
        {
            kError () << "kpToolSelectionMoveCommand::moveTo() without doc" << endl;
            return;
        }

        kpSelection *sel = doc->selection ();

        // have to have pulled pixmap by now
        if (!sel)
        {
            kError () << "kpToolSelectionMoveCommand::moveTo() no sel region" << endl;
            return;
        }

        if (!sel->pixmap ())
        {
            kError () << "kpToolSelectionMoveCommand::moveTo() no sel pixmap" << endl;
            return;
        }

        if (point == sel->topLeft ())
            return;

        sel->moveTo (point);
    }

    m_endPoint = point;
}

// public
void kpToolSelectionMoveCommand::moveTo (int x, int y, bool moveLater)
{
    moveTo (QPoint (x, y), moveLater);
}

// public
void kpToolSelectionMoveCommand::copyOntoDocument ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionMoveCommand::copyOntoDocument()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel)
    {
        kError () << "\tkpToolSelectionMoveCommand::copyOntoDocument() without sel region" << endl;
        return;
    }

    if (!sel->pixmap ())
    {
        kError () << "kpToolSelectionMoveCommand::moveTo() no sel pixmap" << endl;
        return;
    }

    if (m_oldDocumentPixmap.isNull ())
        m_oldDocumentPixmap = *doc->pixmap ();

    QRect selBoundingRect = sel->boundingRect ();
    m_documentBoundingRect.unite (selBoundingRect);

    doc->selectionCopyOntoDocument ();

    m_copyOntoDocumentPoints.putPoints (m_copyOntoDocumentPoints.count (),
                                        1,
                                        selBoundingRect.x (),
                                        selBoundingRect.y ());
}

// public
void kpToolSelectionMoveCommand::finalize ()
{
    if (!m_oldDocumentPixmap.isNull () && !m_documentBoundingRect.isNull ())
    {
        m_oldDocumentPixmap = kpTool::neededPixmap (m_oldDocumentPixmap,
                                                    m_documentBoundingRect);
    }
}


#include <kpToolSelectionMoveCommand.moc>
