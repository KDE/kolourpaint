
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


#define DEBUG_KP_TOOL_SELECTION 1


#include <kpAbstractImageSelectionTool.h>

// LOREFACTOR: Remove unneeded #include
#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpAbstractImageSelection.h>
#include <kpAbstractSelection.h>
#include <kpBug.h>
#include <kpCommandHistory.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpMacroCommand.h>
#include <kpTextSelection.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionDestroyCommand.h>
#include <kpToolSelectionEnvironment.h>
#include <kpToolSelectionMoveCommand.h>
#include <kpToolSelectionResizeScaleCommand.h>
#include <kpToolImageSelectionTransparencyCommand.h>
#include <kpToolTextGiveContentCommand.h>
#include <kpToolToolBar.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


// private slot
// HIREFACTOR: Who calls us?
void kpAbstractImageSelectionTool::selectionTransparencyChanged (const QString & /*name*/)
{
#if 0
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpAbstractImageSelectionTool::selectionTransparencyChanged(" << name << ")" << endl;
#endif

    if (environ ()->settingImageSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << environ ()->settingImageSelectionTransparency () << endl;
    #endif
        return;
    }

    if (document ()->selection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave sel - set transparency" << endl;
    #endif

        kpImageSelectionTransparency oldST = document ()->selection ()->transparency ();
        kpImageSelectionTransparency st = environ ()->selectionTransparency ();

        // TODO: This "NOP" check causes us a great deal of trouble e.g.:
        //
        //       Select a solid red rectangle.
        //       Switch to transparent and set red as the background colour.
        //       (the selection is now invisible)
        //       Invert Colours.
        //       (the selection is now cyan)
        //       Change the background colour to green.
        //       (no command is added to undo this as the selection does not change)
        //       Undo.
        //       The rectangle is no longer invisible.
        //
        //if (document ()->selection ()->setTransparency (st, true/*check harder for no change in mask*/))

        document ()->selection ()->setTransparency (st);
        if (true)
        {
        #if DEBUG_KP_TOOL_SELECTION
            kDebug () << "\t\twhich changed the image" << endl;
        #endif

            commandHistory ()->addCommand (new kpToolImageSelectionTransparencyCommand (
                i18n ("Selection: Transparency"), // name,
                st, oldST,
                environ ()->commandEnvironment ()),
                false/* no exec*/);
        }
    }
#endif

    // TODO: I've duplicated the code (see below 3x) to make sure
    //       kpImageSelectionTransparency(oldST)::transparentColor() is defined
    //       and not taken from kpDocument (where it may not be defined because
    //       the transparency may be opaque).
    //
    //       That way kpToolImageSelectionTransparencyCommand can force set colours.
}


// protected slot virtual [kpAbstractSelectionTool]
void kpAbstractImageSelectionTool::slotIsOpaqueChanged (bool /*isOpaque*/)
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpAbstractImageSelectionTool::slotIsOpaqueChanged()" << endl;
#endif

    if (environ ()->settingImageSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << environ ()->settingImageSelectionTransparency () << endl;
    #endif
        return;
    }

    kpAbstractImageSelection *imageSel = document ()->imageSelection ();
    if (imageSel)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave image sel - set transparency" << endl;
    #endif

        QApplication::setOverrideCursor (Qt::WaitCursor);

        if (hasBegunShape ())
            endShapeInternal ();

        kpImageSelectionTransparency st = environ ()->imageSelectionTransparency ();
        kpImageSelectionTransparency oldST = st;
        oldST.setOpaque (!oldST.isOpaque ());

        if (imageSel->hasContent () && st.isTransparent ())
            environ ()->flashColorSimilarityToolBarItem ();

        imageSel->setTransparency (st);

        commandHistory ()->addCommand (new kpToolImageSelectionTransparencyCommand (
            st.isOpaque () ?
                i18n ("Selection: Opaque") :
                i18n ("Selection: Transparent"),
            st, oldST,
            environ ()->commandEnvironment ()),
            false/* no exec*/);

        QApplication::restoreOverrideCursor ();
    }
}

// protected slot virtual [base kpTool]
void kpAbstractImageSelectionTool::slotBackgroundColorChanged (const kpColor &)
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpAbstractImageSelectionTool::slotBackgroundColorChanged()" << endl;
#endif

    if (environ ()->settingImageSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << environ ()->settingImageSelectionTransparency () << endl;
    #endif
        return;
    }

    kpAbstractImageSelection *imageSel = document ()->imageSelection ();
    if (imageSel)
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave image sel (hasContent=" << imageSel->hasContent ()
                  << ") - set transparency" << endl;
    #endif

        QApplication::setOverrideCursor (Qt::WaitCursor);

        kpImageSelectionTransparency st = environ ()->imageSelectionTransparency ();
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\tisTransparent=" << st.isTransparent () << endl;
    #endif
        kpImageSelectionTransparency oldST = st;
        oldST.setTransparentColor (oldBackgroundColor ());

        if (imageSel->hasContent () && st.isTransparent ())
            environ ()->flashColorSimilarityToolBarItem ();

        imageSel->setTransparency (st);

        commandHistory ()->addCommand (new kpToolImageSelectionTransparencyCommand (
            i18n ("Selection: Transparency Color"),
            st, oldST,
            environ ()->commandEnvironment ()),
            false/* no exec*/);

        QApplication::restoreOverrideCursor ();
    }
}

// protected slot virtual [base kpTool]
void kpAbstractImageSelectionTool::slotColorSimilarityChanged (double, int)
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpAbstractImageSelectionTool::slotColorSimilarityChanged()" << endl;
#endif

    if (environ ()->settingImageSelectionTransparency ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\trecursion - abort setting selection transparency: "
                   << environ ()->settingImageSelectionTransparency () << endl;
    #endif
        return;
    }

    kpAbstractImageSelection *imageSel = document ()->imageSelection ();
    if (document ()->imageSelection ())
    {
    #if DEBUG_KP_TOOL_SELECTION
        kDebug () << "\thave image sel - set transparency" << endl;
    #endif

        QApplication::setOverrideCursor (Qt::WaitCursor);

        kpImageSelectionTransparency st = environ ()->imageSelectionTransparency ();
        kpImageSelectionTransparency oldST = st;
        oldST.setColorSimilarity (oldColorSimilarity ());

        if (imageSel->hasContent () && st.isTransparent ())
            environ ()->flashColorSimilarityToolBarItem ();

        imageSel->setTransparency (st);

        commandHistory ()->addCommand (new kpToolImageSelectionTransparencyCommand (
            i18n ("Selection: Transparency Color Similarity"),
            st, oldST,
            environ ()->commandEnvironment ()),
            false/* no exec*/);

        QApplication::restoreOverrideCursor ();
    }
}
