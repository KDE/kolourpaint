
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

#include <kpmainwindow.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qvaluevector.h>

#include <kaction.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstdaction.h>

#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kpselectiondrag.h>
#include <kptool.h>
#include <kptoolresizescale.h>
#include <kptoolselection.h>
#include <kptooltext.h>
#include <kpview.h>
#include <kpviewmanager.h>


// private
kpPixmapFX::WarnAboutLossInfo kpMainWindow::pasteWarnAboutLossInfo ()
{
    return kpPixmapFX::WarnAboutLossInfo (this,
        i18n ("image to be pasted"),
        "paste");
}


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
    d->m_actionPasteInNewWindow = new KAction (i18n ("Paste in &New Window"),
        Qt::CTRL + Qt::SHIFT + Qt::Key_V,
        this, SLOT (slotPasteInNewWindow ()), ac, "edit_paste_in_new_window");

    //m_actionDelete = KStdAction::clear (this, SLOT (slotDelete ()), ac);
    m_actionDelete = new KAction (i18n ("&Delete Selection"), 0,
        this, SLOT (slotDelete ()), ac, "edit_clear");

    m_actionSelectAll = KStdAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    m_actionDeselect = KStdAction::deselect (this, SLOT (slotDeselect ()), ac);


    d->m_actionCopyToFile = new KAction (i18n ("C&opy to File..."), 0,
        this, SLOT (slotCopyToFile ()), ac, "edit_copy_to_file");
    d->m_actionPasteFromFile = new KAction (i18n ("Paste &From File..."), 0,
        this, SLOT (slotPasteFromFile ()), ac, "edit_paste_from_file");


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
    // m_actionPasteInNewWindow

    // m_actionDelete

    m_actionSelectAll->setEnabled (enable);
    // m_actionDeselect

    m_editMenuDocumentActionsEnabled = enable;

    // m_actionCopyToFile
    d->m_actionPasteFromFile->setEnabled (enable);
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

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

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

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpSelection sel = *m_document->selection ();

    if (sel.isText ())
    {
        QApplication::clipboard ()->setData (new QTextDrag (sel.text ()),
                                             QClipboard::Clipboard);
    }
    else
    {
        if (!sel.pixmap ())
            sel.setPixmap (m_document->getSelectedPixmap ());
        QApplication::clipboard ()->setData (new kpSelectionDrag (sel),
                                             QClipboard::Clipboard);
    }

    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotEnablePaste ()
{
    bool shouldEnable = false;

    QMimeSource *ms = QApplication::clipboard ()->data (QClipboard::Clipboard);
    if (ms)
    {
        shouldEnable = (kpSelectionDrag::canDecode (ms) ||
                        QTextDrag::canDecode (ms));
    }

    d->m_actionPasteInNewWindow->setEnabled (shouldEnable);
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

// private
void kpMainWindow::paste (const kpSelection &sel)
{
    if (!sel.pixmap ())
    {
        kdError () << "kpMainWindow::paste() with sel without pixmap" << endl;
        return;
    }

    QApplication::setOverrideCursor (Qt::waitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


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
        selInUsefulPos.isText () ?
            i18n ("Text: Create Box") :
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
                kpToolResizeScaleCommand::Resize,
                this));
    }


    QApplication::restoreOverrideCursor ();
}

// public
void kpMainWindow::pasteText (const QString &text, bool forceNewTextSelection)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::pasteText(" << text
               << ",forceNewTextSelection=" << forceNewTextSelection
               << ")" << endl;
#endif

    QApplication::setOverrideCursor (Qt::waitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    if (!forceNewTextSelection &&
        m_document && m_document->selection () &&
        m_document->selection ()->isText () &&
        m_commandHistory && m_viewManager)
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\treusing existing Text Selection" << endl;
    #endif
        m_commandHistory->addCommand (new kpToolTextInsertCommand (
            i18n ("Text: Paste"),
            m_viewManager->textCursorRow (), m_viewManager->textCursorCol (),
            text,
            this),
            false/*no exec*/);
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\tcreating Text Selection" << endl;
    #endif

        QValueVector <QString> textLines (1, QString::null);

        for (int i = 0; i < (int) text.length (); i++)
        {
            if (text [i] == '\n')
                textLines.push_back (QString::null);
            else
                textLines [textLines.size () - 1].append (text [i]);
        }


        const kpTextStyle ts = textStyle ();
        const QFontMetrics fontMetrics = ts.fontMetrics ();

        int height = textLines.size () * fontMetrics.height ();
        if (textLines.size () >= 1)
            height += (textLines.size () - 1) * fontMetrics.leading ();

        int width = 0;
        for (QValueVector <QString>::const_iterator it = textLines.begin ();
             it != textLines.end ();
             it++)
        {
            const int w = fontMetrics.width (*it);
            if (w > width)
                width = w;
        }


        kpSelection sel (QRect (0, 0,
                                width + kpSelection::textBorderSize () * 2,
                                height + kpSelection::textBorderSize () * 2),
                         textLines,
                         ts);

        paste (sel);
    }


    QApplication::restoreOverrideCursor ();
}

// public
void kpMainWindow::pasteTextAt (const QString &text, const QPoint &point)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::pasteTextAt(" << text
               << ",point=" << point
               << ")" << endl;
#endif

    QApplication::setOverrideCursor (Qt::waitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    if (m_document &&
        m_document->selection () &&
        m_document->selection ()->isText () &&
        m_document->selection ()->pointIsInTextArea (point))
    {
        kpSelection *sel = m_document->selection ();

        const int row = sel->textRowForPoint (point);
        const int col = sel->textColForPoint (point);

        m_viewManager->setTextCursorPosition (row, col);

        pasteText (text);
    }
    else
    {
        pasteText (text, true/*force new text selection*/);
    }

    QApplication::restoreOverrideCursor ();
}

// public slot
void kpMainWindow::slotPaste ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotPaste() CALLED" << endl;
#endif

    // sync: restoreOverrideCursor() in all exit paths
    QApplication::setOverrideCursor (Qt::waitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


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
    QString text;
    if (kpSelectionDrag::decode (ms, sel/*ref*/, pasteWarnAboutLossInfo ()))
    {
        sel.setTransparency (selectionTransparency ());
        paste (sel);
    }
    else if (QTextDrag::decode (ms, text/*ref*/))
    {
        pasteText (text);
    }
    else
    {
        kdError () << "kpMainWindow::slotPaste() could not decode selection" << endl;
        QApplication::restoreOverrideCursor ();
        return;
    }

    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotPasteInNewWindow ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotPasteInNewWindow() CALLED" << endl;
#endif

    QApplication::setOverrideCursor (Qt::waitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    kpMainWindow *win = new kpMainWindow (0/*no document*/);
    win->show ();

    win->slotPaste ();


    QApplication::restoreOverrideCursor ();
}

// public slot
void kpMainWindow::slotDelete ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::slotDelete() CALLED" << endl;
#endif
    if (!m_actionDelete->isEnabled ())
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kdDebug () << "\taction not enabled - was probably called from kpTool::keyPressEvent()" << endl;
    #endif
        return;
    }

    if (!m_document || !m_document->selection ())
    {
        kdError () << "kpMainWindow::slotDelete () doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addImageOrSelectionCommand (new kpToolSelectionDestroyCommand (
        m_document->selection ()->isText () ?
            i18n ("Text: Delete Box") :  // not to be confused with i18n ("Text: Delete")
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

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    if (m_document->selection ())
        slotDeselect ();

    // just the border - don't actually pull pixmap from doc yet
    m_document->setSelection (kpSelection (kpSelection::Rectangle, m_document->rect (), selectionTransparency ()));

    if (tool ())
        tool ()->somethingBelowTheCursorChanged ();
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
            if (tool ())
                tool ()->somethingBelowTheCursorChanged ();

            if (cmd)
                m_commandHistory->addCommand (cmd);
        }
        else
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kdDebug () << "\treal selection with pixmap - push onto doc cmd" << endl;
        #endif
            KCommand *deselectCommand = new kpToolSelectionDestroyCommand (
                sel->isText () ?
                    i18n ("Text: Finish") :
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

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    addDeselectFirstCommand (0);
}


// private slot
void kpMainWindow::slotCopyToFile ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotCopyToFile()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    QString chosenMimeType;
    KURL chosenURL = askForSaveURL (i18n ("Copy to File"),
                                    d->m_lastCopyToURL.url (),
                                    d->m_lastCopyToMimeType,
                                    "Edit/Copy To File",
                                    false/*allow remote files*/,
                                    chosenMimeType/*ref*/);

    if (chosenURL.isEmpty () || chosenMimeType.isEmpty ())
        return;


    d->m_lastCopyToURL = chosenURL;
    d->m_lastCopyToMimeType = chosenMimeType;
    saveLastOutputMimeType (chosenMimeType, "Edit/Copy To File");


    if (!kpDocument::savePixmapToFile (m_document->getSelectedPixmap (),
                                       chosenURL, chosenMimeType,
                                       true/*overwrite prompt*/,
                                       this))
    {
        return;
    }


    addRecentURL (chosenURL);
}

// private slot
void kpMainWindow::slotPasteFromFile ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotPasteFromFile()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    KURL::List urls = askForOpenURLs (i18n ("Paste From File"),
                                      d->m_lastPasteFromURL.url (),
                                      false/*only 1 URL*/);

    if (urls.count () != 1)
        return;

    KURL url = urls.first ();
    d->m_lastPasteFromURL = url;


    QPixmap pixmap = kpDocument::getPixmapFromFile (url,
        false/*show error message if doesn't exist*/,
        this);


    if (pixmap.isNull ())
        return;


    addRecentURL (url);

    paste (kpSelection (kpSelection::Rectangle,
                        QRect (0, 0, pixmap.width (), pixmap.height ()),
                        pixmap));
}

