/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

//
// Tool initialisation and basic accessors.
//

#define DEBUG_KP_TOOL 0

#include "kpTool.h"
#include "kpToolPrivate.h"

#include <climits>

#include "kpLogCategories.h"
#include <KActionCollection>
#include <KLocalizedString>

#include "environments/tools/kpToolEnvironment.h"
#include "imagelib/kpColor.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "widgets/toolbars/kpToolToolBar.h"
#undef environ // macro on win32

//---------------------------------------------------------------------

kpTool::kpTool(const QString &text, const QString &description, int key, kpToolEnvironment *environ, QObject *parent, const QString &name)
    : QObject(parent)
    , d(new kpToolPrivate())
{
    d->key = key;
    d->action = nullptr;
    d->ignoreColorSignals = 0;
    d->shiftPressed = false;
    d->controlPressed = false;
    d->altPressed = false; // set in beginInternal()
    d->beganDraw = false;
    d->text = text;
    d->description = description;
    d->began = false;
    d->viewUnderStartPoint = nullptr;
    d->userShapeStartPoint = KP_INVALID_POINT;
    d->userShapeEndPoint = KP_INVALID_POINT;
    d->userShapeSize = KP_INVALID_SIZE;

    d->environ = environ;

    setObjectName(name);
    initAction();
}

//---------------------------------------------------------------------

kpTool::~kpTool()
{
    // before destructing, stop using the tool
    if (d->began) {
        endInternal();
    }

    delete d->action;

    delete d;
}

//---------------------------------------------------------------------

// private
void kpTool::initAction()
{
    KActionCollection *ac = d->environ->actionCollection();
    Q_ASSERT(ac);

    d->action = ac->add<KToggleAction>(objectName());
    d->action->setText(text());
    d->action->setWhatsThis(d->description);
    d->action->setIcon(QIcon::fromTheme(objectName()));
    ac->setDefaultShortcuts(d->action, shortcutForKey(d->key));

    connect(d->action, &QAction::triggered, this, &kpTool::actionActivated);

    // Make tools mutually exclusive by placing them in the same group.
    d->action->setActionGroup(d->environ->toolsActionGroup());
}

//---------------------------------------------------------------------
// public

QString kpTool::text() const
{
    return d->text;
}

//---------------------------------------------------------------------
// public static

QList<QKeySequence> kpTool::shortcutForKey(int key)
{
    QList<QKeySequence> shortcut;

    if (key) {
        shortcut.append(QKeySequence(key));
        // (CTRL+<key>, ALT+<key>, CTRL+ALT+<key>, CTRL+SHIFT+<key>
        //  all clash with global KDE shortcuts)
        shortcut.append(QKeySequence(static_cast<int>(Qt::ALT) + static_cast<int>(Qt::SHIFT) + key));
    }

    return shortcut;
}

//---------------------------------------------------------------------
// public

KToggleAction *kpTool::action() const
{
    return d->action;
}

//---------------------------------------------------------------------

kpDocument *kpTool::document() const
{
    return d->environ->document();
}

//---------------------------------------------------------------------

kpViewManager *kpTool::viewManager() const
{
    return d->environ->viewManager();
}

//---------------------------------------------------------------------

kpToolToolBar *kpTool::toolToolBar() const
{
    return d->environ->toolToolBar();
}

//---------------------------------------------------------------------

kpColor kpTool::color(int which) const
{
    return d->environ->color(which);
}

//---------------------------------------------------------------------

kpColor kpTool::foregroundColor() const
{
    return color(0);
}

//---------------------------------------------------------------------

kpColor kpTool::backgroundColor() const
{
    return color(1);
}

//---------------------------------------------------------------------

// TODO: Some of these might not be common enough.
//       Just put in kpToolEnvironment?

double kpTool::colorSimilarity() const
{
    return d->environ->colorSimilarity();
}

int kpTool::processedColorSimilarity() const
{
    return d->environ->processedColorSimilarity();
}

kpColor kpTool::oldForegroundColor() const
{
    return d->environ->oldForegroundColor();
}

kpColor kpTool::oldBackgroundColor() const
{
    return d->environ->oldBackgroundColor();
}

double kpTool::oldColorSimilarity() const
{
    return d->environ->oldColorSimilarity();
}

kpCommandHistory *kpTool::commandHistory() const
{
    return d->environ->commandHistory();
}

kpToolEnvironment *kpTool::environ() const
{
    return d->environ;
}

#include "moc_kpTool.cpp"
