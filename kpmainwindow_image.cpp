
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include <kpcolortoolbar.h>
#include <kpmainwindow.h>
#include <kptool.h>
#include <kptoolautocrop.h>
#include <kptoolclear.h>
#include <kptoolconverttograyscale.h>
#include <kptoolconverttoblackandwhite.h>
#include <kptoolflip.h>
#include <kptoolinvertcolors.h>
#include <kptoolresizescale.h>
#include <kptoolrotate.h>
#include <kptoolskew.h>


// private
void kpMainWindow::setupImageMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionResizeScale = new KAction (i18n ("&Resize / Scale..."), CTRL + Key_R,
        this, SLOT (slotResizeScale ()), ac, "image_resize_scale");

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

    m_actionClear = new KAction (i18n ("&Clear"), CTRL + SHIFT + Key_N,
        this, SLOT (slotClear ()), ac, "image_clear");

    enableImageMenuDocumentActions (false);
}

// private
void kpMainWindow::enableImageMenuDocumentActions (bool enable)
{
    m_actionResizeScale->setEnabled (enable);
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
void kpMainWindow::slotResizeScale ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpToolResizeScaleDialog *dialog = new kpToolResizeScaleDialog (this);

    if (dialog->exec () && !dialog->isNoop ())
    {
        m_commandHistory->addCommand (
            new kpToolResizeScaleCommand (m_document, m_viewManager,
                                          dialog->imageWidth (), dialog->imageHeight (),
                                          dialog->scaleToFit ()));
    }
}

// private slot
void kpMainWindow::slotAutoCrop ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();
        
    kpToolAutoCrop (this);
}

// private slot
void kpMainWindow::slotFlip ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();
    
    kpToolFlipDialog *dialog = new kpToolFlipDialog (this);

    if (dialog->exec () && !dialog->isNoopFlip ())
    {
        m_commandHistory->addCommand (
            new kpToolFlipCommand (m_document, m_viewManager,
                                   dialog->getHorizontalFlip (), dialog->getVerticalFlip ()));
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
        m_commandHistory->addCommand (
            new kpToolRotateCommand (m_document, m_viewManager, dialog->angle ()));
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
        m_commandHistory->addCommand (
            new kpToolSkewCommand (m_document, m_viewManager,
                                   dialog->horizontalAngle (), dialog->verticalAngle ()));
    }
}

// private slot
void kpMainWindow::slotConvertToBlackAndWhite ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    m_commandHistory->addCommand (
        new kpToolConvertToBlackAndWhiteCommand (m_document, m_viewManager));
}

// private slot
void kpMainWindow::slotConvertToGrayscale ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    m_commandHistory->addCommand (
        new kpToolConvertToGrayscaleCommand (m_document, m_viewManager));
}

// private slot
void kpMainWindow::slotInvertColors ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    m_commandHistory->addCommand (
        new kpToolInvertColorsCommand (m_document, m_viewManager));
}

// private slot
void kpMainWindow::slotClear ()
{
#if DEBUG_KPMAINWINDOW && 1
    kdDebug () << "toolHasBegunShape: " << toolHasBegunShape () << endl;
#endif
    
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::slotClear ()" << endl;
    kdDebug () << "\ttoolHasBegunShape now: " << toolHasBegunShape () << endl;
#endif

    m_commandHistory->addCommand (
        new kpToolClearCommand (m_document, m_viewManager,
                                colorToolBar ()->backgroundColor ()));
}

