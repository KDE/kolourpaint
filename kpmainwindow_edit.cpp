
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

#include <qapplication.h>
#include <qclipboard.h>
#include <qpixmap.h>

#include <kaction.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstdaction.h>

#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpviewmanager.h>


// private
void kpMainWindow::setupEditMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    // Undo/Redo
    m_commandHistory = new KCommandHistory (ac, true);
    m_commandHistory->setUndoLimit (5);  // CONFIG
    
    /*m_actionUndo = new KStdAction::undo (this, SLOT (slotUndo ()), ac);
    m_actionRedo = new KStdAction::redo (this, SLOT (slotRedo ()), ac);*/
    
    m_actionCut = KStdAction::cut (this, SLOT (slotCut ()), ac);
    m_actionCopy = KStdAction::copy (this, SLOT (slotCopy ()), ac);
    m_actionPaste = KStdAction::paste (this, SLOT (slotPaste ()), ac);
    m_actionDelete = KStdAction::clear (this, SLOT (slotDelete ()), ac);
    /*new KAction (i18n ("&Delete"), Key_Delete,
        this, SLOT (slotDelete ()), ac, "edit_delete");*/

    m_actionSelectAll = KStdAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    m_actionDeselect = KStdAction::deselect (this, SLOT (slotDeselect ()), ac);

    m_actionCut->setEnabled (false);
    m_actionCopy->setEnabled (false);
}


// private slot
void kpMainWindow::slotUndo ()
{
}

// private slot
void kpMainWindow::slotRedo ()
{
}


// private slot
void kpMainWindow::slotCut ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    if (m_viewManager->selectionActive ())
    {
        QApplication::setOverrideCursor (Qt::waitCursor);
        QApplication::clipboard ()->setPixmap (m_viewManager->tempPixmap (), QClipboard::Clipboard);
        m_viewManager->invalidateTempPixmap ();
        QApplication::restoreOverrideCursor ();
    }
}

// private slot
void kpMainWindow::slotCopy ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    if (m_viewManager->selectionActive ())
    {
        QApplication::setOverrideCursor (Qt::waitCursor);
        QApplication::clipboard ()->setPixmap (m_viewManager->tempPixmap (), QClipboard::Clipboard);
        QApplication::restoreOverrideCursor ();
    }
}

// private slot
void kpMainWindow::slotPaste ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    QApplication::setOverrideCursor (Qt::waitCursor);

    QPixmap pixmap = QApplication::clipboard ()->pixmap (QClipboard::Clipboard);

    if (!pixmap.isNull ())
    {
        m_viewManager->setTempPixmapAt (pixmap, QPoint (0, 0), kpViewManager::SelectionPixmap);
        slotToolRectSelection ();
    }

    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotDelete ()
{
}


// private slot
void kpMainWindow::slotSelectAll ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    m_viewManager->setTempPixmapAt (*m_document->pixmap (), QPoint (0, 0), kpViewManager::SelectionPixmap);
    slotToolRectSelection ();
}

// private slot
void kpMainWindow::slotDeselect ()
{
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;

    if (m_viewManager->selectionActive ())
        m_viewManager->invalidateTempPixmap ();
}
