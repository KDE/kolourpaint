
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


#include "kpToolSelectionMoveCommand.h"

#include "layers/selections/kpAbstractSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/manager/kpViewManager.h"

#include "kpLogCategories.h"

//--------------------------------------------------------------------------------

kpToolSelectionMoveCommand::kpToolSelectionMoveCommand (const QString &name,
        kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ)
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);
    Q_ASSERT (doc->selection ());

    m_startPoint = m_endPoint = doc->selection ()->topLeft ();
}

kpToolSelectionMoveCommand::~kpToolSelectionMoveCommand () = default;


// public
kpAbstractSelection *kpToolSelectionMoveCommand::originalSelectionClone () const
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);
    Q_ASSERT (doc->selection ());

    kpAbstractSelection *selection = doc->selection ()->clone ();
    selection->moveTo (m_startPoint);

    return selection;
}


// public virtual [base kpComand]
kpCommandSize::SizeType kpToolSelectionMoveCommand::size () const
{
    return ImageSize (m_oldDocumentImage) +
           PolygonSize (m_copyOntoDocumentPoints);
}


// public virtual [base kpCommand]
void kpToolSelectionMoveCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolSelectionMoveCommand::execute()";
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpAbstractSelection *sel = doc->selection ();
    // Must have content before it can be moved.
    Q_ASSERT (sel && sel->hasContent ());

    kpViewManager *vm = viewManager ();
    Q_ASSERT (vm);

    vm->setQueueUpdates ();
    {
        for (const auto &p : m_copyOntoDocumentPoints)
        {
            sel->moveTo (p);
            doc->selectionCopyOntoDocument ();
        }

        sel->moveTo (m_endPoint);

        environ ()->somethingBelowTheCursorChanged ();
    }
    vm->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpToolSelectionMoveCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "kpToolSelectionMoveCommand::unexecute()";
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpAbstractSelection *sel = doc->selection ();
    // Must have content before it can be un-moved.
    Q_ASSERT (sel && sel->hasContent ());

    kpViewManager *vm = viewManager ();
    Q_ASSERT (vm);

    vm->setQueueUpdates ();

    if (!m_oldDocumentImage.isNull ()) {
        doc->setImageAt (m_oldDocumentImage, m_documentBoundingRect.topLeft ());
    }

#if DEBUG_KP_TOOL_SELECTION && 1
    qCDebug(kpLogCommands) << "\tmove to startPoint=" << m_startPoint;
#endif
    sel->moveTo (m_startPoint);

    environ ()->somethingBelowTheCursorChanged ();

    vm->restoreQueueUpdates ();
}

// public
void kpToolSelectionMoveCommand::moveTo (const QPoint &point, bool moveLater)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    qCDebug(kpLogCommands) << "kpToolSelectionMoveCommand::moveTo" << point
               << " moveLater=" << moveLater;
#endif

    if (!moveLater)
    {
        kpDocument *doc = document ();
        Q_ASSERT (doc);

        kpAbstractSelection *sel = doc->selection ();
        // Must have content before it can be moved.
        Q_ASSERT (sel && sel->hasContent ());

        if (point == sel->topLeft ()) {
            return;
        }

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
    qCDebug(kpLogCommands) << "kpToolSelectionMoveCommand::copyOntoDocument()";
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpAbstractSelection *sel = doc->selection ();
    // Must have content before we allow it be stamped onto the document,
    // to be consistent with the requirement on other selection operations.
    Q_ASSERT (sel && sel->hasContent ());

    if (m_oldDocumentImage.isNull ()) {
        m_oldDocumentImage = doc->image ();
    }

    QRect selBoundingRect = sel->boundingRect ();
    m_documentBoundingRect = m_documentBoundingRect.united (selBoundingRect);

    doc->selectionCopyOntoDocument ();

    m_copyOntoDocumentPoints.putPoints (m_copyOntoDocumentPoints.count (),
                                        1,
                                        selBoundingRect.x (),
                                        selBoundingRect.y ());
}

// public
void kpToolSelectionMoveCommand::finalize ()
{
    if (!m_oldDocumentImage.isNull () && !m_documentBoundingRect.isNull ())
    {
        m_oldDocumentImage = kpTool::neededPixmap (m_oldDocumentImage,
                                                    m_documentBoundingRect);
    }
}

