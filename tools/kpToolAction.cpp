
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

#define DEBUG_KP_TOOL_ACTION 0


#include <kpToolAction.h>

#include <kactioncollection.h>
#include <kdebug.h>
#include <kicon.h>

#include <kpTool.h>


kpToolAction::kpToolAction (const QString &text,
                            const QString &pic, const KShortcut &shortcut,
                            const QObject *receiver, const char *slot,
                            KActionCollection *ac, const QString &name)
    : KToggleAction (KIcon (pic), text, ac)
{
#if DEBUG_KP_TOOL_ACTION
    kDebug () << "kpToolAction<" << name << ">::kpToolAction(shortcut="
              << shortcut.toString () << ")" << endl;
#endif

    KToggleAction::setShortcut (shortcut);

    if (receiver && slot)
        connect (this, SIGNAL (triggered (bool)), receiver, slot);

    updateToolTip ();

    ac->addAction (name, this);
}

kpToolAction::~kpToolAction ()
{
}


// protected
void kpToolAction::updateToolTip ()
{
    const QString newToolTip =
        kpTool::toolTipForTextAndShortcut (text (), shortcut ());
#if DEBUG_KP_TOOL_ACTION
    kDebug () << "\tkpToolAction<" << objectName () << ">::updateToolTip()"
              << " text='" << text () << "' shortcut=" << shortcut ().toString ()
              << " oldToolTip=" << toolTip () << " newToolTip=" << newToolTip
              << endl;
#endif
    if (newToolTip == toolTip ())
        return;

    setToolTip (newToolTip);
    emit toolTipChanged (newToolTip);
}


// public static
void kpToolAction::updateAllActionsToolTips (KActionCollection *ac)
{
#if DEBUG_KP_TOOL_ACTION
    kDebug () << "kpToolAction::updateAllActionsToolTips()";
#endif

    foreach (QAction *action, ac->actions ())
    {
        kpToolAction *toolAction = qobject_cast <kpToolAction *> (action);
        if (!toolAction)
            continue;

        toolAction->updateToolTip ();
    }
}


//
// KToggleAction overrides
//

// public
void kpToolAction::setText (const QString &text)
{
#if DEBUG_KP_TOOL_ACTION
    kDebug () << "kpToolAction<" << objectName ()
               << ">::setText(" << text << ")" << endl;
#endif

    KToggleAction::setText (text);
    updateToolTip ();
}

// public
void kpToolAction::setShortcut (const KShortcut &shortcut)
{
#if DEBUG_KP_TOOL_ACTION
    kDebug () << "kpToolAction<" << objectName ()
               << ">::setShortcut(" << shortcut.toString () << ")" << endl;
#endif

    KToggleAction::setShortcut (shortcut);
    updateToolTip ();
}


#include <kpToolAction.moc>
