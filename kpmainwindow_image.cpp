
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <qcolor.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenubar.h>

#include <kpcolortoolbar.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptoolautocrop.h>
#include <kptoolclear.h>
#include <kptoolconverttograyscale.h>
#include <kptoolconverttoblackandwhite.h>
#include <kptoolflip.h>
#include <kptoolinvertcolors.h>
#include <kptoolresizescale.h>
#include <kptoolrotate.h>
#include <kptoolselection.h>
#include <kptoolskew.h>
#include <kpviewmanager.h>


// private
void kpMainWindow::setupImageMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionResizeScale = new KAction (i18n ("R&esize / Scale..."), CTRL + Key_E,
        this, SLOT (slotResizeScale ()), ac, "image_resize_scale");

    m_actionCrop = new KAction (i18n ("C&rop outside the selection"), CTRL + Key_R,
        this, SLOT (slotCrop ()), ac, "image_crop");

    m_actionAutoCrop = new KAction (i18n ("A&utocrop"), CTRL + Key_U,
        this, SLOT (slotAutoCrop ()), ac, "image_auto_crop");

    m_actionFlip = new KAction (i18n ("&Flip..."), CTRL + Key_F,
        this, SLOT (slotFlip ()), ac, "image_flip");

    m_actionRotate = new KAction (i18n ("R&otate..."), 0,
        this, SLOT (slotRotate ()), ac, "image_rotate");

    m_actionSkew = new KAction (i18n ("&Skew..."), 0,
        this, SLOT (slotSkew ()), ac, "image_skew");

    m_actionConvertToBlackAndWhite = new KAction (i18n ("Convert to &Black && White"), 0,
        this, SLOT (slotConvertToBlackAndWhite ()), ac, "image_convert_to_black_and_white");

    m_actionConvertToGrayscale = new KAction (i18n ("Convert to &Grayscale"), 0,
        this, SLOT (slotConvertToGrayscale ()), ac, "image_convert_to_grayscale");

    m_actionInvertColors = new KAction (i18n ("&Invert Colors"), CTRL + Key_I,
        this, SLOT (slotInvertColors ()), ac, "image_invert_colors");

    m_actionClear = new KAction (i18n ("C&lear"), CTRL + SHIFT + Key_N,
        this, SLOT (slotClear ()), ac, "image_clear");

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
}


// private slot
void kpMainWindow::slotImageMenuUpdateDueToSelection ()
{
    bool isSelectionEnabled = m_document ? m_document->selection () : false;


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
            if (isSelectionEnabled)
                mBar->changeItem (id, menuBarItemTextSelection);
            else
                mBar->changeItem (id, menuBarItemTextImage);

            break;
        }
    }


    // (any action other than Image/Crop will do)
    bool imageMenuDocumentActionsEnabled = m_actionClear->isEnabled ();

    // Image/Crop available only with active selection
    m_actionCrop->setEnabled (imageMenuDocumentActionsEnabled &&
                              isSelectionEnabled);
}


// public
QColor kpMainWindow::backgroundColor () const
{
    if (m_colorToolBar)
        return m_colorToolBar->backgroundColor ();
    else
        return QColor ();  // transparent
}


// public
void kpMainWindow::addImageOrSelectionCommand (KCommand *cmd, bool actOnSelectionIfAvail)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::addImageOrSelectionCommand() actOnSelectionIfAvail="
               << actOnSelectionIfAvail
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
    if (actOnSelectionIfAvail && sel && !sel->pixmap ())
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\tmust pull selection from doc" << endl;
    #endif

        // create selection region
        m_commandHistory->addCommand (new kpToolSelectionCreateCommand (
            i18n ("Create selection"),
            *sel,
            this),
            false/*no exec - user already dragged out sel*/);

    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\t\tsel=" << sel
                << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
                << endl;
    #endif


        KMacroCommand *macroCmd = new KMacroCommand (cmd->name ());

        macroCmd->addCommand (new kpToolSelectionPullFromDocumentCommand (
            QString::null/*uninteresting child of macro cmd*/,
            this));

        macroCmd->addCommand (cmd);

        m_commandHistory->addCommand (macroCmd);

    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\t\tsel=" << sel
               << " sel->pixmap=" << (sel ? sel->pixmap () : 0)
               << endl;
    #endif
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\tjust add cmd" << endl;
    #endif

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

    kpToolResizeScaleDialog *dialog = new kpToolResizeScaleDialog (this);

    if (dialog->exec () && !dialog->isNoop ())
    {
        addImageOrSelectionCommand (
            new kpToolResizeScaleCommand (
                m_document->selection (),
                dialog->imageWidth (), dialog->imageHeight (),
                dialog->scaleToFit (),
                this));
    }
}

// private slot
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

    kpSelection *sel = m_document->selection ();


    KMacroCommand *macroCmd = new KMacroCommand (i18n ("Crop outside the selection"));

    macroCmd->addCommand (
        new kpToolResizeScaleCommand (
            false/*act on doc, not sel*/,
            sel->width (), sel->height (),
            false/*resize, not scale*/,
            this));

    macroCmd->addCommand (
        new kpToolClearCommand (
            false/*act on doc, not sel*/,
            this));

    kpToolSelectionMoveCommand *moveCmd =
        new kpToolSelectionMoveCommand (
            QString::null/*uninteresting child of macro cmd*/,
            this);
    moveCmd->moveTo (QPoint (0, 0), true/*move on exec, not now*/);
    moveCmd->finalize ();
    macroCmd->addCommand (moveCmd);

    addImageOrSelectionCommand (macroCmd);
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

    kpToolFlipDialog *dialog = new kpToolFlipDialog (this);

    if (dialog->exec () && !dialog->isNoopFlip ())
    {
        addImageOrSelectionCommand (
            new kpToolFlipCommand (m_document->selection (),
                                   dialog->getHorizontalFlip (), dialog->getVerticalFlip (),
                                   this));
    }
}

// private slot
void kpMainWindow::slotRotate ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpToolRotateDialog *dialog = new kpToolRotateDialog (this);

    if (dialog->exec () && !dialog->isNoopRotate ())
    {
        addImageOrSelectionCommand (
            new kpToolRotateCommand (m_document->selection (),
                                     dialog->angle (),
                                     this));
    }
}

// private slot
void kpMainWindow::slotSkew ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpToolSkewDialog *dialog = new kpToolSkewDialog (this);

    if (dialog->exec () && !dialog->isNoop ())
    {
        addImageOrSelectionCommand (
            new kpToolSkewCommand (m_document->selection (),
                                   dialog->horizontalAngle (), dialog->verticalAngle (),
                                   this));
    }
}

// private slot
void kpMainWindow::slotConvertToBlackAndWhite ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addImageOrSelectionCommand (
        new kpToolConvertToBlackAndWhiteCommand (m_document->selection (), this));
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
        new kpToolInvertColorsCommand (m_document->selection (), this));
}

// private slot
void kpMainWindow::slotClear ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "toolHasBegunShape: " << toolHasBegunShape () << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotClear ()" << endl;
#endif

    addImageOrSelectionCommand (
        new kpToolClearCommand (m_document->selection (), this));
}


