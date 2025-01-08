
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_CROP 0

#include "kpTransformCrop.h"
#include "kpTransformCropPrivate.h"

#include "commands/kpCommandHistory.h"
#include "commands/kpMacroCommand.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "document/kpDocument.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "imagelib/kpImage.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "mainWindow/kpMainWindow.h"
#include "pixmapfx/kpPixmapFX.h"
#include "views/manager/kpViewManager.h"

// See the "image selection" part of the kpTransformCrop() API Doc.
//
// REFACTOR: Move into commands/
class SetDocumentToSelectionImageCommand : public kpCommand
{
public:
    explicit SetDocumentToSelectionImageCommand(kpCommandEnvironment *environ);
    ~SetDocumentToSelectionImageCommand() override;

    /* (uninteresting child of macro cmd) */
    QString name() const override
    {
        return {};
    }

    kpCommandSize::SizeType size() const override
    {
        return ImageSize(m_oldImage) + SelectionSize(m_fromSelectionPtr) + ImageSize(m_imageIfFromSelectionDoesntHaveOne);
    }

    // ASSUMPTION: Document has been resized to be the same size as the
    //             selection.
    void execute() override;
    void unexecute() override;

protected:
    kpColor m_backgroundColor;
    kpImage m_oldImage;
    kpAbstractImageSelection *m_fromSelectionPtr;
    kpImage m_imageIfFromSelectionDoesntHaveOne;
};

SetDocumentToSelectionImageCommand::SetDocumentToSelectionImageCommand(kpCommandEnvironment *environ)
    : kpCommand(environ)
    , m_backgroundColor(environ->backgroundColor())
    , m_fromSelectionPtr(dynamic_cast<kpAbstractImageSelection *>(environ->document()->selection()->clone()))
{
    Q_ASSERT(m_fromSelectionPtr);

    if (m_fromSelectionPtr) // make coverity happy
    {
        m_imageIfFromSelectionDoesntHaveOne = m_fromSelectionPtr->hasContent() ? kpImage() : document()->getSelectedBaseImage();
    }
}

//---------------------------------------------------------------------

SetDocumentToSelectionImageCommand::~SetDocumentToSelectionImageCommand()
{
    delete m_fromSelectionPtr;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void SetDocumentToSelectionImageCommand::execute()
{
#if DEBUG_KP_TOOL_CROP
    qCDebug(kpLogImagelib) << "SetDocumentToSelectionImageCommand::execute()";
#endif

    viewManager()->setQueueUpdates();
    {
        // kpTransformCrop_ImageSelection's <resizeDocCommand> has
        // executed, resizing the document to be the size of the selection
        // bounding rectangle.
        Q_ASSERT(document()->width() == m_fromSelectionPtr->width());
        Q_ASSERT(document()->height() == m_fromSelectionPtr->height());
        m_oldImage = document()->image();

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
        qCDebug(kpLogImagelib) << "\tsel: rect=" << m_fromSelectionPtr->boundingRect() << " pm=" << m_fromSelectionPtr->hasContent();
#endif
        QImage setTransparentImage;

        if (m_fromSelectionPtr->hasContent()) {
            setTransparentImage = m_fromSelectionPtr->transparentImage();

#if DEBUG_KP_TOOL_CROP
            qCDebug(kpLogImagelib) << "\thave pixmap; rect=" << setTransparentImage.rect();
#endif
        } else {
            setTransparentImage = m_imageIfFromSelectionDoesntHaveOne;
#if DEBUG_KP_TOOL_CROP
            qCDebug(kpLogImagelib) << "\tno pixmap in sel - get it; rect=" << setTransparentImage.rect();
#endif
        }

        kpPixmapFX::paintPixmapAt(&newDocImage, QPoint(0, 0), setTransparentImage);

        document()->setImageAt(newDocImage, QPoint(0, 0));
        document()->selectionDelete();

        environ()->somethingBelowTheCursorChanged();
    }
    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
void SetDocumentToSelectionImageCommand::unexecute()
{
#if DEBUG_KP_TOOL_CROP
    qCDebug(kpLogImagelib) << "SetDocumentToSelectionImageCommand::unexecute()";
#endif

    viewManager()->setQueueUpdates();
    {
        document()->setImageAt(m_oldImage, QPoint(0, 0));
        m_oldImage = kpImage();

#if DEBUG_KP_TOOL_CROP
        qCDebug(kpLogImagelib) << "\tsel: rect=" << m_fromSelectionPtr->boundingRect() << " pm=" << m_fromSelectionPtr->hasContent();
#endif
        document()->setSelection(*m_fromSelectionPtr);

        environ()->somethingBelowTheCursorChanged();
    }
    viewManager()->restoreQueueUpdates();
}

//---------------------------------------------------------------------

void kpTransformCrop_ImageSelection(kpMainWindow *mainWindow, const QString &commandName, kpCommand *resizeDocCommand)
{
    // Save starting selection, minus the border.
    auto *borderImageSel = dynamic_cast<kpAbstractImageSelection *>(mainWindow->document()->selection()->clone());

    Q_ASSERT(borderImageSel);

    if (!borderImageSel) { // make coverity happy
        return;
    }

    // (only interested in border)
    borderImageSel->deleteContent();
    borderImageSel->moveTo(QPoint(0, 0));

    auto *environ = mainWindow->commandEnvironment();
    auto *macroCmd = new kpMacroCommand(commandName, environ);

    // (must resize doc _before_ SetDocumentToSelectionImageCommand in case
    //  doc needs to get bigger - else selection image may not fit)
    macroCmd->addCommand(resizeDocCommand);

#if DEBUG_KP_TOOL_CROP
    qCDebug(kpLogImagelib) << "\tis pixmap sel";
    qCDebug(kpLogImagelib) << "\tcreating SetImage cmd";
#endif
    macroCmd->addCommand(new SetDocumentToSelectionImageCommand(environ));

    mainWindow->addImageOrSelectionCommand(macroCmd, true /*add create cmd*/, false /*don't add pull cmd*/);

    // Add selection border back for convenience.
    mainWindow->commandHistory()->addCommand(new kpToolSelectionCreateCommand(i18n("Selection: Create"), *borderImageSel, mainWindow->commandEnvironment()));

    delete borderImageSel;
}
