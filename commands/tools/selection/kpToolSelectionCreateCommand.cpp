
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


#include "commands/tools/selection/kpToolSelectionCreateCommand.h"

#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

#include "kpLogCategories.h"

#include <QApplication>
#include <QBitmap>
#include <QCursor>
#include <QEvent>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QTimer>


kpToolSelectionCreateCommand::kpToolSelectionCreateCommand (const QString &name,
        const kpAbstractSelection &fromSelection,
        kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      m_fromSelection (nullptr),
      m_textRow (0), m_textCol (0)
{
    setFromSelection (fromSelection);
}

kpToolSelectionCreateCommand::~kpToolSelectionCreateCommand ()
{
    delete m_fromSelection;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolSelectionCreateCommand::size () const
{
    return SelectionSize (m_fromSelection);
}


// public
const kpAbstractSelection *kpToolSelectionCreateCommand::fromSelection () const
{
    return m_fromSelection;
}

// public
void kpToolSelectionCreateCommand::setFromSelection (const kpAbstractSelection &fromSelection)
{
    delete m_fromSelection;
    m_fromSelection = fromSelection.clone ();
}

// public virtual [base kpCommand]
void kpToolSelectionCreateCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "kpToolSelectionCreateCommand::execute()";
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    if (m_fromSelection)
    {
    #if DEBUG_KP_TOOL_SELECTION
        qCDebug(kpLogCommands) << "\tusing fromSelection";
        qCDebug(kpLogCommands) << "\t\thave sel=" << doc->selection () << endl;
    #endif
        kpAbstractImageSelection *imageSel =
            dynamic_cast <kpAbstractImageSelection *> (m_fromSelection);
        kpTextSelection *textSel =
            dynamic_cast <kpTextSelection *> (m_fromSelection);
        if (imageSel)
        {
            if (imageSel->transparency () != environ ()->imageSelectionTransparency ()) {
                environ ()->setImageSelectionTransparency (imageSel->transparency ());
            }
        }
        else if (textSel)
        {
            if (textSel->textStyle () != environ ()->textStyle ()) {
                environ ()->setTextStyle (textSel->textStyle ());
            }
        }
        else {
            Q_ASSERT (!"Unknown selection type");
        }

        viewManager ()->setTextCursorPosition (m_textRow, m_textCol);
        doc->setSelection (*m_fromSelection);

        environ ()->somethingBelowTheCursorChanged ();
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
        if (m_fromSelection && !m_fromSelection->hasContent ()) {
            return;
        }

        Q_ASSERT (!"kpToolSelectionCreateCommand::unexecute() without sel region");
        return;
    }

    m_textRow = viewManager ()->textCursorRow ();
    m_textCol = viewManager ()->textCursorCol ();

    doc->selectionDelete ();

    environ ()->somethingBelowTheCursorChanged ();
}

