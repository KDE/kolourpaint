
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


#include "environments/kpEnvironmentBase.h"

#include "widgets/toolbars/kpColorToolBar.h"
#include "document/kpDocument.h"
#include "mainWindow/kpMainWindow.h"
#include "layers/selections/text/kpTextStyle.h"
#include "tools/kpTool.h"


struct kpEnvironmentBasePrivate
{
    kpMainWindow *mainWindow;
};

kpEnvironmentBase::kpEnvironmentBase (kpMainWindow *mainWindow)
    : QObject (mainWindow),
      d (new kpEnvironmentBasePrivate ())
{
    Q_ASSERT (mainWindow);

    d->mainWindow = mainWindow;
}

kpEnvironmentBase::~kpEnvironmentBase ()
{
    delete d;
}


// public
kpDocument *kpEnvironmentBase::document () const
{
    return d->mainWindow->document ();
}


// public
kpAbstractSelection *kpEnvironmentBase::selection () const
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    return doc->selection ();
}

// public
kpAbstractImageSelection *kpEnvironmentBase::imageSelection () const
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    return doc->imageSelection ();
}

// public
kpTextSelection *kpEnvironmentBase::textSelection () const
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    return doc->textSelection ();
}


// public
kpViewManager *kpEnvironmentBase::viewManager () const
{
    return mainWindow ()->viewManager ();
}


// public
kpCommandEnvironment *kpEnvironmentBase::commandEnvironment () const
{
    return mainWindow ()->commandEnvironment ();
}


// public
kpColor kpEnvironmentBase::backgroundColor (bool ofSelection) const
{
    return d->mainWindow->backgroundColor (ofSelection);
}


// protected
kpMainWindow *kpEnvironmentBase::mainWindow () const
{
    return d->mainWindow;
}


