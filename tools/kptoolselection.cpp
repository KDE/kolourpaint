
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_SELECTION 1

#include <qbitmap.h>
#include <qcursor.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptoolselection.h>
#include <kpviewmanager.h>


kpToolSelection::kpToolSelection (kpMainWindow *mainWindow)
    : kpTool (QString::null, QString::null, mainWindow, "tool_selection_base_class"),
      m_currentPullFromDocumentCommand (0),
      m_currentMoveCommand (0)
{
    setMode (kpToolSelection::Rectangle);
}

kpToolSelection::~kpToolSelection ()
{
}


// private
void kpToolSelection::pushOntoDocument ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelection::end() CALLED" << endl;
#endif
    mainWindow ()->slotDeselect ();
}


// virtual
void kpToolSelection::begin ()
{
    viewManager ()->setSelectionBorderVisible (true);
    viewManager ()->setSelectionBorderFinished (true);

    m_startDragFromSelectionTopLeft = QPoint ();
    m_dragMoving = false;
    m_haventDraggedYet = true;

    m_currentPullFromDocumentCommand = 0;
    m_currentMoveCommand = 0;
}

// virtual
void kpToolSelection::end ()
{
    if (document ()->selection ())
        pushOntoDocument ();
}


// virtual
void kpToolSelection::beginDraw ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::beginDraw() CALLED" << endl;
#endif

    // Dragging with the RMB would make no sense
    // TODO: RMB click brings up Image popupmenu
    if (m_mouseButton != 0/*left*/)
        return;

    m_dragMoving = false;

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\thas sel region" << endl;
    #endif
        QRect selectionRect = document ()->selection ()->boundingRect ();

        if (document ()->selection ()->contains (m_currentPoint))
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\t\tis move" << endl;
        #endif

            m_startDragFromSelectionTopLeft = m_currentPoint - selectionRect.topLeft ();
            m_dragMoving = true;
            m_haventDraggedYet = true;

            // don't show border while moving (or when we start to move)
            viewManager ()->setSelectionBorderVisible (false);
            viewManager ()->setSelectionBorderFinished (true);
        }
        else
        {
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\t\tis new sel" << endl;
        #endif

            pushOntoDocument ();
        }
    }

    // creating new selection?
    if (!m_dragMoving)
    {
        viewManager ()->setSelectionBorderVisible (true);
        viewManager ()->setSelectionBorderFinished (false);
    }
}

// virtual
void kpToolSelection::hover (const QPoint &point)
{
    // TODO: this will surely overflow the stack
    if (document () && document ()->selection () && document ()->selection ()->contains (point))
        viewManager ()->setCursor (QCursor (Qt::SizeAllCursor));
    else
        viewManager ()->setCursor (QCursor (Qt::CrossCursor));
}

// virtual
void kpToolSelection::draw (const QPoint &thisPoint, const QPoint & /*lastPoint*/,
                            const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::draw() CALLED" << endl;
#endif

    // Dragging with the RMB would make no sense
    // TODO: RMB click brings up Image popupmenu
    if (m_mouseButton != 0/*left*/)
        return;

    if (!m_dragMoving)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tnot moving - resizing rect to" << normalizedRect
                   << endl;
    #endif

        switch (m_mode)
        {
        case kpToolSelection::Rectangle:
            document ()->setSelection (kpSelection (kpSelection::Rectangle, normalizedRect));
            break;
        case kpToolSelection::Ellipse:
            document ()->setSelection (kpSelection (kpSelection::Ellipse, normalizedRect));
            break;
        case kpToolSelection::FreeForm:
            QPointArray points;

            if (document ()->selection ())
                points = document ()->selection ()->points ();

            // TODO: there should be an upper limit on this before drawing the
            //       polygon becomes too slow

            // not detached so will modify "points" directly but
            // still need to call setSelectionBorderPoints() to
            // set selectionBorderType at least once & update screen every time
            points.putPoints (points.count (), 1, thisPoint.x (), thisPoint.y ());

            document ()->setSelection (kpSelection (points));
        #if DEBUG_KP_TOOL_SELECTION
            kdDebug () << "\t\tfreeform; #points=" << document ()->selection ()->points ().count () << endl;
        #endif
            break;
        }

        viewManager ()->setSelectionBorderVisible (true);
    }
    else if (!m_haventDraggedYet || (m_haventDraggedYet && thisPoint != m_startPoint))
    {
    #if DEBUG_KP_TOOL_SELECTION
       kdDebug () << "\tmoving selection" << endl;
    #endif

        if (!document ()->selection ()->pixmap () && !m_currentPullFromDocumentCommand)
        {
            m_currentPullFromDocumentCommand = new kpToolSelectionPullFromDocumentCommand (
                QString::null/*uninteresting child of macro cmd*/,
                mainWindow ());
            m_currentPullFromDocumentCommand->execute ();
        }

        if (!m_currentMoveCommand)
        {
            m_currentMoveCommand = new kpToolSelectionMoveCommand (
                QString::null/*uninteresting child of macro cmd*/,
                mainWindow ());
        }


        //viewManager ()->setQueueUpdates ();
        //viewManager ()->setFastUpdates ();

        if (m_haventDraggedYet && (m_controlPressed || m_shiftPressed))
            m_currentMoveCommand->copyOntoDocument ();

    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\t\ttopLeft=" << normalizedRect.topLeft ()
                   << " startPoint=" << m_startPoint
                   << " startDragFromSel=" << m_startDragFromSelectionTopLeft
                   << endl;

        kdDebug () << "\t\tthisPoint=" << thisPoint
                   << " destPoint=" << thisPoint - m_startDragFromSelectionTopLeft
                   << endl;
    #endif
        m_currentMoveCommand->moveTo (thisPoint - m_startDragFromSelectionTopLeft);

        if (m_shiftPressed)
            m_currentMoveCommand->copyOntoDocument ();

        //viewManager ()->restoreFastUpdates ();
        //viewManager ()->restoreQueueUpdates ();


        m_haventDraggedYet = false;
    }
}

// virtual
void kpToolSelection::cancelShape ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelection::cancelShape() CALLED" << endl;
#endif

    if (m_mouseButton != 0/*left*/)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tstarted draw with right button (which is banned) - abort" << endl;
    #endif
        return;
    }

    if (m_dragMoving)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\twas drag moving - undo drag and undo acquire" << endl;
    #endif

        if (m_currentMoveCommand)
        {
            m_currentMoveCommand->finalize ();
            m_currentMoveCommand->unexecute ();
            delete m_currentMoveCommand;
            m_currentMoveCommand = 0;
        }

        if (m_currentPullFromDocumentCommand)
        {
            m_currentPullFromDocumentCommand->unexecute ();
            delete m_currentPullFromDocumentCommand;
            m_currentPullFromDocumentCommand = 0;
        }
    }
    else
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\twas creating sel - kill" << endl;
    #endif

        // TODO: should we give the user back the selection s/he had before (if any)?
        document ()->selectionDelete ();
    }

    viewManager ()->setSelectionBorderVisible (true);
    viewManager ()->setSelectionBorderFinished (true);
}

// virtual
void kpToolSelection::endDraw (const QPoint & /*thisPoint*/, const QRect & /*normalizedRect*/)
{
    if (m_mouseButton != 0/*left*/)
        return;

    KMacroCommand *cmd = m_currentMoveCommand
                             ?
                         new KMacroCommand (i18n ("Move selection"))
                             :
                         0;

    if (m_currentPullFromDocumentCommand)
    {
        if (!m_currentMoveCommand)
        {
            kdError () << "kpToolSelection::endDraw() pull without move" << endl;
            delete m_currentPullFromDocumentCommand;
            m_currentPullFromDocumentCommand = 0;
        }
        else
        {
            kpSelection selection = m_currentMoveCommand->originalSelection ();

            // just the border
            selection.setPixmap (QPixmap ());

            commandHistory ()->addCommand (new kpToolSelectionCreateCommand (
                i18n ("Create selection"),
                selection,
                mainWindow ()),
                false/*no exec - user already dragged out sel*/);


            cmd->addCommand (m_currentPullFromDocumentCommand);
            m_currentPullFromDocumentCommand = 0;
        }
    }

    if (m_currentMoveCommand)
    {
        m_currentMoveCommand->finalize ();
        cmd->addCommand (m_currentMoveCommand);
        m_currentMoveCommand = 0;
    }

    if (cmd)
        commandHistory ()->addCommand (cmd, false/*no exec*/);

    viewManager ()->setSelectionBorderVisible (true);
    viewManager ()->setSelectionBorderFinished (true);
}



/*
 * kpToolSelectionCreateCommand
 */

kpToolSelectionCreateCommand::kpToolSelectionCreateCommand (const QString &name,
                                                            const kpSelection &fromSelection,
                                                            kpMainWindow *mainWindow)
    : m_name (name),
      m_fromSelection (new kpSelection (fromSelection)),
      m_mainWindow (mainWindow)
{
}

// public virtual [base KCommand]
QString kpToolSelectionCreateCommand::name () const
{
    return m_name;
}

kpToolSelectionCreateCommand::~kpToolSelectionCreateCommand ()
{
    delete m_fromSelection;
}


// private
kpDocument *kpToolSelectionCreateCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public virtual [base KCommand]
void kpToolSelectionCreateCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionCreateCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionCreateCommand::execute() without doc" << endl;
        return;
    }

    if (m_fromSelection)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tusing fromSelection" << endl;
        kdDebug () << "\t\thave sel=" << doc->selection ()
                   << " pixmap=" << (doc->selection () ? doc->selection ()->pixmap () : 0)
                   << endl;
    #endif
        doc->setSelection (*m_fromSelection);
    }
}

// public virtual [base KCommand]
void kpToolSelectionCreateCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionCreateCommand::unexecute() without doc" << endl;
        return;
    }

    if (!doc->selection ())
    {
        kdError () << "kpToolSelectionCreateCommand::unexecute() without sel region" << endl;
        return;
    }

    doc->selectionDelete ();
}


/*
 * kpToolSelectionPullFromDocumentCommand
 */

kpToolSelectionPullFromDocumentCommand::kpToolSelectionPullFromDocumentCommand (const QString &name,
                                                                                kpMainWindow *mainWindow)
    : m_name (name),
      m_mainWindow (mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : Qt::white),
      m_originalSelectionRegion (0)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionPullFromDocumentCommand::<ctor>() mainWindow="
               << m_mainWindow
               << endl;
#endif
}

// public virtual [base KCommand]
QString kpToolSelectionPullFromDocumentCommand::name () const
{
    return m_name;
}

kpToolSelectionPullFromDocumentCommand::~kpToolSelectionPullFromDocumentCommand ()
{
    delete m_originalSelectionRegion;
}


// private
kpDocument *kpToolSelectionPullFromDocumentCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public virtual [base KCommand]
void kpToolSelectionPullFromDocumentCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionPullFromDocumentCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();

    if (!doc)
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::execute() without doc" << endl;
        return;
    }

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;
    if (vm)
        vm->setQueueUpdates ();

    // must have selection region but not pixmap
    if (!doc->selection () || doc->selection ()->pixmap ())
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::execute() sel="
                   << doc->selection ()
                   << " pixmap="
                   << (doc->selection () ? doc->selection ()->pixmap () : 0)
                   << endl;
        if (vm)
            vm->restoreQueueUpdates ();
        return;
    }

    // In case the user CTRL+Z'ed, selected a random region to throw us off
    // and then CTRL+Shift+Z'ed putting us here.  Make sure we pull from the
    // originally requested region - not the random one.
    if (m_originalSelectionRegion)
        doc->setSelection (*m_originalSelectionRegion);

    doc->selectionPullFromDocument (m_backgroundColor);

    if (vm)
        vm->restoreQueueUpdates ();
}

// public virtual [base KCommand]
void kpToolSelectionPullFromDocumentCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kdDebug () << "kpToolSelectionPullFromDocumentCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();

    if (!doc)
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::unexecute() without doc" << endl;
        return;
    }

    // must have selection pixmap
    if (!doc->selection () || !doc->selection ()->pixmap ())
    {
        kdError () << "kpToolSelectionPullFromDocumentCommand::unexecute() sel="
                   << doc->selection ()
                   << " pixmap="
                   << (doc->selection () ? doc->selection ()->pixmap () : 0)
                   << endl;
        return;
    }


    // We can have faith that this is the state of the selection after
    // execute(), rather than after the user tried to throw us off by
    // simply selecting another region as to do that, a destroy command
    // must have been used.
    doc->selectionCopyOntoDocument ();
    doc->selection ()->setPixmap (QPixmap ());

    delete m_originalSelectionRegion;
    m_originalSelectionRegion = new kpSelection (*doc->selection ());
}


/*
 * kpToolSelectionMoveCommand
 */

kpToolSelectionMoveCommand::kpToolSelectionMoveCommand (const QString &name,
                                                        kpMainWindow *mainWindow)
    : m_name (name),
      m_mainWindow (mainWindow)
{
    kpDocument *doc = document ();
    if (doc && doc->selection ())
    {
        m_startPoint = m_endPoint = doc->selection ()->topLeft ();
    }
}

// public virtual [base KCommand]
QString kpToolSelectionMoveCommand::name () const
{
    return m_name;
}

kpToolSelectionMoveCommand::~kpToolSelectionMoveCommand ()
{
}


// private
kpDocument *kpToolSelectionMoveCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public
kpSelection kpToolSelectionMoveCommand::originalSelection () const
{
    kpDocument *doc = document ();
    if (!doc || !doc->selection ())
    {
        kdError () << "kpToolSelectionMoveCommand::originalSelection() doc="
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


// public virtual [base KCommand]
void kpToolSelectionMoveCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "kpToolSelectionMoveCommand::execute() no doc" << endl;
    #endif
        return;
    }

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel || !sel->pixmap ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "kpToolSelectionMoveCommand::execute() but haven't pulled pixmap yet: "
                   << "sel=" << sel << " sel->pixmap=" << sel->pixmap ()
                   << endl;
    #endif
        return;
    }

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    if (vm)
        vm->setQueueUpdates ();

    QPointArray::ConstIterator copyOntoDocumentPointsEnd = m_copyOntoDocumentPoints.end ();
    for (QPointArray::ConstIterator it = m_copyOntoDocumentPoints.begin ();
         it != copyOntoDocumentPointsEnd;
         it++)
    {
        sel->moveTo (*it);
        doc->selectionCopyOntoDocument ();
    }

    sel->moveTo (m_endPoint);

    if (vm)
        vm->restoreQueueUpdates ();
}

// public virtual [base KCommand]
void kpToolSelectionMoveCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "kpToolSelectionMoveCommand::unexecute() no doc" << endl;
    #endif
        return;
    }

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel || !sel->pixmap ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "kpToolSelectionMoveCommand::unexecute() but haven't pulled pixmap yet: "
                   << "sel=" << sel << " sel->pixmap=" << sel->pixmap ()
                   << endl;
    #endif
        return;
    }

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    if (vm)
        vm->setQueueUpdates ();

    if (!m_oldDocumentPixmap.isNull ())
        doc->setPixmapAt (m_oldDocumentPixmap, m_documentBoundingRect.topLeft ());
    sel->moveTo (m_startPoint);

    if (vm)
        vm->restoreQueueUpdates ();
}

// public
void kpToolSelectionMoveCommand::moveTo (const QPoint &point, bool moveLater)
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionMoveCommand::moveTo" << point
               << " moveLater=" << moveLater
               <<endl;
#endif

    if (!moveLater)
    {
        kpDocument *doc = document ();
        if (!doc)
        {
            kdError () << "kpToolSelectionMoveCommand::moveTo() without doc" << endl;
            return;
        }

        kpSelection *sel = doc->selection ();

        // have to have pulled pixmap by now
        if (!sel)
        {
            kdError () << "kpToolSelectionMoveCommand::moveTo() no sel region" << endl;
            return;
        }

        if (!sel->pixmap ())
        {
            kdError () << "kpToolSelectionMoveCommand::moveTo() no sel pixmap" << endl;
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
    kdDebug () << "kpToolSelectionMoveCommand::copyOntoDocument()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    kpSelection *sel = doc->selection ();

    // have to have pulled pixmap by now
    if (!sel)
    {
        kdError () << "\tkpToolSelectionMoveCommand::copyOntoDocument() without sel region" << endl;
        return;
    }

    if (!sel->pixmap ())
    {
        kdError () << "kpToolSelectionMoveCommand::moveTo() no sel pixmap" << endl;
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


/*
 * kpToolSelectionDestroyCommand
 */

kpToolSelectionDestroyCommand::kpToolSelectionDestroyCommand (const QString &name,
                                                              bool pushOntoDocument,
                                                              kpMainWindow *mainWindow)
    : m_name (name),
      m_pushOntoDocument (pushOntoDocument),
      m_oldSelection (0),
      m_mainWindow (mainWindow)
{
}

// public virtual [base KCommand]
QString kpToolSelectionDestroyCommand::name () const
{
    return m_name;
}

kpToolSelectionDestroyCommand::~kpToolSelectionDestroyCommand ()
{
    delete m_oldSelection;
}


// private
kpDocument *kpToolSelectionDestroyCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public virtual [base KCommand]
void kpToolSelectionDestroyCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionDestroyCommand::execute () CALLED" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionDestroyCommand::execute() without doc" << endl;
        return;
    }

    if (!doc->selection ())
    {
        kdError () << "kpToolSelectionDestroyCommand::execute() without sel region" << endl;
        return;
    }

    m_oldSelection = new kpSelection (*doc->selection ());
    if (m_pushOntoDocument)
    {
        m_oldDocPixmap = doc->getPixmapAt (doc->selection ()->boundingRect ());
        doc->selectionPushOntoDocument ();
    }
    else
        doc->selectionDelete ();
}

// public virtual [base KCommand]
void kpToolSelectionDestroyCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "kpToolSelectionDestroyCommand::unexecute () CALLED" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolSelectionDestroyCommand::unexecute() without doc" << endl;
        return;
    }

    if (doc->selection ())
    {
        // not error because it's possible that the user dragged out a new
        // region (without pulling pixmap), and then CTRL+Z
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "kpToolSelectionDestroyCommand::unexecute() already has sel region" << endl;
    #endif

        if (doc->selection ()->pixmap ())
        {
            kdError () << "kpToolSelectionDestroyCommand::unexecute() already has sel pixmap" << endl;
            return;
        }
    }

    if (!m_oldSelection)
    {
        kdError () << "kpToolSelectionDestroyCommand::unexecute() without old sel" << endl;
        return;
    }

    if (m_pushOntoDocument)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kdDebug () << "\tunpush oldDocPixmap onto doc first" << endl;
    #endif
        doc->setPixmapAt (m_oldDocPixmap, m_oldSelection->topLeft ());
    }

#if DEBUG_KP_TOOL_SELECTION
    kdDebug () << "\tsetting selection to: rect=" << m_oldSelection->boundingRect ()
               << " pixmap=" << m_oldSelection->pixmap ()
               << " pixmap.isNull()=" << (m_oldSelection->pixmap ()
                                              ?
                                          m_oldSelection->pixmap ()->isNull ()
                                              :
                                          true)
               << endl;
#endif
    doc->setSelection (*m_oldSelection);

    delete m_oldSelection;
    m_oldSelection = 0;
}

#include <kptoolselection.moc>
