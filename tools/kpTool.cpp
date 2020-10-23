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

//
// Tool initialisation and basic accessors.
//

#define DEBUG_KP_TOOL 0

#include "kpTool.h"
#include "kpToolPrivate.h"

#include <climits>

#include <KActionCollection>
#include "kpLogCategories.h"
#include <KLocalizedString>

#include "imagelib/kpColor.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "tools/kpToolAction.h"
#include "environments/tools/kpToolEnvironment.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#undef environ  // macro on win32

//---------------------------------------------------------------------

kpTool::kpTool(const QString &text, const QString &description,
               int key,
               kpToolEnvironment *environ,
               QObject *parent, const QString &name)
    : QObject(parent),
      d(new kpToolPrivate())
{
    d->key = key;
    d->action = nullptr;
    d->ignoreColorSignals = 0;
    d->shiftPressed = false;
    d->controlPressed = false;
    d->altPressed = false;  // set in beginInternal()
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

kpTool::~kpTool ()
{
    // before destructing, stop using the tool
    if (d->began) {
        endInternal ();
    }

    delete d->action;

    delete d;
}

//---------------------------------------------------------------------

// private
void kpTool::initAction ()
{
    KActionCollection *ac = d->environ->actionCollection ();
    Q_ASSERT (ac);

    d->action = new kpToolAction(text(), objectName(), shortcutForKey(d->key),
                                 this, SIGNAL(actionActivated()),
                                 ac, objectName());

    // Make tools mutually exclusive by placing them in the same group.
    d->action->setActionGroup(d->environ->toolsActionGroup ());

    d->action->setWhatsThis(d->description);

    connect (d->action, &kpToolAction::changed, this, &kpTool::actionToolTipChanged);
}

//---------------------------------------------------------------------
// public

QString kpTool::text () const
{
    return d->text;
}

//---------------------------------------------------------------------

static bool KeyIsText (int key)
{
    // TODO: should work like !QKeyEvent::text().isEmpty()
    return !(static_cast<unsigned int> (key) & (Qt::KeyboardModifierMask ^ Qt::ShiftModifier));
}

//---------------------------------------------------------------------

// public static
QString kpTool::toolTipForTextAndShortcut (const QString &text,
                                           const QList<QKeySequence> &shortcut)
{
    for(const auto &seq : shortcut)
    {
        if (seq.count () == 1 && ::KeyIsText (seq [0])) {
            return i18nc ("<Tool Name> (<Single Accel Key>)", "%1 (%2)", text, seq.toString ().toUpper ());
        }
    }

    return text;
}

//---------------------------------------------------------------------

QString kpTool::toolTip () const
{
    return toolTipForTextAndShortcut(d->text, d->action->shortcuts());
}

//---------------------------------------------------------------------
// public static

QList<QKeySequence> kpTool::shortcutForKey (int key)
{
    QList<QKeySequence> shortcut;

    if (key)
    {
        shortcut.append (QKeySequence (key));
        // (CTRL+<key>, ALT+<key>, CTRL+ALT+<key>, CTRL+SHIFT+<key>
        //  all clash with global KDE shortcuts)
        shortcut.append (QKeySequence (static_cast<int>(Qt::ALT) + static_cast<int>(Qt::SHIFT) + key));
    }

    return shortcut;
}

//---------------------------------------------------------------------
// public

kpToolAction *kpTool::action () const
{
    return d->action;
}

//---------------------------------------------------------------------

kpDocument *kpTool::document () const
{
    return d->environ->document ();
}

//---------------------------------------------------------------------

kpViewManager *kpTool::viewManager () const
{
    return d->environ->viewManager ();
}

//---------------------------------------------------------------------

kpToolToolBar *kpTool::toolToolBar () const
{
    return d->environ->toolToolBar ();
}

//---------------------------------------------------------------------

kpColor kpTool::color (int which) const
{
    return d->environ->color (which);
}

//---------------------------------------------------------------------

kpColor kpTool::foregroundColor () const
{
    return color (0);
}

//---------------------------------------------------------------------

kpColor kpTool::backgroundColor () const
{
    return color (1);
}

//---------------------------------------------------------------------

// TODO: Some of these might not be common enough.
//       Just put in kpToolEnvironment?

double kpTool::colorSimilarity () const
{
    return d->environ->colorSimilarity ();
}

int kpTool::processedColorSimilarity () const
{
    return d->environ->processedColorSimilarity ();
}


kpColor kpTool::oldForegroundColor () const
{
    return d->environ->oldForegroundColor ();
}

kpColor kpTool::oldBackgroundColor () const
{
    return d->environ->oldBackgroundColor ();
}

double kpTool::oldColorSimilarity () const
{
    return d->environ->oldColorSimilarity ();
}

kpCommandHistory *kpTool::commandHistory () const
{
    return d->environ->commandHistory ();
}


kpToolEnvironment *kpTool::environ () const
{
    return d->environ;
}
