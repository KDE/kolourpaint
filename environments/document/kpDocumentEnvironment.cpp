
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


#define DEBUG_KP_DOCUMENT_ENVIRONMENT 1


#include <kpDocumentEnvironment.h>

#include <KDebug>

#include <kpMainWindow.h>
#include <kpAbstractSelection.h>
#include <kpEllipticalImageSelection.h>
#include <kpFreeFormImageSelection.h>
#include <kpImageSelectionTransparency.h>
#include <kpRectangularImageSelection.h>
#include <kpTextSelection.h>
#include <kpTextStyle.h>
#if DEBUG_KP_DOCUMENT_ENVIRONMENT
    #include <kpTool.h>
#endif
#include <kpViewManager.h>


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


// public
void kpDocumentEnvironment::switchToCompatibleTool (const kpAbstractSelection &selection,
        bool *isTextChanged) const
{
#if DEBUG_KP_DOCUMENT_ENVIRONMENT
    kDebug () << "kpDocumentEnvironment::switchToCompatibleTool("
              << &selection << ")"
              << " mainwindow.tool="
              << (mainWindow ()->tool () ? mainWindow ()->tool ()->objectName () : 0)
              << " mainWindow.toolIsTextTool=" << mainWindow ()->toolIsTextTool ()
              << " selection is text="
              << dynamic_cast <const kpTextSelection *> (&selection)
              << endl;
#endif

    *isTextChanged = (mainWindow ()->toolIsTextTool () !=
                     (dynamic_cast <const kpTextSelection *> (&selection) != 0));

    // (we don't change the Selection Tool if the new selection's
    //  shape is different to the tool's because all the Selection
    //  Tools act the same, except for what would be really irritating
    //  if it kept changing whenever you paste an image - drawing the
    //  selection region)
    if (!mainWindow ()->toolIsASelectionTool () || *isTextChanged)
    {
        // Switch to the appropriately shaped selection tool
        // _before_ we change the selection
        // (all selection tool's ::end() functions nuke the current selection)
        if (dynamic_cast <const kpRectangularImageSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            kDebug () << "\tswitch to rect selection tool";
        #endif
            mainWindow ()->slotToolRectSelection ();
        }
        else if (dynamic_cast <const kpEllipticalImageSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            kDebug () << "\tswitch to elliptical selection tool";
        #endif
            mainWindow ()->slotToolEllipticalSelection ();
        }
        else if (dynamic_cast <const kpFreeFormImageSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            kDebug () << "\tswitch to free form selection tool";
        #endif
            mainWindow ()->slotToolFreeFormSelection ();
        }
        else if (dynamic_cast <const kpTextSelection *> (&selection))
        {
        #if DEBUG_KP_DOCUMENT_ENVIRONMENT
            kDebug () << "\tswitch to text selection tool";
        #endif
            mainWindow ()->slotToolText ();
        }
        else
            Q_ASSERT (!"Unknown selection type");
    }

#if DEBUG_KP_DOCUMENT_ENVIRONMENT
    kDebug () << "kpDocumentEnvironment::switchToCompatibleTool(" << &selection << ") finished";
#endif
}

// public
void kpDocumentEnvironment::assertMatchingUIState (const kpAbstractSelection &selection) const
{
    // Trap and try to recover from bugs.
    // TODO: See kpDocument::setSelection() API comment and determine best fix.
    const kpAbstractImageSelection *imageSelection =
        dynamic_cast <const kpAbstractImageSelection *> (&selection);
    const kpTextSelection *textSelection =
        dynamic_cast <const kpTextSelection *> (&selection);
    if (imageSelection)
    {
        if (imageSelection->transparency () != mainWindow ()->imageSelectionTransparency ())
        {
            kError () << "kpDocument::setSelection() sel's transparency differs "
                          "from mainWindow's transparency - setting mainWindow's transparency "
                          "to sel"
                       << endl;
            kError () << "\tisOpaque: sel=" << imageSelection->transparency ().isOpaque ()
                       << " mainWindow=" << mainWindow ()->imageSelectionTransparency ().isOpaque ()
                       << endl;
            mainWindow ()->setImageSelectionTransparency (imageSelection->transparency ());
        }
    }
    else if (textSelection)
    {
        if (textSelection->textStyle () != mainWindow ()->textStyle ())
        {
            kError () << "kpDocument::setSelection() sel's textStyle differs "
                          "from mainWindow's textStyle - setting mainWindow's textStyle "
                          "to sel"
                       << endl;
            mainWindow ()->setTextStyle (textSelection->textStyle ());
        }
    }
    else
    {
        Q_ASSERT (!"Unknown selection type");
    }
}


#include <kpDocumentEnvironment.moc>
