
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


#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "imagelib/kpColor.h"
#include "kpDefs.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "commands/imagelib/effects/kpEffectInvertCommand.h"
#include "commands/imagelib/effects/kpEffectReduceColorsCommand.h"
#include "dialogs/imagelib/effects/kpEffectsDialog.h"
#include "commands/imagelib/effects/kpEffectClearCommand.h"
#include "commands/imagelib/effects/kpEffectGrayscaleCommand.h"
#include "commands/kpMacroCommand.h"
#include "layers/selections/text/kpTextSelection.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "commands/tools/selection/kpToolSelectionPullFromDocumentCommand.h"
#include "commands/tools/selection/text/kpToolTextGiveContentCommand.h"
#include "imagelib/transforms/kpTransformAutoCrop.h"
#include "imagelib/transforms/kpTransformCrop.h"
#include "environments/dialogs/imagelib/transforms/kpTransformDialogEnvironment.h"
#include "commands/imagelib/transforms/kpTransformFlipCommand.h"
#include "commands/imagelib/transforms/kpTransformResizeScaleCommand.h"
#include "dialogs/imagelib/transforms/kpTransformResizeScaleDialog.h"
#include "commands/imagelib/transforms/kpTransformRotateCommand.h"
#include "dialogs/imagelib/transforms/kpTransformRotateDialog.h"
#include "commands/imagelib/transforms/kpTransformSkewCommand.h"
#include "dialogs/imagelib/transforms/kpTransformSkewDialog.h"
#include "views/manager/kpViewManager.h"
#include "commands/imagelib/effects/kpEffectBlurSharpenCommand.h"
#include "imagelib/effects/kpEffectBlurSharpen.h"
#include "kpLogCategories.h"

#include <KActionCollection>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QAction>
#include <QMenuBar>
#include <QSize>


//---------------------------------------------------------------------

// private
kpTransformDialogEnvironment *kpMainWindow::transformDialogEnvironment ()
{
    if (!d->transformDialogEnvironment)
        d->transformDialogEnvironment = new kpTransformDialogEnvironment (this);

    return d->transformDialogEnvironment;
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::isSelectionActive () const
{
    return (d->document ? bool (d->document->selection ()) : false);
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::isTextSelection () const
{
    return (d->document && d->document->textSelection ());
}

//---------------------------------------------------------------------

// private
QString kpMainWindow::autoCropText () const
{
    return kpTransformAutoCropCommand::text(isSelectionActive(),
                                            kpTransformAutoCropCommand::ShowAccel);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setupImageMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    d->actionResizeScale = ac->addAction (QStringLiteral("image_resize_scale"));
    d->actionResizeScale->setText (i18n ("R&esize / Scale..."));
    connect (d->actionResizeScale, &QAction::triggered, this, &kpMainWindow::slotResizeScale);

    ac->setDefaultShortcut (d->actionResizeScale, Qt::CTRL | Qt::Key_E);

    d->actionCrop = ac->addAction (QStringLiteral("image_crop"));
    d->actionCrop->setText (i18n ("Se&t as Image (Crop)"));
    connect (d->actionCrop, &QAction::triggered, this, &kpMainWindow::slotCrop);
    ac->setDefaultShortcut (d->actionCrop, Qt::CTRL | Qt::Key_T);

    d->actionAutoCrop = ac->addAction (QStringLiteral("image_auto_crop"));
    d->actionAutoCrop->setText (autoCropText ());
    connect (d->actionAutoCrop, &QAction::triggered, this, &kpMainWindow::slotAutoCrop);
    ac->setDefaultShortcut (d->actionAutoCrop, Qt::CTRL | Qt::Key_U);

    d->actionFlip = ac->addAction (QStringLiteral("image_flip"));
    d->actionFlip->setText (i18n ("&Flip (upside down)"));
    connect (d->actionFlip, &QAction::triggered, this, &kpMainWindow::slotFlip);
    ac->setDefaultShortcut (d->actionFlip, Qt::CTRL | Qt::Key_F);

    d->actionMirror = ac->addAction (QStringLiteral("image_mirror"));
    d->actionMirror->setText (i18n ("Mirror (horizontally)"));
    connect (d->actionMirror, &QAction::triggered, this, &kpMainWindow::slotMirror);
    //ac->setDefaultShortcut (d->actionMirror, Qt::CTRL | Qt::Key_M);

    d->actionRotate = ac->addAction (QStringLiteral("image_rotate"));
    d->actionRotate->setText (i18n ("&Rotate..."));
    d->actionRotate->setIcon(QIcon::fromTheme(QStringLiteral("transform-rotate")));
    connect (d->actionRotate, &QAction::triggered, this, &kpMainWindow::slotRotate);
    ac->setDefaultShortcut (d->actionRotate, Qt::CTRL | Qt::Key_R);

    d->actionRotateLeft = ac->addAction (QStringLiteral("image_rotate_270deg"));
    d->actionRotateLeft->setText (i18n ("Rotate &Left"));
    d->actionRotateLeft->setIcon(QIcon::fromTheme(QStringLiteral("object-rotate-left")));
    connect (d->actionRotateLeft, &QAction::triggered, this, &kpMainWindow::slotRotate270);
    ac->setDefaultShortcut (d->actionRotateLeft, Qt::CTRL | Qt::SHIFT | Qt::Key_Left);

    d->actionRotateRight = ac->addAction (QStringLiteral("image_rotate_90deg"));
    d->actionRotateRight->setText (i18n ("Rotate Righ&t"));
    d->actionRotateRight->setIcon(QIcon::fromTheme(QStringLiteral("object-rotate-right")));
    connect (d->actionRotateRight, &QAction::triggered, this, &kpMainWindow::slotRotate90);
    ac->setDefaultShortcut (d->actionRotateRight, Qt::CTRL | Qt::SHIFT | Qt::Key_Right);

    d->actionSkew = ac->addAction (QStringLiteral("image_skew"));
    d->actionSkew->setText (i18n ("S&kew..."));
    connect (d->actionSkew, &QAction::triggered, this, &kpMainWindow::slotSkew);
    ac->setDefaultShortcut (d->actionSkew, Qt::CTRL | Qt::Key_K);

    d->actionConvertToBlackAndWhite = ac->addAction (QStringLiteral("image_convert_to_black_and_white"));
    d->actionConvertToBlackAndWhite->setText (i18n ("Reduce to Mo&nochrome (Dithered)"));
    connect (d->actionConvertToBlackAndWhite, &QAction::triggered,
             this, &kpMainWindow::slotConvertToBlackAndWhite);

    d->actionConvertToGrayscale = ac->addAction (QStringLiteral("image_convert_to_grayscale"));
    d->actionConvertToGrayscale->setText (i18n ("Reduce to &Grayscale"));
    connect (d->actionConvertToGrayscale, &QAction::triggered, this, &kpMainWindow::slotConvertToGrayscale);

    d->actionInvertColors = ac->addAction (QStringLiteral("image_invert_colors"));
    d->actionInvertColors->setText (i18n ("&Invert Colors"));
    connect (d->actionInvertColors, &QAction::triggered, this, &kpMainWindow::slotInvertColors);
    ac->setDefaultShortcut (d->actionInvertColors, Qt::CTRL | Qt::Key_I);

    d->actionClear = ac->addAction (QStringLiteral("image_clear"));
    d->actionClear->setText (i18n ("C&lear"));
    connect (d->actionClear, &QAction::triggered, this, &kpMainWindow::slotClear);
    ac->setDefaultShortcut (d->actionClear, Qt::CTRL | Qt::SHIFT | Qt::Key_N);

    d->actionBlur = ac->addAction(QStringLiteral("image_make_confidential"));
    d->actionBlur->setText(i18n("Make Confidential"));
    connect(d->actionBlur, &QAction::triggered, this, &kpMainWindow::slotMakeConfidential);

    d->actionMoreEffects = ac->addAction (QStringLiteral("image_more_effects"));
    d->actionMoreEffects->setText (i18n ("&More Effects..."));
    connect (d->actionMoreEffects, &QAction::triggered, this, &kpMainWindow::slotMoreEffects);
    ac->setDefaultShortcut (d->actionMoreEffects, Qt::CTRL | Qt::Key_M);


    enableImageMenuDocumentActions (false);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableImageMenuDocumentActions (bool enable)
{
    d->actionResizeScale->setEnabled (enable);
    d->actionCrop->setEnabled (enable);
    d->actionAutoCrop->setEnabled (enable);
    d->actionFlip->setEnabled (enable);
    d->actionMirror->setEnabled (enable);
    d->actionRotate->setEnabled (enable);
    d->actionRotateLeft->setEnabled (enable);
    d->actionRotateRight->setEnabled (enable);
    d->actionSkew->setEnabled (enable);
    d->actionConvertToBlackAndWhite->setEnabled (enable);
    d->actionConvertToGrayscale->setEnabled (enable);
    d->actionInvertColors->setEnabled (enable);
    d->actionClear->setEnabled (enable);
    d->actionBlur->setEnabled (enable);
    d->actionMoreEffects->setEnabled (enable);

    d->imageMenuDocumentActionsEnabled = enable;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotImageMenuUpdateDueToSelection ()
{
    // SYNC: kolourpaintui.rc
    const QString MenuBarItemTextImage = i18nc (
        "Image/Selection Menu caption - make sure the translation has"
        " the same accel as the Select&ion translation",
        "&Image");
    const QString MenuBarItemTextSelection = i18nc (
        "Image/Selection Menu caption - make sure that translation has"
        " the same accel as the &Image translation",
        "Select&ion");

    Q_ASSERT (menuBar ());
    for (auto *action : menuBar ()->actions ())
    {
        if (action->text () == MenuBarItemTextImage ||
            action->text () == MenuBarItemTextSelection)
        {
            if (isSelectionActive ()) {
                action->setText (MenuBarItemTextSelection);
            }
            else {
                action->setText (MenuBarItemTextImage);
            }

            break;
        }
    }


    d->actionResizeScale->setEnabled (d->imageMenuDocumentActionsEnabled);
    d->actionCrop->setEnabled (d->imageMenuDocumentActionsEnabled &&
                              isSelectionActive ());

    const bool enable = (d->imageMenuDocumentActionsEnabled && !isTextSelection ());
    d->actionAutoCrop->setText (autoCropText ());
    d->actionAutoCrop->setEnabled (enable);
    d->actionFlip->setEnabled (enable);
    d->actionMirror->setEnabled (enable);
    d->actionRotate->setEnabled (enable);
    d->actionRotateLeft->setEnabled (enable);
    d->actionRotateRight->setEnabled (enable);
    d->actionSkew->setEnabled (enable);
    d->actionConvertToBlackAndWhite->setEnabled (enable);
    d->actionConvertToGrayscale->setEnabled (enable);
    d->actionInvertColors->setEnabled (enable);
    d->actionClear->setEnabled (enable);
    d->actionBlur->setEnabled (enable);
    d->actionMoreEffects->setEnabled (enable);
}

//---------------------------------------------------------------------

// public
kpColor kpMainWindow::backgroundColor (bool ofSelection) const
{
    if (ofSelection) {
        return kpColor::Transparent;
    }

    Q_ASSERT (d->colorToolBar);
    return d->colorToolBar->backgroundColor ();
}

//---------------------------------------------------------------------

// public
// REFACTOR: sync: Code dup with kpAbstractSelectionTool::addNeedingContentCommand().
void kpMainWindow::addImageOrSelectionCommand (kpCommand *cmd,
    bool addSelCreateCmdIfSelAvail,
    bool addSelContentCmdIfSelAvail)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    qCDebug(kpLogMainWindow) << "kpMainWindow::addImageOrSelectionCommand()"
               << " addSelCreateCmdIfSelAvail=" << addSelCreateCmdIfSelAvail
               << " addSelContentCmdIfSelAvail=" << addSelContentCmdIfSelAvail;
#endif

    Q_ASSERT (d->document);


    if (d->viewManager) {
        d->viewManager->setQueueUpdates ();
    }


    kpAbstractSelection *sel = d->document->selection ();
#if DEBUG_KP_MAIN_WINDOW && 1
    qCDebug(kpLogMainWindow) << "\timage sel=" << sel
               << " sel->hasContent=" << (sel ? sel->hasContent () : 0);
#endif
    if (addSelCreateCmdIfSelAvail && sel && !sel->hasContent ())
    {
        QString createCmdName;

        if (dynamic_cast <kpAbstractImageSelection *> (sel)) {
            createCmdName = i18n ("Selection: Create");
        }
        else if (dynamic_cast <kpTextSelection *> (sel)) {
            createCmdName = i18n ("Text: Create Box");
        }
        else {
            Q_ASSERT (!"Unknown selection type");
        }

        // create selection region
        commandHistory ()->addCreateSelectionCommand (
            new kpToolSelectionCreateCommand (
                createCmdName,
                *sel,
                commandEnvironment ()),
            false/*no exec - user already dragged out sel*/);
    }


    if (addSelContentCmdIfSelAvail && sel && !sel->hasContent ())
    {
        auto *imageSel = dynamic_cast <kpAbstractImageSelection *> (sel);
        auto *textSel = dynamic_cast <kpTextSelection *> (sel);

        if (imageSel && imageSel->transparency ().isTransparent ()) {
            d->colorToolBar->flashColorSimilarityToolBarItem ();
        }

        kpMacroCommand *macroCmd = new kpMacroCommand (cmd->name (),
            commandEnvironment ());

        if (imageSel)
        {
            macroCmd->addCommand (
                new kpToolSelectionPullFromDocumentCommand (
                    *imageSel,
                    backgroundColor (),
                    QString()/*uninteresting child of macro cmd*/,
                    commandEnvironment ()));
        }
        else if (textSel)
        {
            macroCmd->addCommand (
                new kpToolTextGiveContentCommand (
                    *textSel,
                    QString()/*uninteresting child of macro cmd*/,
                    commandEnvironment ()));
        }
        else {
            Q_ASSERT (!"Unknown selection type");
        }

        macroCmd->addCommand (cmd);

        d->commandHistory->addCommand (macroCmd);
    }
    else
    {
        d->commandHistory->addCommand (cmd);
    }


    if (d->viewManager) {
        d->viewManager->restoreQueueUpdates ();
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotResizeScale ()
{
    toolEndShape ();

    kpTransformResizeScaleDialog dialog(transformDialogEnvironment(), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        auto *cmd = new kpTransformResizeScaleCommand (
            dialog.actOnSelection (),
            dialog.imageWidth (), dialog.imageHeight (),
            dialog.type (),
            commandEnvironment ());

        bool addSelCreateCommand = (dialog.actOnSelection () ||
                                    cmd->scaleSelectionWithImage ());
        bool addSelContentCommand = dialog.actOnSelection ();

        addImageOrSelectionCommand (
            cmd,
            addSelCreateCommand,
            addSelContentCommand);

        // Resized document?
        if (!dialog.actOnSelection () &&
            dialog.type () == kpTransformResizeScaleCommand::Resize)
        {
            // TODO: this should be the responsibility of kpDocument
            saveDefaultDocSize (QSize (dialog.imageWidth (), dialog.imageHeight ()));
        }
    }
}

//---------------------------------------------------------------------

// public slot
void kpMainWindow::slotCrop ()
{
    toolEndShape ();

    Q_ASSERT (d->document && d->document->selection ());


    ::kpTransformCrop (this);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotAutoCrop ()
{
    toolEndShape ();

    ::kpTransformAutoCrop (this);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotFlip()
{
    toolEndShape();

    addImageOrSelectionCommand(
        new kpTransformFlipCommand(d->document->selection(),
                                   false, true, commandEnvironment()));
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotMirror()
{
    toolEndShape();

    addImageOrSelectionCommand(
        new kpTransformFlipCommand(d->document->selection(),
                                   true, false, commandEnvironment()));
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotRotate ()
{
    toolEndShape ();

    kpTransformRotateDialog dialog (static_cast<bool> (d->document->selection ()),
        transformDialogEnvironment (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpTransformRotateCommand (d->document->selection (),
                dialog.angle (),
                commandEnvironment ()));
    }
}

// private slot
void kpMainWindow::slotRotate270 ()
{
    toolEndShape ();

    // TODO: Special command name instead of just "Rotate"?
    addImageOrSelectionCommand (
        new kpTransformRotateCommand (
            d->document->selection (),
            270,
            commandEnvironment ()));
}

// private slot
void kpMainWindow::slotRotate90 ()
{
    toolEndShape ();

    // TODO: Special command name instead of just "Rotate"?
    addImageOrSelectionCommand (
        new kpTransformRotateCommand (
            d->document->selection (),
            90,
            commandEnvironment ()));
}


// private slot
void kpMainWindow::slotSkew ()
{
    toolEndShape ();

    kpTransformSkewDialog dialog (static_cast<bool> (d->document->selection ()),
        transformDialogEnvironment (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpTransformSkewCommand (d->document->selection (),
                dialog.horizontalAngle (), dialog.verticalAngle (),
                commandEnvironment ()));
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotConvertToBlackAndWhite ()
{
    toolEndShape ();

    addImageOrSelectionCommand (
        new kpEffectReduceColorsCommand (1/*depth*/, true/*dither*/,
            d->document->selection (),
            commandEnvironment ()));
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotConvertToGrayscale ()
{
    toolEndShape ();

    addImageOrSelectionCommand (
        new kpEffectGrayscaleCommand (d->document->selection (),
            commandEnvironment ()));
}

//--------------------------------------------------------------------------------

// private slot
void kpMainWindow::slotInvertColors ()
{
    toolEndShape ();

    addImageOrSelectionCommand (
        new kpEffectInvertCommand (d->document->selection (),
            commandEnvironment ()));
}

//--------------------------------------------------------------------------------

// private slot
void kpMainWindow::slotClear ()
{
    toolEndShape ();

    addImageOrSelectionCommand (
        new kpEffectClearCommand (
            d->document->selection (),
            backgroundColor (),
            commandEnvironment ()));
}

//--------------------------------------------------------------------------------

void kpMainWindow::slotMakeConfidential()
{
    toolEndShape();

    addImageOrSelectionCommand(
        new kpEffectBlurSharpenCommand(kpEffectBlurSharpen::MakeConfidential, kpEffectBlurSharpen::MaxStrength,
                                       d->document->selection(), commandEnvironment()));
}

//--------------------------------------------------------------------------------

// private slot
void kpMainWindow::slotMoreEffects ()
{
    toolEndShape ();

    kpEffectsDialog dialog (static_cast<bool> (d->document->selection ()),
        transformDialogEnvironment (), this,
        d->moreEffectsDialogLastEffect);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (dialog.createCommand ());
    }


    if (d->moreEffectsDialogLastEffect != dialog.selectedEffect ())
    {
        d->moreEffectsDialogLastEffect = dialog.selectedEffect ();

        KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);

        cfg.writeEntry (kpSettingMoreEffectsLastEffect,
                         d->moreEffectsDialogLastEffect);
        cfg.sync ();
    }
}

//--------------------------------------------------------------------------------
