
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

#ifndef KP_SINGLE_KEY_TRIGGERS_ACTION_H
#define KP_SINGLE_KEY_TRIGGERS_ACTION_H


#include <kshortcut.h>


class kpSingleKeyTriggersActionInterface
{
public:
    kpSingleKeyTriggersActionInterface ();
    virtual ~kpSingleKeyTriggersActionInterface ();

    bool singleKeyTriggersEnabled () const;
    void enableSingleKeyTriggers (bool enable = true);

    // Avoid inheritance diamond by not deriving from KAction
    // so you'll have to implement these by talking to your base KAction.
    virtual const char *actionName () const { return 0; }
    virtual KShortcut actionShortcut () const = 0;
    virtual void actionSetShortcut (const KShortcut &shortcut) = 0;

protected:
    bool m_singleKeyTriggersEnabled;
    KShortcut m_fullShortcut;
};


#include <kaction.h>


class kpSingleKeyTriggersAction : public KAction,
                                  public kpSingleKeyTriggersActionInterface
{
Q_OBJECT

public:
    kpSingleKeyTriggersAction (const QString &text,
                               const KShortcut &shortcut,
                               const QObject *receiver, const char *slot,
                               KActionCollection *parent, const char *name);
    virtual ~kpSingleKeyTriggersAction ();


    //
    // kpSingleKeyTriggersActionInterface
    //

    virtual const char *actionName () const;
    virtual KShortcut actionShortcut () const;
    virtual void actionSetShortcut (const KShortcut &shortcut);
};


#endif  // KP_SINGLE_KEY_TRIGGERS_ACTION_H
