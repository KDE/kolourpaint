
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
#include <kpselection.h>
#include <kpselectiondrag.h>
#include <kptool.h>
#include <kptoolresizescale.h>
#include <kptoolselection.h>
#include <kpview.h>
#include <kpviewmanager.h>


// private
void kpMainWindow::setupEditMenuActions ()
{
    KActionCollection *ac = actionCollection ();

    // Undo/Redo
    m_commandHistory = new kpCommandHistory (this);
    m_commandHistory->setUndoLimit (10);  // CONFIG

    m_actionCut = KStdAction::cut (this, SLOT (slotCut ()), ac);
    m_actionCopy = KStdAction::copy (this, SLOT (slotCopy ()), ac);
    m_actionPaste = KStdAction::paste (this, SLOT (slotPaste ()), ac);

    //m_actionDelete = KStdAction::clear (this, SLOT (slotDelete ()), ac);
    m_actionDelete = new KAction (i18n ("&Delete"), Key_Delete,
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


// private slot
void kpMainWindow::slotCut ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotCut() CALLED" << endl;
#endif
    if (!m_document || !m_document->selection ())
    {
        kdError () << "kpMainWindow::slotCut () doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }


    QApplication::setOverrideCursor (Qt::waitCursor);

    slotCopy ();
    slotDelete ();

    QApplication::restoreOverrideCursor ();

}

// private slot
void kpMainWindow::slotCopy ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotCopy() CALLED" << endl;
#endif
    if (!m_document || !m_document->selection ())
    {
        kdError () << "kpMainWindow::slotCopy () doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }

    QApplication::setOverrideCursor (Qt::waitCursor);
    kpSelection sel = *m_document->selection ();
    if (!sel.pixmap ())
        sel.setPixmap (m_document->getSelectedPixmap ());
    QApplication::clipboard ()->setData (new kpSelectionDrag (sel),
                                         QClipboard::Clipboard);
    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotEnablePaste ()
{
    bool shouldEnable = false;

    QMimeSource *ms = QApplication::clipboard ()->data (QClipboard::Clipboard);
    if (ms)
        shouldEnable = kpSelectionDrag::canDecode (ms);

    m_actionPaste->setEnabled (shouldEnable);
}


// private
QRect kpMainWindow::calcUsefulPasteRect (int pixmapWidth, int pixmapHeight)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::calcUsefulPasteRect("
               << pixmapWidth << "," << pixmapHeight
               << ")"
               << endl;
#endif
    if (!m_document)
    {
        kdError () << "kpMainWindow::calcUsefulPasteRect() without doc" << endl;
        return QRect ();
    }

    // TODO: 1st choice is to paste sel near but not overlapping last deselect point

    if (m_mainView && m_scrollView)
    {
        const QPoint viewTopLeft (m_scrollView->contentsX (),
                                  m_scrollView->contentsY ());

        const QPoint docTopLeft = m_mainView->zoomViewToDoc (viewTopLeft);

        if ((docTopLeft.x () + pixmapWidth <= m_document->width () &&
             docTopLeft.y () + pixmapHeight <= m_document->height ()) ||
            pixmapWidth <= docTopLeft.x () ||
            pixmapHeight <= docTopLeft.y ())
        {
            return QRect (docTopLeft.x (), docTopLeft.y (),
                          pixmapWidth, pixmapHeight);
        }
    }

    return QRect (0, 0, pixmapWidth, pixmapHeight);
}

// private slot
void kpMainWindow::paste (const kpSelection &sel)
{
    if (!sel.pixmap ())
    {
        kdError () << "kpMainWindow::paste() with sel without pixmap" << endl;
        return;
    }

    QApplication::setOverrideCursor (Qt::waitCursor);


    //
    // Make sure we've got a document (esp. with File/Close)
    //

    if (!m_document)
    {
        kpDocument *newDoc = new kpDocument (
            sel.width (), sel.height (), 32, this);

        // will also create viewManager
        setDocument (newDoc);
    }


    //
    // Paste as new selection
    //

    kpSelection selInUsefulPos = sel;
    selInUsefulPos.moveTo (calcUsefulPasteRect (sel.width (), sel.height ()).topLeft ());
    addDeselectFirstCommand (new kpToolSelectionCreateCommand (
        i18n ("Selection: Create"),
        selInUsefulPos,
        this));


    // If the selection is bigger than the document, automatically
    // resize the document (with the option of Undo'ing) to fit
    // the selection.
    //
    // No annoying dialog necessary.
    //
    if (sel.width () > m_document->width () ||
        sel.height () > m_document->height ())
    {
        m_commandHistory->addCommand (
            new kpToolResizeScaleCommand (
                false/*act on doc, not sel*/,
                QMAX (sel.width (), m_document->width ()),
                QMAX (sel.height (), m_document->height ()),
                false/*no scale*/,
                this));
    }


    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotPaste ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotPaste() CALLED" << endl;
#endif

    // sync: restoreOverrideCursor() in all exit paths
    QApplication::setOverrideCursor (Qt::waitCursor);


    //
    // Acquire the pixmap
    //

    QMimeSource *ms = QApplication::clipboard ()->data (QClipboard::Clipboard);
    if (!ms)
    {
        kdError () << "kpMainWindow::slotPaste() without mimeSource" << endl;
        QApplication::restoreOverrideCursor ();
        return;
    }

    kpSelection sel;
    if (!kpSelectionDrag::decode (ms, sel/*ref*/))
    {
        kdError () << "kpMainWindow::slotPaste() could not decode selection" << endl;
        QApplication::restoreOverrideCursor ();
        return;
    }


    paste (sel);


    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotDelete ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotDelete() CALLED" << endl;
#endif
    if (!m_document || !m_document->selection ())
    {
        kdError () << "kpMainWindow::slotDelete () doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }

    addImageOrSelectionCommand (new kpToolSelectionDestroyCommand (
        i18n ("Selection: Delete"),
        false/*no push onto doc*/,
        this));
}


// private slot
void kpMainWindow::slotSelectAll ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotSelectAll() CALLED" << endl;
#endif
    if (!m_document)
    {
        kdError () << "kpMainWindow::slotSelectAll() without doc" << endl;
        return;
    }

    if (m_document->selection ())
        slotDeselect ();

    // just the border - don't actually pull pixmap from doc yet
    m_document->setSelection (kpSelection (kpSelection::Rectangle, m_document->rect ()));
}


// private
void kpMainWindow::addDeselectFirstCommand (KCommand *cmd)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::addDeselectFirstCommand("
               << cmd
               << ")"
               << endl;
#endif


    kpSelection *sel = m_document->selection ();

#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "\tsel=" << sel << endl;
#endif

    if (sel)
    {
        // if you just dragged out something with no action then
        // forget the drag
        if (!sel->pixmap ())
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kdDebug () << "\tjust a fresh border - was nop - delete" << endl;
        #endif
            m_document->selectionDelete ();
            if (cmd)
                m_commandHistory->addCommand (cmd);
        }
        else
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kdDebug () << "\treal selection with pixmap - push onto doc cmd" << endl;
        #endif
            KCommand *deselectCommand = new kpToolSelectionDestroyCommand (
                i18n ("Selection: Deselect"),
                true/*push onto document*/,
                this);

            if (cmd)
            {
                KMacroCommand *macroCmd = new KMacroCommand (cmd->name ());
                macroCmd->addCommand (deselectCommand);
                macroCmd->addCommand (cmd);
                m_commandHistory->addCommand (macroCmd);
            }
            else
                m_commandHistory->addCommand (deselectCommand);
        }
    }
    else
    {
        if (cmd)
            m_commandHistory->addCommand (cmd);
    }
}


// public slot
void kpMainWindow::slotDeselect ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotDeselect() CALLED" << endl;
#endif
    if (!m_document || !m_document->selection ())
    {
        kdError () << "kpMainWindow::slotDeselect() doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }

    addDeselectFirstCommand (0);
}
