
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


#include <kpToolSelectionPullFromDocumentCommand.h>

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

#include <kpAbstractImageSelection.h>
#include <kpCommandEnvironment.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


kpToolSelectionPullFromDocumentCommand::kpToolSelectionPullFromDocumentCommand (
        const QString &name,
        kpCommandEnvironment *environ)
    : kpNamedCommand (name, environ),
      m_backgroundColor (environ->backgroundColor ()),
      m_originalSelectionRegion (0)
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionPullFromDocumentCommand::<ctor>() environ="
               << environ
               << endl;
#endif
}

kpToolSelectionPullFromDocumentCommand::~kpToolSelectionPullFromDocumentCommand ()
{
    delete m_originalSelectionRegion;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolSelectionPullFromDocumentCommand::size () const
{
    return SelectionSize (m_originalSelectionRegion);
}


// public virtual [base kpCommand]
void kpToolSelectionPullFromDocumentCommand::execute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionPullFromDocumentCommand::execute()" << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpViewManager *vm = viewManager ();
    Q_ASSERT (vm);

    vm->setQueueUpdates ();

    // In case the user CTRL+Z'ed, selected a random region to throw us off
    // and then CTRL+Shift+Z'ed putting us here.  Make sure we pull from the
    // originally requested region - not the random one.
    if (m_originalSelectionRegion)
    {
        if (m_originalSelectionRegion->transparency () != environ ()->imageSelectionTransparency ())
            environ ()->setImageSelectionTransparency (m_originalSelectionRegion->transparency ());

        doc->setSelection (*m_originalSelectionRegion);
    }
    else
    {
        // Must have selection region but not image content.
        Q_ASSERT (imageSelection () && !imageSelection ()->hasContent ());
    }

    doc->imageSelectionPullFromDocument (m_backgroundColor);

    vm->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpToolSelectionPullFromDocumentCommand::unexecute ()
{
#if DEBUG_KP_TOOL_SELECTION && 1
    kDebug () << "kpToolSelectionPullFromDocumentCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);
    // Must have selection image content.
    Q_ASSERT (doc->imageSelection () && doc->imageSelection ()->hasContent ());


    // We can have faith that this is the state of the selection after
    // execute(), rather than after the user tried to throw us off by
    // simply selecting another region as to do that, a destroy command
    // must have been used.
    doc->selectionCopyOntoDocument (false/*use opaque pixmap*/);
    doc->imageSelection ()->setBaseImage (kpImage ());

    delete m_originalSelectionRegion;
    m_originalSelectionRegion = imageSelection ()->clone ();
}


#include <kpToolSelectionPullFromDocumentCommand.moc>
