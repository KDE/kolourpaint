
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


#define DEBUG_KP_DOCUMENT_ENVIRONMENT 0


#include "environments/document/kpDocumentEnvironment.h"

#include "kpLogCategories.h"

#include "mainWindow/kpMainWindow.h"
#include "layers/selections/kpAbstractSelection.h"
#include "document/kpDocument.h"
#include "layers/selections/image/kpEllipticalImageSelection.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"
#include "layers/selections/image/kpImageSelectionTransparency.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "layers/selections/text/kpTextSelection.h"
#include "layers/selections/text/kpTextStyle.h"
#if DEBUG_KP_DOCUMENT_ENVIRONMENT
    #include "tools/kpTool.h"
#endif
#include "views/manager/kpViewManager.h"


struct kpDocumentEnvironmentPrivate
{
};

kpDocumentEnvironment::kpDocumentEnvironment (kpMainWindow *mainWindow)
    : kpEnvironmentBase (mainWindow),
      d (new kpDocumentEnvironmentPrivate ())
{
}

kpDocumentEnvironment::~kpDocumentEnvironment ()
{
    delete d;
}


// public
QWidget *kpDocumentEnvironment::dialogParent () const
{
    return mainWindow ();
}


static kpViewManager *ViewManager (kpMainWindow *mw)
{
    return mw->viewManager ();
}

// public
void kpDocumentEnvironment::setQueueViewUpdates () const
{
    ::ViewManager (mainWindow ())->setQueueUpdates ();
}

// public
void kpDocumentEnvironment::restoreQueueViewUpdates () const
{
    ::ViewManager (mainWindow ())->restoreQueueUpdates ();
}

//---------------------------------------------------------------------

// public
void kpDocumentEnvironment::switchToCompatibleTool (const kpAbstractSelection &selection,
        bool *isTextChanged) const
{
#if DEBUG_KP_DOCUMENT_ENVIRONMENT
    qCDebug(kpLogEnvironments) << "kpDocumentEnvironment::switchToCompatibleTool("
              << &selection << ")"
              << " mainwindow.tool="
              << (mainWindow ()->tool () ? mainWindow ()->tool ()->objectName () : nullptr)
              << " mainWindow.toolIsTextTool=" << mainWindow ()->toolIsTextTool ()
              << " current selection="
              << document ()->selection ()
              << " new selection is text="
              << dynamic_cast <const kpTextSelection *> (&selection);
#endif

    *isTextChanged = (mainWindow ()->toolIsTextTool () !=
                     (dynamic_cast <const kpTextSelection *> (&selection) != nullptr));

    // We don't change the Selection Tool if the new selection's
    // shape is merely different to the current tool's (e.g. rectangular
    // vs elliptical) because:
    //
    // 1. All image selection tools support editing selections of all the
    //    different shapes anyway.
    // 2. Suppose the user is trying out different drags of selection borders
    //    and then decides to paste a differently shaped selection before continuing
    //    to try out different borders.  If the pasting were to switch to
    //    a differently shaped tool, the borders drawn after the paste would
    //    be using a new shape rather than the shape before the paste.  This
    //    could get irritating so we don't do the switch.
    if (!mainWindow ()->toolIsASelectionTool () || *isTextChanged)
    {
        // See kpDocument::setSelection() APIDoc for this assumption.
        Q_ASSERT (!document ()->selection ());

        // Switch to the appropriately shaped selection tool
        // _before_ we change the selection
        // (all selection tool's ::end() functions nuke the current selection)
        if (dynamic_cast <const kpRectangularImageSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            qCDebug(kpLogEnvironments) << "\tswitch to rect selection tool";
        #endif
            mainWindow ()->slotToolRectSelection ();
        }
        else if (dynamic_cast <const kpEllipticalImageSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            qCDebug(kpLogEnvironments) << "\tswitch to elliptical selection tool";
        #endif
            mainWindow ()->slotToolEllipticalSelection ();
        }
        else if (dynamic_cast <const kpFreeFormImageSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            qCDebug(kpLogEnvironments) << "\tswitch to free form selection tool";
        #endif
            mainWindow ()->slotToolFreeFormSelection ();
        }
        else if (dynamic_cast <const kpTextSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            qCDebug(kpLogEnvironments) << "\tswitch to text selection tool";
        #endif
            mainWindow ()->slotToolText ();
        }
        else {
            Q_ASSERT (!"Unknown selection type");
        }
    }

#if DEBUG_KP_DOCUMENT_ENVIRONMENT
    qCDebug(kpLogEnvironments) << "kpDocumentEnvironment::switchToCompatibleTool(" << &selection << ") finished";
#endif
}

//---------------------------------------------------------------------

// public
void kpDocumentEnvironment::assertMatchingUIState (const kpAbstractSelection &selection) const
{
    // Trap and try to recover from bugs.
    // TODO: See kpDocument::setSelection() API comment and determine best fix.
    const auto *imageSelection = dynamic_cast <const kpAbstractImageSelection *> (&selection);
    const auto *textSelection = dynamic_cast <const kpTextSelection *> (&selection);

    if (imageSelection)
    {
        if (imageSelection->transparency () != mainWindow ()->imageSelectionTransparency ())
        {
            qCCritical(kpLogEnvironments) << "kpDocument::setSelection() sel's transparency differs "
                          "from mainWindow's transparency - setting mainWindow's transparency "
                          "to sel";

            qCCritical(kpLogEnvironments) << "\tisOpaque: sel=" << imageSelection->transparency ().isOpaque ()
                       << " mainWindow=" << mainWindow ()->imageSelectionTransparency ().isOpaque ();

            mainWindow ()->setImageSelectionTransparency (imageSelection->transparency ());
        }
    }
    else if (textSelection)
    {
        if (textSelection->textStyle () != mainWindow ()->textStyle ())
        {
            qCCritical(kpLogEnvironments) << "kpDocument::setSelection() sel's textStyle differs "
                          "from mainWindow's textStyle - setting mainWindow's textStyle "
                          "to sel";

            mainWindow ()->setTextStyle (textSelection->textStyle ());
        }
    }
    else
    {
        Q_ASSERT (!"Unknown selection type");
    }
}


