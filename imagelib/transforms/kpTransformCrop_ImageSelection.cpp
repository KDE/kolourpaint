
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
#include <kpTransformCropPrivate.h>

#include <kpAbstractImageSelection.h>
#include <kpCommandEnvironment.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpImage.h>
#include <kpMacroCommand.h>
#include <kpMainWindow.h>
#include <kpPixmapFX.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpViewManager.h>


// See the "image selection" part of the kpTransformCrop() API Doc.
//
// REFACTOR: Move into commands/
class SetDocumentToSelectionImageCommand : public kpCommand
{
public:
    SetDocumentToSelectionImageCommand (kpCommandEnvironment *environ);
    virtual ~SetDocumentToSelectionImageCommand ();

    /* (uninteresting child of macro cmd) */
    virtual QString name () const { return QString(); }

    virtual kpCommandSize::SizeType size () const
    {
        return ImageSize (m_oldImage) +
               SelectionSize (m_fromSelectionPtr) +
               ImageSize (m_imageIfFromSelectionDoesntHaveOne);
    }

    // ASSUMPTION: Document has been resized to be the same size as the
    //             selection.
    virtual void execute ();
    virtual void unexecute ();

protected:
    kpColor m_backgroundColor;
    kpImage m_oldImage;
    kpAbstractImageSelection *m_fromSelectionPtr;
    kpImage m_imageIfFromSelectionDoesntHaveOne;
};


SetDocumentToSelectionImageCommand::SetDocumentToSelectionImageCommand (kpCommandEnvironment *environ)
    : kpCommand (environ),
      m_backgroundColor (environ->backgroundColor ()),
      m_fromSelectionPtr (
        dynamic_cast <kpAbstractImageSelection *> (
            environ->document ()->selection ()->clone ()))
{
    Q_ASSERT (m_fromSelectionPtr);
    m_imageIfFromSelectionDoesntHaveOne =
        m_fromSelectionPtr->hasContent () ?
            kpImage () :
            document ()->getSelectedBaseImage ();
}

//---------------------------------------------------------------------

SetDocumentToSelectionImageCommand::~SetDocumentToSelectionImageCommand ()
{
    delete m_fromSelectionPtr;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void SetDocumentToSelectionImageCommand::execute ()
{
#if DEBUG_KP_TOOL_CROP
    kDebug () << "SetDocumentToSelectionImageCommand::execute()";
#endif

    viewManager ()->setQueueUpdates ();
    {
        // kpTransformCrop_ImageSelection's <resizeDocCommand> has
        // executed, resizing the document to be the size of the selection
        // bounding rectangle.
        Q_ASSERT (document ()->width () == m_fromSelectionPtr->width ());
        Q_ASSERT (document ()->height () == m_fromSelectionPtr->height ());
        m_oldImage = document ()->image ();


        //
        // e.g. original elliptical selection:
        //
        //     t/---\    T = original transparent selection pixel
        //     | TT |    t = outside the selection region
        //     t\__/t    [every other character] = original opaque selection pixel
        //
        // Afterwards, the _document_ image becomes:
        //
        //      b/---\   T = [unchanged]
        //      | TT |   b = background color
        //      b\__/b   [every other character] = [unchanged]
        //
        // The selection is deleted.
        //
        // TODO: Do not introduce a mask if the result will not contain
        //       any transparent pixels.
        //

        QImage newDocImage(document()->width(), document()->height(), QImage::Format_ARGB32_Premultiplied);
        newDocImage.fill(m_backgroundColor.toQRgb());

    #if DEBUG_KP_TOOL_CROP
        kDebug () << "\tsel: rect=" << m_fromSelectionPtr->boundingRect ()
                   << " pm=" << m_fromSelectionPtr->hasContent ()
                   << endl;
    #endif
        QImage setTransparentImage;

        if (m_fromSelectionPtr->hasContent ())
        {
            setTransparentImage = m_fromSelectionPtr->transparentImage ();

        #if DEBUG_KP_TOOL_CROP
            kDebug () << "\thave pixmap; rect="
                       << setTransparentImage.rect ()
                       << endl;
        #endif
        }
        else
        {
            setTransparentImage = m_imageIfFromSelectionDoesntHaveOne;
        #if DEBUG_KP_TOOL_CROP
            kDebug () << "\tno pixmap in sel - get it; rect="
                       << setTransparentImage.rect ()
                       << endl;
        #endif
        }

#if 0
        kpPixmapFX::paintMaskTransparentWithBrush (&newDocImage,
            QPoint (0, 0),
            m_fromSelectionPtr->shapeBitmap ());
#endif

        kpPixmapFX::paintPixmapAt (&newDocImage,
            QPoint (0, 0),
            setTransparentImage);


        document ()->setImageAt (newDocImage, QPoint (0, 0));
        document ()->selectionDelete ();


        environ ()->somethingBelowTheCursorChanged ();
    }
    viewManager ()->restoreQueueUpdates ();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void SetDocumentToSelectionImageCommand::unexecute ()
{
#if DEBUG_KP_TOOL_CROP
    kDebug () << "SetDocumentToSelectionImageCommand::unexecute()";
#endif

    viewManager ()->setQueueUpdates ();
    {
        document ()->setImageAt (m_oldImage, QPoint (0, 0));
        m_oldImage = kpImage ();

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

//---------------------------------------------------------------------


void kpTransformCrop_ImageSelection (kpMainWindow *mainWindow,
        const QString &commandName, kpCommand *resizeDocCommand)
{
    // Save starting selection, minus the border.
    kpAbstractImageSelection *borderImageSel =
        dynamic_cast <kpAbstractImageSelection *> (
            mainWindow->document ()->selection ()->clone ());
    Q_ASSERT (borderImageSel);
    // (only interested in border)
    borderImageSel->deleteContent ();
    borderImageSel->moveTo (QPoint (0, 0));


    kpCommandEnvironment *environ = mainWindow->commandEnvironment ();


    kpMacroCommand *macroCmd = new kpMacroCommand (commandName, environ);

    // (must resize doc _before_ SetDocumentToSelectionImageCommand in case
    //  doc needs to gets bigger - else selection image may not fit)
    macroCmd->addCommand (resizeDocCommand);

#if DEBUG_KP_TOOL_CROP
    kDebug () << "\tis pixmap sel";
    kDebug () << "\tcreating SetImage cmd";
#endif
    macroCmd->addCommand (new SetDocumentToSelectionImageCommand (environ));


    mainWindow->addImageOrSelectionCommand (
        macroCmd,
        true/*add create cmd*/,
        false/*don't add pull cmd*/);


    // Add selection border back for convenience.
    mainWindow->commandHistory ()->addCommand (
        new kpToolSelectionCreateCommand (
            i18n ("Selection: Create"),
            *borderImageSel,
            mainWindow->commandEnvironment ()));


    delete borderImageSel;
}
