
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


#include "kpToolEnvironment.h"

#include "imagelib/kpColor.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "mainWindow/kpMainWindow.h"
#include "widgets/toolbars/kpToolToolBar.h"

#include <KActionCollection>

#include <QActionGroup>
#include <QPoint>
#include <QString>

//--------------------------------------------------------------------------------

bool kpToolEnvironment::drawAntiAliased = true;

//--------------------------------------------------------------------------------

struct kpToolEnvironmentPrivate
{
};

kpToolEnvironment::kpToolEnvironment (kpMainWindow *mainWindow)
    : kpEnvironmentBase (mainWindow),
      d (new kpToolEnvironmentPrivate ())
{
}

kpToolEnvironment::~kpToolEnvironment ()
{
    delete d;
}


// public
KActionCollection *kpToolEnvironment::actionCollection () const
{
    return mainWindow ()->actionCollection ();
}

// public
kpCommandHistory *kpToolEnvironment::commandHistory () const
{
    return mainWindow ()->commandHistory ();
}


// public
QActionGroup *kpToolEnvironment::toolsActionGroup () const
{
    return mainWindow ()->toolsActionGroup ();
}

// public
kpToolToolBar *kpToolEnvironment::toolToolBar () const
{
    return mainWindow ()->toolToolBar ();
}

// public
void kpToolEnvironment::hideAllToolWidgets () const
{
    toolToolBar ()->hideAllToolWidgets ();
}

// public
bool kpToolEnvironment::selectPreviousTool () const
{
    kpToolToolBar *tb = toolToolBar ();

    // (don't end up with no tool selected)
    if (!tb->previousTool ()) {
        return false;
    }

    // endInternal() will be called by kpMainWindow (thanks to this line)
    // so we won't have the view anymore
    // TODO: Update comment.
    tb->selectPreviousTool ();
    return true;
}


static kpColorToolBar *ColorToolBar (kpMainWindow *mw)
{
    return mw->colorToolBar ();
}

// public
kpColor kpToolEnvironment::color (int which) const
{
    return ::ColorToolBar (mainWindow ())->color (which);
}

// public
double kpToolEnvironment::colorSimilarity () const
{
    return ::ColorToolBar (mainWindow ())->colorSimilarity ();
}

// public
int kpToolEnvironment::processedColorSimilarity () const
{
    return ::ColorToolBar (mainWindow ())->processedColorSimilarity ();
}

// public
kpColor kpToolEnvironment::oldForegroundColor () const
{
    return ::ColorToolBar (mainWindow ())->oldForegroundColor ();
}

// public
kpColor kpToolEnvironment::oldBackgroundColor () const
{
    return ::ColorToolBar (mainWindow ())->oldBackgroundColor ();
}

// public
double kpToolEnvironment::oldColorSimilarity () const
{
    return ::ColorToolBar (mainWindow ())->oldColorSimilarity ();
}


// public
void kpToolEnvironment::flashColorSimilarityToolBarItem () const
{
    ::ColorToolBar (mainWindow ())->flashColorSimilarityToolBarItem ();
}


// public
void kpToolEnvironment::setColor (int which, const kpColor &color) const
{
    kpColorToolBar *toolBar = mainWindow ()->colorToolBar ();
    Q_ASSERT (toolBar);

    toolBar->setColor (which, color);
}


// public
void kpToolEnvironment::deleteSelection () const
{
    mainWindow ()->slotDelete ();
}

// public
void kpToolEnvironment::pasteTextAt (const QString &text, const QPoint &point,
                    bool allowNewTextSelectionPointShift) const
{
    mainWindow ()->pasteTextAt (text, point, allowNewTextSelectionPointShift);
}


// public
void kpToolEnvironment::zoomIn (bool centerUnderCursor) const
{
    mainWindow ()->zoomIn (centerUnderCursor);
}

// public
void kpToolEnvironment::zoomOut (bool centerUnderCursor) const
{
    mainWindow ()->zoomOut (centerUnderCursor);
}


// public
void kpToolEnvironment::zoomToRect (
        const QRect &normalizedDocRect,
        bool accountForGrips,
        bool careAboutWidth, bool careAboutHeight) const
{
    mainWindow ()->zoomToRect (
        normalizedDocRect,
        accountForGrips,
        careAboutWidth, careAboutHeight);
}

// public
void kpToolEnvironment::fitToPage () const
{
    mainWindow ()->slotFitToPage ();
}


