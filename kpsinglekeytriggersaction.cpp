
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

#define DEBUG_KP_SINGLE_KEY_TRIGGERS_ACTION 0


//
// kpSingleKeyTriggersActionInterface
//


#include <kpsinglekeytriggersaction.h>

#include <kdebug.h>

#include <kptool.h>


kpSingleKeyTriggersActionInterface::kpSingleKeyTriggersActionInterface ()
{
    m_singleKeyTriggersEnabled = true;
}

kpSingleKeyTriggersActionInterface::~kpSingleKeyTriggersActionInterface ()
{
}


// public
bool kpSingleKeyTriggersActionInterface::singleKeyTriggersEnabled () const
{
    return m_singleKeyTriggersEnabled;
}

// public
void kpSingleKeyTriggersActionInterface::enableSingleKeyTriggers (bool enable)
{
#if DEBUG_KP_SINGLE_KEY_TRIGGERS_ACTION
    kdDebug () << "kpSingleKeyTriggersActionInterface(" << /*virtual*/actionName ()
               << ")::enableSingleKeyTriggers(" << enable << ")"
               << endl;
#endif

    if (enable == m_singleKeyTriggersEnabled)
    {
    #if DEBUG_KP_SINGLE_KEY_TRIGGERS_ACTION
        kdDebug () << "\tnop" << endl;
    #endif
        return;
    }

    m_singleKeyTriggersEnabled = enable;


    if (enable)
    {
    #if DEBUG_KP_SINGLE_KEY_TRIGGERS_ACTION
        kdDebug () << "\tre-enabling to " << m_fullShortcut.toString () << endl;
    #endif
        /*pure virtual*/actionSetShortcut (m_fullShortcut);
    }
    else  // disable single key triggers
    {
        m_fullShortcut = /*pure virtual*/actionShortcut ();

        KShortcut newShortcut;
        if (kpTool::containsSingleKeyTrigger (m_fullShortcut, &newShortcut))
        {
        #if DEBUG_KP_SINGLE_KEY_TRIGGERS_ACTION
            kdDebug () << "\tchange shortcuts: old="
                       << m_fullShortcut.toString ()
                       << " new="
                       << newShortcut.toString ()
                       << endl;
        #endif
            /*pure virtual*/actionSetShortcut (newShortcut);
        }
        else
        {
        #if DEBUG_KP_SINGLE_KEY_TRIGGERS_ACTION
            kdDebug () << "\tshortcut is untouched "
                       << m_fullShortcut.toString ()
                       << endl;
        #endif
        }
    }
}


//
// kpSingleKeyTriggersAction
//


kpSingleKeyTriggersAction::kpSingleKeyTriggersAction (const QString &text,
                            const KShortcut &shortcut,
                            const QObject *receiver, const char *slot,
                            KActionCollection *parent, const char *name)
    : KAction (text, shortcut, receiver, slot, parent, name)
{
}

kpSingleKeyTriggersAction::~kpSingleKeyTriggersAction ()
{
}


//
// kpSingleKeyTriggersAction implements kpSingleKeyTriggersActionInterface
//

// public virtual [base kpSingleKeyTriggersActionInterface]
const char *kpSingleKeyTriggersAction::actionName () const
{
    return name ();
}

// public virtual [base kpSingleKeyTriggersActionInterface]
KShortcut kpSingleKeyTriggersAction::actionShortcut () const
{
    return shortcut ();
}

// public virtual [base kpSingleKeyTriggersActionInterface]
void kpSingleKeyTriggersAction::actionSetShortcut (const KShortcut &shortcut)
{
    setShortcut (shortcut);
}


#include <kpsinglekeytriggersaction.moc>
