
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kpmainwindow.h>
#include <kpmainwindow_p.h>

#include <qcolor.h>
#include <qsize.h>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenubar.h>

#include <kpcolor.h>
#include <kpdefs.h>
#include <kpcoloreffect.h>
#include <kpcolortoolbar.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpeffectinvert.h>
#include <kpeffectreducecolors.h>
#include <kpeffectsdialog.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptoolautocrop.h>
#include <kptoolclear.h>
#include <kptoolconverttograyscale.h>
#include <kptoolcrop.h>
#include <kptoolflip.h>
#include <kptoolresizescale.h>
#include <kptoolrotate.h>
#include <kptoolselection.h>
#include <kptoolskew.h>
#include <kpviewmanager.h>


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
    return kpToolAutoCropCommand::name (isSelectionActive (),
                                        kpToolAutoCropCommand::ShowAccel);
}


// private
void kpMainWindow::setupImageMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionResizeScale = new KAction (i18n ("R&esize / Scale..."), Qt::CTRL + Qt::Key_E,
        this, SLOT (slotResizeScale ()), ac, "image_resize_scale");

    m_actionCrop = new KAction (i18n ("Se&t as Image (Crop)"), Qt::CTRL + Qt::Key_T,
        this, SLOT (slotCrop ()), ac, "image_crop");

    m_actionAutoCrop = new KAction (autoCropText (), Qt::CTRL + Qt::Key_U,
        this, SLOT (slotAutoCrop ()), ac, "image_auto_crop");

    m_actionFlip = new KAction (i18n ("&Flip..."), Qt::CTRL + Qt::Key_F,
        this, SLOT (slotFlip ()), ac, "image_flip");

    m_actionRotate = new KAction (i18n ("&Rotate..."), Qt::CTRL + Qt::Key_R,
        this, SLOT (slotRotate ()), ac, "image_rotate");

    m_actionSkew = new KAction (i18n ("S&kew..."), Qt::CTRL + Qt::Key_K,
        this, SLOT (slotSkew ()), ac, "image_skew");

    m_actionConvertToBlackAndWhite = new KAction (i18n ("Reduce to Mo&nochrome (Dithered)"), 0,
        this, SLOT (slotConvertToBlackAndWhite ()), ac, "image_convert_to_black_and_white");

    m_actionConvertToGrayscale = new KAction (i18n ("Reduce to &Grayscale"), 0,
        this, SLOT (slotConvertToGrayscale ()), ac, "image_convert_to_grayscale");

    m_actionInvertColors = new KAction (i18n ("&Invert Colors"), Qt::CTRL + Qt::Key_I,
        this, SLOT (slotInvertColors ()), ac, "image_invert_colors");

    m_actionClear = new KAction (i18n ("C&lear"), Qt::CTRL + Qt::SHIFT + Qt::Key_N,
        this, SLOT (slotClear ()), ac, "image_clear");

    m_actionMoreEffects = new KAction (i18n ("&More Effects..."), Qt::CTRL + Qt::Key_M,
        this, SLOT (slotMoreEffects ()), ac, "image_more_effects");

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
    KMenuBar *mBar = menuBar ();
    if (!mBar)  // just in case
        return;

    int mBarNumItems = (int) mBar->count ();
    for (int index = 0; index < mBarNumItems; index++)
    {
        int id = mBar->idAt (index);

        // SYNC: kolourpaintui.rc
        QString menuBarItemTextImage = i18n ("&Image");
        QString menuBarItemTextSelection = i18n ("Select&ion");

        const QString menuBarItemText = mBar->text (id);
        if (menuBarItemText == menuBarItemTextImage ||
            menuBarItemText == menuBarItemTextSelection)
        {
            if (isSelectionActive ())
                mBar->changeItem (id, menuBarItemTextSelection);
            else
                mBar->changeItem (id, menuBarItemTextImage);

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
        return kpColor::transparent;
    else
    {
        if (m_colorToolBar)
            return m_colorToolBar->backgroundColor ();
        else
        {
            kdError () << "kpMainWindow::backgroundColor() without colorToolBar" << endl;
            return kpColor::invalid;
        }
    }
}


// public
void kpMainWindow::addImageOrSelectionCommand (kpCommand *cmd,
    bool addSelCreateCmdIfSelAvail,
    bool addSelPullCmdIfSelAvail)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::addImageOrSelectionCommand()"
               << " addSelCreateCmdIfSelAvail=" << addSelCreateCmdIfSelAvail
               << " addSelPullCmdIfSelAvail=" << addSelPullCmdIfSelAvail
               << endl;
#endif

    if (!m_document)
    {
        kdError () << "kpMainWindow::addImageOrSelectionCommand() without doc" << endl;
        return;
    }


    if (m_viewManager)
        m_viewManager->setQueueUpdates ();


    kpSelection *sel = m_document->selection ();
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "\tsel=" << sel
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

    kpToolResizeScaleDialog dialog (this);
    dialog.setKeepAspectRatio (d->m_resizeScaleDialogLastKeepAspect);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        kpToolResizeScaleCommand *cmd = new kpToolResizeScaleCommand (
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
            dialog.type () == kpToolResizeScaleCommand::Resize)
        {
            // TODO: this should be the responsibility of kpDocument
            saveDefaultDocSize (QSize (dialog.imageWidth (), dialog.imageHeight ()));
        }
    }


    if (d->m_resizeScaleDialogLastKeepAspect != dialog.keepAspectRatio ())
    {
        d->m_resizeScaleDialogLastKeepAspect = dialog.keepAspectRatio ();

        KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
        KConfigBase *cfg = cfgGroupSaver.config ();

        cfg->writeEntry (kpSettingResizeScaleLastKeepAspect,
                         d->m_resizeScaleDialogLastKeepAspect);
        cfg->sync ();
    }
}

// public slot
void kpMainWindow::slotCrop ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (!m_document || !m_document->selection ())
    {
        kdError () << "kpMainWindow::slotCrop() doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }


    ::kpToolCrop (this);
}

// private slot
void kpMainWindow::slotAutoCrop ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    ::kpToolAutoCrop (this);
}

// private slot
void kpMainWindow::slotFlip ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpToolFlipDialog dialog ((bool) m_document->selection (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpToolFlipCommand (m_document->selection (),
                                   dialog.getHorizontalFlip (), dialog.getVerticalFlip (),
                                   this));
    }
}

// private slot
void kpMainWindow::slotRotate ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpToolRotateDialog dialog ((bool) m_document->selection (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpToolRotateCommand (m_document->selection (),
                                     dialog.angle (),
                                     this));
    }
}

// private slot
void kpMainWindow::slotSkew ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpToolSkewDialog dialog ((bool) m_document->selection (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        addImageOrSelectionCommand (
            new kpToolSkewCommand (m_document->selection (),
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
        new kpToolConvertToGrayscaleCommand (m_document->selection (), this));
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
        new kpToolClearCommand (m_document->selection (), this));
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

        KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupGeneral);
        KConfigBase *cfg = cfgGroupSaver.config ();

        cfg->writeEntry (kpSettingMoreEffectsLastEffect,
                         d->m_moreEffectsDialogLastEffect);
        cfg->sync ();
    }
}
