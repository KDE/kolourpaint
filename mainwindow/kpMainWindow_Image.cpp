
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


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <qcolor.h>
#include <qsize.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenubar.h>

#include <kpColor.h>
#include <kpDefs.h>
#include <kpColorToolBar.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpEffectInvertCommand.h>
#include <kpEffectReduceColorsCommand.h>
#include <kpEffectsDialog.h>
#include <kpSelection.h>
#include <kpTool.h>
#include <kpTransformAutoCrop.h>
#include <kpEffectClearCommand.h>
#include <kpEffectGrayscaleCommand.h>
#include <kpTransformCrop.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionPullFromDocumentCommand.h>
#include <kpTransformFlipCommand.h>
#include <kpTransformFlipDialog.h>
#include <kpTransformResizeScaleCommand.h>
#include <kpTransformResizeScaleDialog.h>
#include <kpTransformRotateCommand.h>
#include <kpTransformRotateDialog.h>
#include <kpTransformSkewCommand.h>
#include <kpTransformSkewDialog.h>
#include <kpViewManager.h>
#include <kglobal.h>
#include <kconfiggroup.h>


// private
bool kpMainWindow::isSelectionActive () const
{
    return (m_document ? bool (m_document->selection ()) : false);
}

// private
bool kpMainWindow::isTextSelection () const
{
    return (m_document && m_document->selection () &&
            m_document->selection ()->isText ());
}


// private
QString kpMainWindow::autoCropText () const
{
    return kpTransformAutoCropCommand::name (isSelectionActive (),
                                        kpTransformAutoCropCommand::ShowAccel);
}


// private
void kpMainWindow::setupImageMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionResizeScale = ac->addAction ("image_resize_scale");
    m_actionResizeScale->setText (i18n ("R&esize / Scale..."));
    connect(m_actionResizeScale, SIGNAL(triggered(bool) ), SLOT (slotResizeScale ()));
    m_actionResizeScale->setShortcut(Qt::CTRL + Qt::Key_E);

    m_actionCrop = ac->addAction ("image_crop");
    m_actionCrop->setText (i18n ("Se&t as Image (Crop)"));
    connect(m_actionCrop, SIGNAL(triggered(bool) ), SLOT (slotCrop ()));
    m_actionCrop->setShortcut(Qt::CTRL + Qt::Key_T);

    m_actionAutoCrop = ac->addAction ("image_auto_crop");
    m_actionAutoCrop->setText (autoCropText ());
    connect(m_actionAutoCrop, SIGNAL(triggered(bool) ), SLOT (slotAutoCrop ()));
    m_actionAutoCrop->setShortcut(Qt::CTRL + Qt::Key_U);

    m_actionFlip = ac->addAction ("image_flip");
    m_actionFlip->setText (i18n ("&Flip..."));
    connect(m_actionFlip, SIGNAL(triggered(bool) ), SLOT (slotFlip ()));
    m_actionFlip->setShortcut(Qt::CTRL + Qt::Key_F);

    m_actionRotate = ac->addAction ("image_rotate");
    m_actionRotate->setText (i18n ("&Rotate..."));
    connect(m_actionRotate, SIGNAL(triggered(bool) ), SLOT (slotRotate ()));
    m_actionRotate->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_R);

    m_actionRotate = ac->addAction ("image_rotate_270deg");
    m_actionRotate->setText (i18n ("Rotate 90 Degrees Counterclockwise"));
    connect(m_actionRotate, SIGNAL(triggered(bool) ), SLOT (slotRotate270 ()));
    m_actionRotate->setShortcut(Qt::CTRL + Qt::Key_L);

    m_actionRotate = ac->addAction ("image_rotate_90deg");
    m_actionRotate->setText (i18n ("Rotate 90 Degrees Clockwise"));
    connect(m_actionRotate, SIGNAL(triggered(bool) ), SLOT (slotRotate90 ()));
    m_actionRotate->setShortcut(Qt::CTRL + Qt::Key_R);

    m_actionSkew = ac->addAction ("image_skew");
    m_actionSkew->setText (i18n ("S&kew..."));
    connect(m_actionSkew, SIGNAL(triggered(bool) ), SLOT (slotSkew ()));
    m_actionSkew->setShortcut(Qt::CTRL + Qt::Key_K);

    m_actionConvertToBlackAndWhite = ac->addAction ("image_convert_to_black_and_white");
    m_actionConvertToBlackAndWhite->setText (i18n ("Reduce to Mo&nochrome (Dithered)"));
    connect(m_actionConvertToBlackAndWhite, SIGNAL(triggered(bool) ), SLOT (slotConvertToBlackAndWhite ()));

    m_actionConvertToGrayscale = ac->addAction ("image_convert_to_grayscale");
    m_actionConvertToGrayscale->setText (i18n ("Reduce to &Grayscale"));
    connect(m_actionConvertToGrayscale, SIGNAL(triggered(bool) ), SLOT (slotConvertToGrayscale ()));

    m_actionInvertColors = ac->addAction ("image_invert_colors");
    m_actionInvertColors->setText (i18n ("&Invert Colors"));
    connect(m_actionInvertColors, SIGNAL(triggered(bool) ), SLOT (slotInvertColors ()));
    m_actionInvertColors->setShortcut(Qt::CTRL + Qt::Key_I);

    m_actionClear = ac->addAction ("image_clear");
    m_actionClear->setText (i18n ("C&lear"));
    connect(m_actionClear, SIGNAL(triggered(bool) ), SLOT (slotClear ()));
    m_actionClear->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);

    m_actionMoreEffects = ac->addAction ("image_more_effects");
    m_actionMoreEffects->setText (i18n ("&More Effects..."));
    connect(m_actionMoreEffects, SIGNAL(triggered(bool) ), SLOT (slotMoreEffects ()));
    m_actionMoreEffects->setShortcut(Qt::CTRL + Qt::Key_M);

    enableImageMenuDocumentActions (false);
}

// private
void kpMainWindow::enableImageMenuDocumentActions (bool enable)
{
    m_actionResizeScale->setEnabled (enable);
    m_actionCrop->setEnabled (enable);
    m_actionAutoCrop->setEnabled (enable);
    m_actionFlip->setEnabled (enable);
    m_actionRotate->setEnabled (enable);
    m_actionSkew->setEnabled (enable);
    m_actionConvertToBlackAndWhite->setEnabled (enable);
    m_actionConvertToGrayscale->setEnabled (enable);
    m_actionInvertColors->setEnabled (enable);
    m_actionClear->setEnabled (enable);
    m_actionMoreEffects->setEnabled (enable);

    m_imageMenuDocumentActionsEnabled = enable;
}


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
    foreach (QAction *action, menuBar ()->actions ())
    {
        if (action->text () == MenuBarItemTextImage ||
            action->text () == MenuBarItemTextSelection)
        {
            if (isSelectionActive ())
                action->setText (MenuBarItemTextSelection);
            else
                action->setText (MenuBarItemTextImage);

            break;
        }
    }


    m_actionResizeScale->setEnabled (m_imageMenuDocumentActionsEnabled);
    m_actionCrop->setEnabled (m_imageMenuDocumentActionsEnabled &&
                              isSelectionActive ());

    const bool enable = (m_imageMenuDocumentActionsEnabled && !isTextSelection ());
    m_actionAutoCrop->setText (autoCropText ());
    m_actionAutoCrop->setEnabled (enable);
    m_actionFlip->setEnabled (enable);
    m_actionRotate->setEnabled (enable);
    m_actionSkew->setEnabled (enable);
    m_actionConvertToBlackAndWhite->setEnabled (enable);
    m_actionConvertToGrayscale->setEnabled (enable);
    m_actionInvertColors->setEnabled (enable);
    m_actionClear->setEnabled (enable);
    m_actionMoreEffects->setEnabled (enable);
}


// public
kpColor kpMainWindow::backgroundColor (bool ofSelection) const
{
    if (ofSelection)
        return kpColor::Transparent;
    else
    {
        if (m_colorToolBar)
            return m_colorToolBar->backgroundColor ();
        else
        {
            kError () << "kpMainWindow::backgroundColor() without colorToolBar" << endl;
            return kpColor::Invalid;
        }
    }
}


// public
void kpMainWindow::addImageOrSelectionCommand (kpCommand *cmd,
    bool addSelCreateCmdIfSelAvail,
    bool addSelPullCmdIfSelAvail)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::addImageOrSelectionCommand()"
               << " addSelCreateCmdIfSelAvail=" << addSelCreateCmdIfSelAvail
               << " addSelPullCmdIfSelAvail=" << addSelPullCmdIfSelAvail
               << endl;
#endif

    if (!m_document)
    {
        kError () << "kpMainWindow::addImageOrSelectionCommand() without doc" << endl;
        return;
    }


    if (m_viewManager)
        m_viewManager->setQueueUpdates ();


    kpSelection *sel = m_document->selection ();
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "\tsel=" << sel
               << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
               << endl;
#endif
    if (addSelCreateCmdIfSelAvail && sel && !sel->pixmap ())
    {
        // create selection region
        kpCommand *createCommand = new kpToolSelectionCreateCommand (
            i18n ("Selection: Create"),
            *sel,
            this);

        if (kpToolSelectionCreateCommand::nextUndoCommandIsCreateBorder (commandHistory ()))
            commandHistory ()->setNextUndoCommand (createCommand);
        else
            commandHistory ()->addCommand (createCommand,
                                           false/*no exec - user already dragged out sel*/);
    }


    if (addSelPullCmdIfSelAvail && sel && !sel->pixmap ())
    {
        kpMacroCommand *macroCmd = new kpMacroCommand (cmd->name (), this);

        macroCmd->addCommand (new kpToolSelectionPullFromDocumentCommand (
            QString::null/*uninteresting child of macro cmd*/,
            this));

        macroCmd->addCommand (cmd);

        m_commandHistory->addCommand (macroCmd);
    }
    else
    {
        m_commandHistory->addCommand (cmd);
    }


    if (m_viewManager)
        m_viewManager->restoreQueueUpdates ();
}

// private slot
void kpMainWindow::slotResizeScale ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpTransformResizeScaleDialog dialog (this);
    dialog.setKeepAspectRatio (d->m_resizeScaleDialogLastKeepAspect);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        kpTransformResizeScaleCommand *cmd = new kpTransformResizeScaleCommand (
            dialog.actOnSelection (),
            dialog.imageWidth (), dialog.imageHeight (),
            dialog.type (),
            this);

        bool addSelCreateCommand = (dialog.actOnSelection () ||
                                    cmd->scaleSelectionWithImage ());
        bool addSelPullCommand = dialog.actOnSelection ();

        addImageOrSelectionCommand (
            cmd,
            addSelCreateCommand,
            addSelPullCommand);

        // Resized document?
        if (!dialog.actOnSelection () &&
            dialog.type () == kpTransformResizeScaleCommand::Resize)
        {
            // TODO: this should be the responsibility of kpDocument
            saveDefaultDocSize (QSize (dialog.imageWidth (), dialog.imageHeight ()));
        }
    }


    if (d->m_resizeScaleDialogLastKeepAspect != dialog.keepAspectRatio ())
    {
        d->m_resizeScaleDialogLastKeepAspect = dialog.keepAspectRatio ();

        KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

        cfg.writeEntry (kpSettingResizeScaleLastKeepAspect,
                         d->m_resizeScaleDialogLastKeepAspect);
        cfg.sync ();
    }
}

// public slot
void kpMainWindow::slotCrop ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (!m_document || !m_document->selection ())
    {
        kError () << "kpMainWindow::slotCrop() doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }


    ::kpTransformCrop (this);
}

// private slot
void kpMainWindow::slotAutoCrop ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    ::kpTransformAutoCrop (this);
}

// private slot
void kpMainWindow::slotFlip ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpTransformFlipDialog dialog ((bool) m_document->selection (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpTransformFlipCommand (m_document->selection (),
                                   dialog.getHorizontalFlip (), dialog.getVerticalFlip (),
                                   this));
    }
}


// private slot
void kpMainWindow::slotRotate ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpTransformRotateDialog dialog ((bool) m_document->selection (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpTransformRotateCommand (m_document->selection (),
                                     dialog.angle (),
                                     this));
    }
}

// private slot
void kpMainWindow::slotRotate270 ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    // TODO: Special command name instead of just "Rotate"?
    addImageOrSelectionCommand (
        new kpTransformRotateCommand (
            m_document->selection (),
            270,
            this));
}

// private slot
void kpMainWindow::slotRotate90 ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    // TODO: Special command name instead of just "Rotate"?
    addImageOrSelectionCommand (
        new kpTransformRotateCommand (
            m_document->selection (),
            90,
            this));
}


// private slot
void kpMainWindow::slotSkew ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpTransformSkewDialog dialog ((bool) m_document->selection (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpTransformSkewCommand (m_document->selection (),
                                   dialog.horizontalAngle (), dialog.verticalAngle (),
                                   this));
    }
}

// private slot
void kpMainWindow::slotConvertToBlackAndWhite ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addImageOrSelectionCommand (
        new kpEffectReduceColorsCommand (1/*depth*/, true/*dither*/,
                                         m_document->selection (), this));
}

// private slot
void kpMainWindow::slotConvertToGrayscale ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addImageOrSelectionCommand (
        new kpEffectGrayscaleCommand (m_document->selection (), this));
}

// private slot
void kpMainWindow::slotInvertColors ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addImageOrSelectionCommand (
        new kpEffectInvertCommand (m_document->selection (), this));
}

// private slot
void kpMainWindow::slotClear ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addImageOrSelectionCommand (
        new kpEffectClearCommand (
            m_document->selection (),
            backgroundColor (),
            this));
}

// private slot
void kpMainWindow::slotMoreEffects ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpEffectsDialog dialog ((bool) m_document->selection (), this);
    dialog.selectEffect (d->m_moreEffectsDialogLastEffect);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (dialog.createCommand ());
    }


    if (d->m_moreEffectsDialogLastEffect != dialog.selectedEffect ())
    {
        d->m_moreEffectsDialogLastEffect = dialog.selectedEffect ();

        KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);

        cfg.writeEntry (kpSettingMoreEffectsLastEffect,
                         d->m_moreEffectsDialogLastEffect);
        cfg.sync ();
    }
}
