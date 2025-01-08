
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "environments/commands/kpCommandEnvironment.h"

#include "document/kpDocument.h"
#include "layers/selections/image/kpImageSelectionTransparency.h"
#include "layers/selections/text/kpTextStyle.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/kpColorToolBar.h"

struct kpCommandEnvironmentPrivate {
};

kpCommandEnvironment::kpCommandEnvironment(kpMainWindow *mainWindow)
    : kpEnvironmentBase(mainWindow)
    , d(new kpCommandEnvironmentPrivate())
{
}

kpCommandEnvironment::~kpCommandEnvironment()
{
    delete d;
}

// public
void kpCommandEnvironment::setColor(int which, const kpColor &color) const
{
    kpColorToolBar *toolBar = mainWindow()->colorToolBar();
    Q_ASSERT(toolBar);

    toolBar->setColor(which, color);
}

// public
void kpCommandEnvironment::somethingBelowTheCursorChanged() const
{
    kpTool *tool = mainWindow()->tool();
    Q_ASSERT(tool);

    tool->somethingBelowTheCursorChanged();
}

// public
kpImageSelectionTransparency kpCommandEnvironment::imageSelectionTransparency() const
{
    return mainWindow()->imageSelectionTransparency();
}

// public
void kpCommandEnvironment::setImageSelectionTransparency(const kpImageSelectionTransparency &transparency, bool forceColorChange)
{
    mainWindow()->setImageSelectionTransparency(transparency, forceColorChange);
}

// public
kpTextStyle kpCommandEnvironment::textStyle() const
{
    return mainWindow()->textStyle();
}

// public
void kpCommandEnvironment::setTextStyle(const kpTextStyle &textStyle)
{
    mainWindow()->setTextStyle(textStyle);
}

#include "moc_kpCommandEnvironment.cpp"
