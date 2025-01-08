
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "environments/kpEnvironmentBase.h"

#include "document/kpDocument.h"
#include "layers/selections/text/kpTextStyle.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/kpColorToolBar.h"

struct kpEnvironmentBasePrivate {
    kpMainWindow *mainWindow;
};

kpEnvironmentBase::kpEnvironmentBase(kpMainWindow *mainWindow)
    : QObject(mainWindow)
    , d(new kpEnvironmentBasePrivate())
{
    Q_ASSERT(mainWindow);

    d->mainWindow = mainWindow;
}

kpEnvironmentBase::~kpEnvironmentBase()
{
    delete d;
}

// public
kpDocument *kpEnvironmentBase::document() const
{
    return d->mainWindow->document();
}

// public
kpAbstractSelection *kpEnvironmentBase::selection() const
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    return doc->selection();
}

// public
kpAbstractImageSelection *kpEnvironmentBase::imageSelection() const
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    return doc->imageSelection();
}

// public
kpTextSelection *kpEnvironmentBase::textSelection() const
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    return doc->textSelection();
}

// public
kpViewManager *kpEnvironmentBase::viewManager() const
{
    return mainWindow()->viewManager();
}

// public
kpCommandEnvironment *kpEnvironmentBase::commandEnvironment() const
{
    return mainWindow()->commandEnvironment();
}

// public
kpColor kpEnvironmentBase::backgroundColor(bool ofSelection) const
{
    return d->mainWindow->backgroundColor(ofSelection);
}

// protected
kpMainWindow *kpEnvironmentBase::mainWindow() const
{
    return d->mainWindow;
}

#include "moc_kpEnvironmentBase.cpp"
