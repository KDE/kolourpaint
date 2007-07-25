
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

#define DEBUG_KP_TOOL_CROP 0


#include <kpTransformCrop.h>

#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpAbstractImageSelection.h>
#include <kpColor.h>
#include <kpCommandEnvironment.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpEffectClearCommand.h>
#include <kpMacroCommand.h>
#include <kpMainWindow.h>
#include <kpTextSelection.h>
#include <kpTool.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionMoveCommand.h>
#include <kpTransformResizeScaleCommand.h>
#include <kpViewManager.h>


kpAbstractSelection *selectionBorderAndMovedTo0_0 (const kpAbstractSelection &sel)
{
    kpAbstractSelection *borderSel = sel.clone ();

    kpAbstractImageSelection *borderImageSel =
        dynamic_cast <kpAbstractImageSelection *> (borderSel);
    if (borderImageSel)
        borderImageSel->setBaseImage (kpImage ());  // only interested in border
    borderSel->moveTo (QPoint (0, 0));

    return borderSel;
}


//
// kpTransformCropSetImageCommand
//

// TODO: Move into commands/
class kpTransformCropSetImageCommand : public kpCommand
{
public:
    kpTransformCropSetImageCommand (kpCommandEnvironment *environ);
    virtual ~kpTransformCropSetImageCommand ();

    /* (uninteresting child of macro cmd) */
    virtual QString name () const { return QString::null; }

    virtual kpCommandSize::SizeType size () const
    {
        return ImageSize (m_oldPixmap) +
               SelectionSize (m_fromSelectionPtr) +
               ImageSize (m_pixmapIfFromSelectionDoesntHaveOne);
    }

    virtual void execute ();
    virtual void unexecute ();

protected:
    kpColor m_backgroundColor;
    QPixmap m_oldPixmap;
    kpAbstractSelection *m_fromSelectionPtr;
    QPixmap m_pixmapIfFromSelectionDoesntHaveOne;
};


static bool SelectionHasImage (const kpAbstractSelection &selection)
{
    if (dynamic_cast <const kpTextSelection *> (&selection))
    {
        // A text selection can be meaningfully displayed even if it just
        // an empty box, with no text content...
        return true;
    }

    const kpAbstractImageSelection *imageSel =
        dynamic_cast <const kpAbstractImageSelection *> (&selection);
    Q_ASSERT (imageSel);

    // ... but an image selection really needs to have pulled image content
    // from the document to be meaningfully displayed.
    return imageSel->hasContent ();
}


kpTransformCropSetImageCommand::kpTransformCropSetImageCommand (kpCommandEnvironment *environ)
    : kpCommand (environ),
      m_backgroundColor (environ->backgroundColor ()),
      m_fromSelectionPtr (environ->document ()->selection ()->clone ()),
      m_pixmapIfFromSelectionDoesntHaveOne (
        ::SelectionHasImage (*m_fromSelectionPtr) ?
            kpImage () :
            document ()->getSelectedBaseImage ())
{
}

kpTransformCropSetImageCommand::~kpTransformCropSetImageCommand ()
{
    delete m_fromSelectionPtr;
}


// public virtual [base kpCommand]
void kpTransformCropSetImageCommand::execute ()
{
#if DEBUG_KP_TOOL_CROP
    kDebug () << "kpTransformCropSetImageCommand::execute()" << endl;
#endif

    viewManager ()->setQueueUpdates ();
    {
        m_oldPixmap = kpPixmapFX::getPixmapAt (document ()->image (),
            QRect (0, 0, m_fromSelectionPtr->width (), m_fromSelectionPtr->height ()));


        //
        // Original rounded rectangle selection:
        //
        //      T/---\      ...............
        //      | TT |      T = Transparent
        //      T\__/T      ...............
        //
        // After Crop Outside the Selection, the _image_ becomes:
        //
        //      Bttttt
        //      ttTTtt      T,t = Transparent
        //      BttttB      B = Background Colour
        //
        // The selection pixmap stays the same.
        //

        QPixmap newDocPixmap (m_fromSelectionPtr->width (), m_fromSelectionPtr->height ());
        kpPixmapFX::fill (&newDocPixmap, m_backgroundColor);

    #if DEBUG_KP_TOOL_CROP
        kDebug () << "\tsel: rect=" << m_fromSelectionPtr->boundingRect ()
                   << " pm=" << m_fromSelectionPtr->hasContent ()
                   << endl;
    #endif
        QPixmap selTransparentPixmap;

        kpAbstractImageSelection *imageSel =
            dynamic_cast <kpAbstractImageSelection *> (m_fromSelectionPtr);
        kpTextSelection *textSel =
            dynamic_cast <kpTextSelection *> (m_fromSelectionPtr);

        if (::SelectionHasImage (*m_fromSelectionPtr))
        {
            if (imageSel)
                selTransparentPixmap = imageSel->transparentImage ();
            else if (textSel)
                selTransparentPixmap = textSel->approximateImage ();
            else
                Q_ASSERT (!"Unknown selection type");

        #if DEBUG_KP_TOOL_CROP
            kDebug () << "\thave pixmap; rect="
                       << selTransparentPixmap.rect ()
                       << endl;
        #endif
        }
        else
        {
            selTransparentPixmap = m_pixmapIfFromSelectionDoesntHaveOne;
        #if DEBUG_KP_TOOL_CROP
            kDebug () << "\tno pixmap in sel - get it; rect="
                       << selTransparentPixmap.rect ()
                       << endl;
        #endif
        }

        if (imageSel)
        {
            kpPixmapFX::paintMaskTransparentWithBrush (&newDocPixmap,
                QPoint (0, 0),
                imageSel->shapeBitmap ());
        }
        else if (textSel)
        {
            // COMPAT: port
        }
        else
            Q_ASSERT (!"Unknown selection type");

        kpPixmapFX::paintPixmapAt (&newDocPixmap,
            QPoint (0, 0),
            selTransparentPixmap);


        document ()->setImageAt (newDocPixmap, QPoint (0, 0));
        document ()->selectionDelete ();


        environ ()->somethingBelowTheCursorChanged ();
    }
    viewManager ()->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpTransformCropSetImageCommand::unexecute ()
{
#if DEBUG_KP_TOOL_CROP
    kDebug () << "kpTransformCropSetImageCommand::unexecute()" << endl;
#endif

    viewManager ()->setQueueUpdates ();
    {
        document ()->setImageAt (m_oldPixmap, QPoint (0, 0));
        m_oldPixmap = QPixmap ();

    #if DEBUG_KP_TOOL_CROP
        kDebug () << "\tsel: rect=" << m_fromSelectionPtr->boundingRect ()
                   << " pm=" << m_fromSelectionPtr->hasContent ()
                   << endl;
    #endif
        document ()->setSelection (*m_fromSelectionPtr);

        environ ()->somethingBelowTheCursorChanged ();
    }
    viewManager ()->restoreQueueUpdates ();
}


//
// kpTransformCropCommand
//


// TODO: Move into commands/
class kpTransformCropCommand : public kpMacroCommand
{
public:
    kpTransformCropCommand (kpCommandEnvironment *environ);
    virtual ~kpTransformCropCommand ();
};


kpTransformCropCommand::kpTransformCropCommand (kpCommandEnvironment *environ)
    : kpMacroCommand (i18n ("Set as Image"), environ)
{
#if DEBUG_KP_TOOL_CROP
    kDebug () << "kpTransformCropCommand::<ctor>()" << endl;
#endif

    Q_ASSERT (document () &&
       document ()->selection ());

    kpAbstractSelection *sel = document ()->selection ();


#if DEBUG_KP_TOOL_CROP
    kDebug () << "\tsel: w=" << sel->width ()
               << " h=" << sel->height ()
               << " <- resizing doc to these dimen" << endl;
#endif

    // (must resize doc _before_ kpTransformCropSetImageCommand in case doc
    //  needs to gets bigger - else pasted down pixmap may not fit)
    addCommand (
        new kpTransformResizeScaleCommand (
            false/*act on doc, not sel*/,
            sel->width (), sel->height (),
            kpTransformResizeScaleCommand::Resize,
            environ));


    if (textSelection ())
    {
    #if DEBUG_KP_TOOL_CROP
        kDebug () << "\tisText" << endl;
        kDebug () << "\tclearing doc with trans cmd" << endl;
    #endif
        addCommand (
            new kpEffectClearCommand (
                false/*act on doc*/,
                kpColor::Transparent,
                environ));

    #if DEBUG_KP_TOOL_CROP
        kDebug () << "\tmoving sel to (0,0) cmd" << endl;
    #endif
        kpToolSelectionMoveCommand *moveCmd =
            new kpToolSelectionMoveCommand (
                QString::null/*uninteresting child of macro cmd*/,
                environ);
        moveCmd->moveTo (QPoint (0, 0), true/*move on exec, not now*/);
        moveCmd->finalize ();
        addCommand (moveCmd);
    }
    else
    {
    #if DEBUG_KP_TOOL_CROP
        kDebug () << "\tis pixmap sel" << endl;
        kDebug () << "\tcreating SetImage cmd" << endl;
    #endif
        addCommand (new kpTransformCropSetImageCommand (environ));

    #if 0
        kpAbstractImageSelection *newSel = selectionBorderAndMovedTo0_0 (*sel);
        addCommand (
            new kpToolSelectionCreateCommand (
                QString::null/*uninteresting child of macro cmd*/,
                *newSel,
                environ ()));
        delete newSel;
    #endif
    }
}

kpTransformCropCommand::~kpTransformCropCommand ()
{
}


void kpTransformCrop (kpMainWindow *mainWindow)
{
    kpDocument *doc = mainWindow->document ();
    Q_ASSERT (doc);

    kpAbstractSelection *sel = doc->selection ();
    Q_ASSERT (sel);


    bool selWasText = dynamic_cast <kpTextSelection *> (sel);
    kpAbstractSelection *borderSel = selectionBorderAndMovedTo0_0 (*sel);


    mainWindow->addImageOrSelectionCommand (
        new kpTransformCropCommand (mainWindow->commandEnvironment ()),
        true/*add create cmd*/,
        false/*don't add pull cmd*/);


    if (!selWasText)
    {
        mainWindow->commandHistory ()->addCommand (
            new kpToolSelectionCreateCommand (
                i18n ("Selection: Create"),
                *borderSel,
                mainWindow->commandEnvironment ()));
    }


    delete borderSel;
}
