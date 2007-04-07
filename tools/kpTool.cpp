
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


// TODO: reduce number of includes
#include <kpTool.h>
#include <kpToolPrivate.h>

#include <limits.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpBug.h>
#include <kpColor.h>
#include <kpColorToolBar.h>
#include <kpDefs.h>
#include <kpMainWindow.h>
#include <kpPixmapFX.h>
#include <kpToolAction.h>
#include <kpToolToolBar.h>
#include <kpView.h>
#include <kpViewManager.h>
#include <kactioncollection.h>


kpTool::kpTool (const QString &text, const QString &description,
                int key,
                kpMainWindow *mainWindow, const QString &name)
    : QObject (mainWindow),
      d (new kpToolPrivate ())
{
    d->key = key;
    d->action = 0;
    d->ignoreColorSignals = 0;
    d->shiftPressed = false, d->controlPressed = false, d->altPressed = false;  // set in beginInternal()
    d->beganDraw = false;
    d->text = text, d->description = description; setObjectName (name);
    d->mainWindow = mainWindow;
    d->began = false;
    d->viewUnderStartPoint = 0;
    d->userShapeStartPoint = KP_INVALID_POINT;
    d->userShapeEndPoint = KP_INVALID_POINT;
    d->userShapeSize = KP_INVALID_SIZE;

    initAction ();
}

kpTool::~kpTool ()
{
    // before destructing, stop using the tool
    if (d->began)
        endInternal ();

    delete d->action;

    delete d;
}


// private
void kpTool::initAction ()
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool(" << objectName () << "::initAction()" << endl;
#endif

    Q_ASSERT (d->mainWindow);

    KActionCollection *ac = d->mainWindow->actionCollection ();
    Q_ASSERT (ac);


    d->action = new kpToolAction (text (), iconName (), shortcutForKey (d->key),
                                 this, SLOT (slotActionActivated ()),
                                 ac, objectName ());

    // Make tools mutually exclusive by placing them in the same group.
    d->action->setActionGroup (d->mainWindow->toolsActionGroup ());

    d->action->setWhatsThis (description ());

    connect (d->action, SIGNAL (toolTipChanged (const QString &)),
             this, SLOT (slotActionToolTipChanged (const QString &)));
}


// public
QString kpTool::text () const
{
    return d->text;
}

// public
void kpTool::setText (const QString &text)
{
    d->text = text;

    d->action->setText (d->text);
}


static bool KeyIsText (int key)
{
    // TODO: should work like !QKeyEvent::text().isEmpty()
    return !(key & (Qt::KeyboardModifierMask ^ Qt::ShiftModifier));
}

// public static
QString kpTool::toolTipForTextAndShortcut (const QString &text,
                                           const KShortcut &shortcut)
{
    foreach(const QKeySequence &seq, shortcut)
    {
        if (seq.count () == 1 && ::KeyIsText (seq [0]))
            return i18nc ("<Tool Name> (<Single Accel Key>)",
                          "%1 (%2)", text, seq.toString ().toUpper ());
    }

    return text;
}

// public static
QString kpTool::toolTip () const
{
    return toolTipForTextAndShortcut (text (), shortcut ());
}


// public
int kpTool::key () const
{
    return d->key;
}

// public
void kpTool::setKey (int key)
{
    d->key = key;

    // TODO: this probably not wise since it nukes the user's settings
    d->action->setShortcut (shortcutForKey (d->key));
}

// public static
KShortcut kpTool::shortcutForKey (int key)
{
    KShortcut shortcut;

    if (key)
    {
        shortcut.setPrimary (key);
        // (CTRL+<key>, ALT+<key>, CTRL+ALT+<key>, CTRL+SHIFT+<key>
        //  all clash with global KDE shortcuts)
        shortcut.setAlternate (Qt::ALT + Qt::SHIFT + key);
    }

    return shortcut;
}

// public
KShortcut kpTool::shortcut () const
{
    return d->action->shortcut ();
}


// public
QString kpTool::description () const
{
    return d->description;
}

// public
void kpTool::setDescription (const QString &description)
{
    d->description = description;

    d->action->setWhatsThis (d->description);
}


// public
QIcon kpTool::iconSet (int forceSize) const
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool(" << objectName () << ")::iconSet(forceSize=" << forceSize << ")" << endl;
#endif
    // (robust in case BarIcon() default arg changes)
    if (forceSize > 0)
        return BarIconSet (objectName (), forceSize);
    else
        return BarIconSet (objectName ());
}

// public
QString kpTool::iconName () const
{
    return objectName ();
}

// public
kpToolAction *kpTool::action () const
{
    return d->action;
}



// TODO: need to add access specifier comments (like "public virtual [base AmOverridingThisClass'Method]") not just in kpTool but all over KolourPaint source.
kpMainWindow *kpTool::mainWindow () const
{
    return d->mainWindow;
}

kpDocument *kpTool::document () const
{
    return d->mainWindow ? d->mainWindow->document () : 0;
}

kpViewManager *kpTool::viewManager () const
{
    return d->mainWindow ? d->mainWindow->viewManager () : 0;
}

kpToolToolBar *kpTool::toolToolBar () const
{
    return d->mainWindow ? d->mainWindow->toolToolBar () : 0;
}

kpColor kpTool::color (int which) const
{
    Q_ASSERT (d->mainWindow);
    return d->mainWindow->colorToolBar ()->color (which);
}

kpColor kpTool::foregroundColor () const
{
    return color (0);
}

kpColor kpTool::backgroundColor () const
{
    return color (1);
}


double kpTool::colorSimilarity () const
{
    Q_ASSERT (d->mainWindow);
    return d->mainWindow->colorToolBar ()->colorSimilarity ();
}

int kpTool::processedColorSimilarity () const
{
    Q_ASSERT (d->mainWindow);
    return d->mainWindow->colorToolBar ()->processedColorSimilarity ();
}


kpColor kpTool::oldForegroundColor () const
{
    Q_ASSERT (d->mainWindow);
    return d->mainWindow->colorToolBar ()->oldForegroundColor ();
}

kpColor kpTool::oldBackgroundColor () const
{
    Q_ASSERT (d->mainWindow);
    return d->mainWindow->colorToolBar ()->oldBackgroundColor ();
}

double kpTool::oldColorSimilarity () const
{
    Q_ASSERT (d->mainWindow);
    return d->mainWindow->colorToolBar ()->oldColorSimilarity ();
}

kpCommandHistory *kpTool::commandHistory () const
{
    return d->mainWindow ? d->mainWindow->commandHistory () : 0;
}


#include <kpTool.moc>
