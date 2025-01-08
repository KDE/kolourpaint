
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpToolEnvironment.h"

#include "imagelib/kpColor.h"
#include "mainWindow/kpMainWindow.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "widgets/toolbars/kpToolToolBar.h"

#include <KActionCollection>

#include <QActionGroup>
#include <QPoint>
#include <QString>

//--------------------------------------------------------------------------------

bool kpToolEnvironment::drawAntiAliased = true;

//--------------------------------------------------------------------------------

struct kpToolEnvironmentPrivate {
};

kpToolEnvironment::kpToolEnvironment(kpMainWindow *mainWindow)
    : kpEnvironmentBase(mainWindow)
    , d(new kpToolEnvironmentPrivate())
{
}

kpToolEnvironment::~kpToolEnvironment()
{
    delete d;
}

// public
KActionCollection *kpToolEnvironment::actionCollection() const
{
    return mainWindow()->actionCollection();
}

// public
kpCommandHistory *kpToolEnvironment::commandHistory() const
{
    return mainWindow()->commandHistory();
}

// public
QActionGroup *kpToolEnvironment::toolsActionGroup() const
{
    return mainWindow()->toolsActionGroup();
}

// public
kpToolToolBar *kpToolEnvironment::toolToolBar() const
{
    return mainWindow()->toolToolBar();
}

// public
void kpToolEnvironment::hideAllToolWidgets() const
{
    toolToolBar()->hideAllToolWidgets();
}

// public
bool kpToolEnvironment::selectPreviousTool() const
{
    kpToolToolBar *tb = toolToolBar();

    // (don't end up with no tool selected)
    if (!tb->previousTool()) {
        return false;
    }

    // endInternal() will be called by kpMainWindow (thanks to this line)
    // so we won't have the view anymore
    // TODO: Update comment.
    tb->selectPreviousTool();
    return true;
}

static kpColorToolBar *ColorToolBar(kpMainWindow *mw)
{
    return mw->colorToolBar();
}

// public
kpColor kpToolEnvironment::color(int which) const
{
    return ::ColorToolBar(mainWindow())->color(which);
}

// public
double kpToolEnvironment::colorSimilarity() const
{
    return ::ColorToolBar(mainWindow())->colorSimilarity();
}

// public
int kpToolEnvironment::processedColorSimilarity() const
{
    return ::ColorToolBar(mainWindow())->processedColorSimilarity();
}

// public
kpColor kpToolEnvironment::oldForegroundColor() const
{
    return ::ColorToolBar(mainWindow())->oldForegroundColor();
}

// public
kpColor kpToolEnvironment::oldBackgroundColor() const
{
    return ::ColorToolBar(mainWindow())->oldBackgroundColor();
}

// public
double kpToolEnvironment::oldColorSimilarity() const
{
    return ::ColorToolBar(mainWindow())->oldColorSimilarity();
}

// public
void kpToolEnvironment::flashColorSimilarityToolBarItem() const
{
    ::ColorToolBar(mainWindow())->flashColorSimilarityToolBarItem();
}

// public
void kpToolEnvironment::setColor(int which, const kpColor &color) const
{
    kpColorToolBar *toolBar = mainWindow()->colorToolBar();
    Q_ASSERT(toolBar);

    toolBar->setColor(which, color);
}

// public
void kpToolEnvironment::deleteSelection() const
{
    mainWindow()->slotDelete();
}

// public
void kpToolEnvironment::pasteTextAt(const QString &text, const QPoint &point, bool allowNewTextSelectionPointShift) const
{
    mainWindow()->pasteTextAt(text, point, allowNewTextSelectionPointShift);
}

// public
void kpToolEnvironment::zoomIn(bool centerUnderCursor) const
{
    mainWindow()->zoomIn(centerUnderCursor);
}

// public
void kpToolEnvironment::zoomOut(bool centerUnderCursor) const
{
    mainWindow()->zoomOut(centerUnderCursor);
}

// public
void kpToolEnvironment::zoomToRect(const QRect &normalizedDocRect, bool accountForGrips, bool careAboutWidth, bool careAboutHeight) const
{
    mainWindow()->zoomToRect(normalizedDocRect, accountForGrips, careAboutWidth, careAboutHeight);
}

// public
void kpToolEnvironment::fitToPage() const
{
    mainWindow()->slotFitToPage();
}

#include "moc_kpToolEnvironment.cpp"
