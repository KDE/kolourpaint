
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpToolSelectionEnvironment.h"

#include "layers/selections/image/kpImageSelectionTransparency.h"
#include "layers/selections/text/kpTextStyle.h"
#include "mainWindow/kpMainWindow.h"

struct kpToolSelectionEnvironmentPrivate {
};

kpToolSelectionEnvironment::kpToolSelectionEnvironment(kpMainWindow *mainWindow)
    : kpToolEnvironment(mainWindow)
    , d(new kpToolSelectionEnvironmentPrivate())
{
}

kpToolSelectionEnvironment::~kpToolSelectionEnvironment()
{
    delete d;
}

// public
kpImageSelectionTransparency kpToolSelectionEnvironment::imageSelectionTransparency() const
{
    return mainWindow()->imageSelectionTransparency();
}

// public
int kpToolSelectionEnvironment::settingImageSelectionTransparency() const
{
    return mainWindow()->settingImageSelectionTransparency();
}

// public
void kpToolSelectionEnvironment::deselectSelection() const
{
    mainWindow()->slotDeselect();
}

// public
QMenu *kpToolSelectionEnvironment::selectionToolRMBMenu() const
{
    return mainWindow()->selectionToolRMBMenu();
}

// public
void kpToolSelectionEnvironment::enableTextToolBarActions(bool enable) const
{
    mainWindow()->enableTextToolBarActions(enable);
}

// public
kpTextStyle kpToolSelectionEnvironment::textStyle() const
{
    return mainWindow()->textStyle();
}

// public
int kpToolSelectionEnvironment::settingTextStyle() const
{
    return mainWindow()->settingTextStyle();
}

#include "moc_kpToolSelectionEnvironment.cpp"
