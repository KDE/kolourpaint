
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


#define DEBUG_KP_TOOL_SELECTION 0


#include "kpAbstractImageSelectionTool.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "commands/kpMacroCommand.h"
#include "generic/kpSetOverrideCursorSaver.h"
#include "layers/selections/text/kpTextSelection.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "commands/tools/selection/kpToolSelectionDestroyCommand.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/kpToolSelectionMoveCommand.h"
#include "commands/tools/selection/kpToolSelectionResizeScaleCommand.h"
#include "commands/tools/selection/kpToolImageSelectionTransparencyCommand.h"
#include "commands/tools/selection/text/kpToolTextGiveContentCommand.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "kpLogCategories.h"

#include <KLocalizedString>


// protected
bool kpAbstractImageSelectionTool::shouldChangeImageSelectionTransparency () const
{
    if (environ ()->settingImageSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        qCDebug(kpLogTools) << "\trecursion - abort setting selection transparency: "
                   << environ ()->settingImageSelectionTransparency ();
    #endif
        return false;
    }

    if (!document ()->imageSelection ()) {
        return false;
    }

    // TODO: Can probably return false if the selection transparency mode
    //       is Opaque, since neither background color nor color similarity
    //       would matter.

    return true;
}

// protected
void kpAbstractImageSelectionTool::changeImageSelectionTransparency (
        const QString &name,
        const kpImageSelectionTransparency &newTrans,
        const kpImageSelectionTransparency &oldTrans)
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "CALL(" << name << ")";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);

    if (hasBegunShape ()) {
        endShapeInternal ();
    }

    kpAbstractImageSelection *imageSel = document ()->imageSelection ();

    if (imageSel->hasContent () && newTrans.isTransparent ()) {
        environ ()->flashColorSimilarityToolBarItem ();
    }

    imageSel->setTransparency (newTrans);

    // We _must_ add the command even if kpAbstractImageSelection::setTransparency()
    // above did not change the selection transparency mask at all.
    // Consider the following case:
    //
    //   0. Ensure that selection transparency is opaque and any
    //      color other than red is the background color.  Ensure that
    //      the color similarity setting is 0.
    //
    //   1. Select a solid red rectangle and pull it off.
    //
    //   2. Switch to transparent and set red as the background color.
    //      [the selection is now invisible as red is the background
    //       color, which is the same as the contents of the selection]
    //
    //   3. Invert Colors.
    //      [the selection is now cyan, red is still the background color]
    //
    //   4. Change the background color to green.
    //      [the selection transparency mask does not change so the
    //       selection is still cyan; green is the background color]
    //
    //   5. Undo
    //
    // If no transparency command were added for Step 4., the Undo
    // in Step 5. would take us straight to the state after Step 2.,
    // where we would expect the red selection to be invisible.  However,
    // as the background color was changed to green in Step 4. and was not
    // undone, the red selection is not invisible when it should be -- Undo
    // has moved us to an incorrect state.
    //
    // KDE3: Copy this comment into the KDE 3 branch.
    commandHistory ()->addCommand (new kpToolImageSelectionTransparencyCommand (
        name,
        newTrans, oldTrans,
        environ ()->commandEnvironment ()),
        false/*no exec*/);
}


// protected slot virtual [kpAbstractSelectionTool]
void kpAbstractImageSelectionTool::slotIsOpaqueChanged (bool /*isOpaque*/)
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractImageSelectionTool::slotIsOpaqueChanged()";
#endif

    if (!shouldChangeImageSelectionTransparency ()) {
        return;
    }

    kpImageSelectionTransparency st = environ ()->imageSelectionTransparency ();

    kpImageSelectionTransparency oldST = st;
    oldST.setOpaque (!oldST.isOpaque ());

    changeImageSelectionTransparency (
        st.isOpaque () ?
            i18n ("Selection: Opaque") :
            i18n ("Selection: Transparent"),
        st, oldST);
}

// protected slot virtual [base kpTool]
void kpAbstractImageSelectionTool::slotBackgroundColorChanged (const kpColor &)
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractImageSelectionTool::slotBackgroundColorChanged()";
#endif

    if (!shouldChangeImageSelectionTransparency ()) {
        return;
    }

    kpImageSelectionTransparency st = environ ()->imageSelectionTransparency ();

    kpImageSelectionTransparency oldST = st;
    oldST.setTransparentColor (oldBackgroundColor ());

    changeImageSelectionTransparency (
        i18n ("Selection: Transparency Color"),
        st, oldST);
}

// protected slot virtual [base kpTool]
void kpAbstractImageSelectionTool::slotColorSimilarityChanged (double, int)
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogTools) << "kpAbstractImageSelectionTool::slotColorSimilarityChanged()";
#endif

    if (!shouldChangeImageSelectionTransparency ()) {
        return;
    }

    kpImageSelectionTransparency st = environ ()->imageSelectionTransparency ();

    kpImageSelectionTransparency oldST = st;
    oldST.setColorSimilarity (oldColorSimilarity ());

    changeImageSelectionTransparency (
        i18n ("Selection: Transparency Color Similarity"),
        st, oldST);
}
