
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
#include <qimage.h>
#include <qpixmap.h>

#include <kaction.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstdaction.h>

#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptool.h>
#include <kptoolresizescale.h>
#include <kpviewmanager.h>


// private
void kpMainWindow::setupEditMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    // Undo/Redo
    m_commandHistory = new kpCommandHistory (this);
    m_commandHistory->setUndoLimit (5);  // CONFIG
    
    m_actionCut = KStdAction::cut (this, SLOT (slotCut ()), ac);
    m_actionCopy = KStdAction::copy (this, SLOT (slotCopy ()), ac);
    m_actionPaste = KStdAction::paste (this, SLOT (slotPaste ()), ac);
    
    //m_actionDelete = KStdAction::clear (this, SLOT (slotDelete ()), ac);
    m_actionDelete = new KAction (i18n ("Clear &Selection"), Key_Delete,
        this, SLOT (slotDelete ()), ac, "edit_clear");

    m_actionSelectAll = KStdAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    m_actionDeselect = KStdAction::deselect (this, SLOT (slotDeselect ()), ac);

    m_editMenuDocumentActionsEnabled = false;
    enableEditMenuDocumentActions (false);

    // Paste should always be enabled, as long as there is something paste
    // (independent of whether we have a document or not)
    connect (QApplication::clipboard (), SIGNAL (dataChanged ()),
             this, SLOT (slotEnablePaste ()));
    slotEnablePaste ();
}

// private
void kpMainWindow::enableEditMenuDocumentActions (bool enable)
{
    // m_actionCut
    // m_actionCopy
    // m_actionPaste
    
    // m_actionDelete
    
    m_actionSelectAll->setEnabled (enable);
    // m_actionDeselect
    
    m_editMenuDocumentActionsEnabled = enable;
}


// private
bool kpMainWindow::checkHasSelectionActive (const char *funcName) const
{
    if (m_viewManager && m_viewManager->selectionActive ())
        return true;
        
    kdError () << "kpMainWindow::checkHasSelectionActive("
               << "funcName=" << funcName
               << ") vm="
               << m_viewManager
               << endl;

    return false;
}

// private
bool kpMainWindow::checkHasDocument (const char *funcName) const
{
    if (m_document)
        return true;
        
    kdError () << "kpMainWindow::checkHasDocument("
               << "funcName=" << funcName
               << ") no doc"
               << endl;

    return false;
}

// private
bool kpMainWindow::checkHasViewManager (const char *funcName) const
{
    if (m_viewManager)
        return true;
    
    kdError () << "kpMainWindow::checkHasViewManager("
               << "funcName=" << funcName
               << ") no vm"
               << endl;
    
    return false;
}


// private slot
void kpMainWindow::slotCut ()
{
    if (!checkHasSelectionActive ("slotCut"))
        return;
        
    QApplication::setOverrideCursor (Qt::waitCursor);
    QApplication::clipboard ()->setPixmap (m_viewManager->tempPixmap (),
                                           QClipboard::Clipboard);
    m_viewManager->invalidateTempPixmap ();
    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotCopy ()
{
    if (!checkHasSelectionActive ("slotCopy"))
        return;
        
    QApplication::setOverrideCursor (Qt::waitCursor);
    QApplication::clipboard ()->setPixmap (m_viewManager->tempPixmap (), QClipboard::Clipboard);
    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotEnablePaste ()
{
    // no need to convert the clipboard to a pixmap
    // - just need to know if an image is there
    QImage image = QApplication::clipboard ()->image (QClipboard::Clipboard);
    m_actionPaste->setEnabled (!image.isNull ());
}

// private slot
void kpMainWindow::slotPaste ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    QApplication::setOverrideCursor (Qt::waitCursor);

    // TODO: should this be pretty?
    QPixmap pixmap = kpPixmapFX::convertToPixmap (QApplication::clipboard ()->image (QClipboard::Clipboard));

    if (!pixmap.isNull ())
    {
        if (!m_document)
        {
            kpDocument *newDoc = new kpDocument (
                pixmap.width (), pixmap.height (), 32, this);
                
            // will also create viewManager
            setDocument (newDoc);
        }
            
        if (checkHasViewManager ("slotPaste"))
        {
            m_viewManager->setTempPixmapAt (pixmap, QPoint (0, 0), kpViewManager::SelectionPixmap);
            slotToolRectSelection ();

            // If the selection is bigger than the document, automatically
            // resize the document (with the option of Undo'ing) to fit
            // the selection.
            // 
            // No annoying dialog necessary.
            // 
            if (pixmap.width () > m_document->width () ||
                pixmap.height () > m_document->height ())
            {
                m_commandHistory->addCommand (
                    new kpToolResizeScaleCommand (
                        m_document, m_viewManager,
                        QMAX (pixmap.width (), m_document->width ()),
                        QMAX (pixmap.height (), m_document->height ()),
                        false/*no scale*/,
                        backgroundColor ()));
            }
        }
    }
    else
    {
        kdError () << "kpMainWindow::slotPaste() null pixmap" << endl;
    }

    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotDelete ()
{
    if (!checkHasSelectionActive ("slotDelete"))
        return;
}


// private slot
void kpMainWindow::slotSelectAll ()
{
    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (!checkHasViewManager ("slotSelectAll") ||
        !checkHasDocument ("slotSelectAll"))
    {
        return;
    }

    m_viewManager->setTempPixmapAt (*m_document->pixmap (),
                                    QPoint (0, 0),
                                    kpViewManager::SelectionPixmap);
    slotToolRectSelection ();
}

// private slot
void kpMainWindow::slotDeselect ()
{
    if (!checkHasSelectionActive ("slotDeselect"))
        return;
        
    m_viewManager->invalidateTempPixmap ();
}
