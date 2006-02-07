
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

#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qpixmap.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <Q3PtrList>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>

#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpdocumentmetainfo.h>
#include <kpdocumentsaveoptions.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kpselectiondrag.h>
#include <kptool.h>
#include <kptoolcrop.h>
#include <kptoolresizescale.h>
#include <kptoolselection.h>
#include <kptooltext.h>
#include <kpviewmanager.h>
#include <kpviewscrollablecontainer.h>
#include <kpzoomedview.h>


// private
kpPixmapFX::WarnAboutLossInfo kpMainWindow::pasteWarnAboutLossInfo ()
{
    return kpPixmapFX::WarnAboutLossInfo (
               i18n ("The image to be pasted"
                     " may have more colors than the current screen mode."
                     " In order to display it, some colors may be changed."
                     " Try increasing your screen depth to at least %1bpp."

                     "\nIt also"

                     " contains translucency which is not fully"
                     " supported. The translucency data will be"
                     " approximated with a 1-bit transparency mask."),
               i18n ("The image to be pasted"
                     " may have more colors than the current screen mode."
                     " In order to display it, some colors may be changed."
                     " Try increasing your screen depth to at least %1bpp."),
               i18n ("The image to be pasted"
                     " contains translucency which is not fully"
                     " supported. The translucency data will be"
                     " approximated with a 1-bit transparency mask."),
               "paste",
               this);
}


// private
void kpMainWindow::setupEditMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    // Undo/Redo
    // CONFIG: need GUI
    m_commandHistory = new kpCommandHistory (true/*read config*/, this);

    if (m_configFirstTime)
    {
        // (so that cfg-file-editing user can modify in the meantime)
        m_commandHistory->writeConfig ();
    }


    m_actionCut = KStdAction::cut (this, SLOT (slotCut ()), ac);
    m_actionCopy = KStdAction::copy (this, SLOT (slotCopy ()), ac);
    m_actionPaste = KStdAction::paste (this, SLOT (slotPaste ()), ac);
    m_actionPasteInNewWindow = new KAction (i18n ("Paste in &New Window"),
        Qt::CTRL + Qt::SHIFT + Qt::Key_V,
        this, SLOT (slotPasteInNewWindow ()), ac, "edit_paste_in_new_window");

    //m_actionDelete = KStdAction::clear (this, SLOT (slotDelete ()), ac);
    m_actionDelete = new KAction (i18n ("&Delete Selection"), 0,
        this, SLOT (slotDelete ()), ac, "edit_clear");

    m_actionSelectAll = KStdAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    m_actionDeselect = KStdAction::deselect (this, SLOT (slotDeselect ()), ac);


    m_actionCopyToFile = new KAction (i18n ("C&opy to File..."), 0,
        this, SLOT (slotCopyToFile ()), ac, "edit_copy_to_file");
    m_actionPasteFromFile = new KAction (i18n ("Paste &From File..."), 0,
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
    m_actionPasteFromFile->setEnabled (enable);
}


// public
Q3PopupMenu *kpMainWindow::selectionToolRMBMenu ()
{
    return (Q3PopupMenu *) guiFactory ()->container ("selectionToolRMBMenu", this);
}


// private slot
void kpMainWindow::slotCut ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotCut() CALLED" << endl;
#endif

    if (!m_document || !m_document->selection ())
    {
        kError () << "kpMainWindow::slotCut () doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }


    QApplication::setOverrideCursor (Qt::WaitCursor);

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
    kDebug () << "kpMainWindow::slotCopy() CALLED" << endl;
#endif

    if (!m_document || !m_document->selection ())
    {
        kError () << "kpMainWindow::slotCopy () doc=" << m_document
                   << " sel=" << (m_document ? m_document->selection () : 0)
                   << endl;
        return;
    }


    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();

    kpSelection sel = *m_document->selection ();
    // Transparency doesn't get sent across the aether so nuke it now
    // so that transparency mask doesn't get needlessly recalculated
    // if we ever call sel.setPixmap().
    sel.setTransparency (kpSelectionTransparency ());

    if (sel.isText ())
    {
        if (!sel.text ().isEmpty ())
        {
            QApplication::clipboard ()->setData (new Q3TextDrag (sel.text ()),
                                                 QClipboard::Clipboard);
        }
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
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow(" << name () << ")::slotEnablePaste()" << endl;
    QTime timer;
    timer.start ();
#endif

    bool shouldEnable = false;

    QMimeSource *ms = QApplication::clipboard ()->data (QClipboard::Clipboard);
    if (ms)
    {
        shouldEnable = (kpSelectionDrag::canDecode (ms) ||
                        Q3TextDrag::canDecode (ms));
    #if DEBUG_KP_MAIN_WINDOW
        kDebug () << "\t" << name () << "***canDecode=" << timer.restart () << endl;
        for (int i = 0; ; i++)
        {
            const char *fmt = ms->format (i);
            if (!fmt)
                break;

            kDebug () << "\t'" << fmt << "'" << endl;
        }
    #endif
    }

    m_actionPasteInNewWindow->setEnabled (shouldEnable);
    m_actionPaste->setEnabled (shouldEnable);
}


// private
QRect kpMainWindow::calcUsefulPasteRect (int pixmapWidth, int pixmapHeight)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::calcUsefulPasteRect("
               << pixmapWidth << "," << pixmapHeight
               << ")"
               << endl;
#endif
    if (!m_document)
    {
        kError () << "kpMainWindow::calcUsefulPasteRect() without doc" << endl;
        return QRect ();
    }

    // TODO: 1st choice is to paste sel near but not overlapping last deselect point

    if (m_mainView && m_scrollView)
    {
        const QPoint viewTopLeft (m_scrollView->contentsX (),
                                  m_scrollView->contentsY ());

        const QPoint docTopLeft = m_mainView->transformViewToDoc (viewTopLeft);

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
void kpMainWindow::paste (const kpSelection &sel, bool forceTopLeft)
{
    if (!sel.pixmap ())
    {
        kError () << "kpMainWindow::paste() with sel without pixmap" << endl;
        return;
    }

    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    //
    // Make sure we've got a document (esp. with File/Close)
    //

    if (!m_document)
    {
        kpDocument *newDoc = new kpDocument (
            sel.width (), sel.height (), this);

        // will also create viewManager
        setDocument (newDoc);
    }


    //
    // Paste as new selection
    //

    kpSelection selInUsefulPos = sel;
    if (!forceTopLeft)
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
void kpMainWindow::pasteText (const QString &text,
                              bool forceNewTextSelection,
                              const QPoint &newTextSelectionTopLeft)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::pasteText(" << text
               << ",forceNewTextSelection=" << forceNewTextSelection
               << ",newTextSelectionTopLeft=" << newTextSelectionTopLeft
               << ")" << endl;
#endif

    if (text.isEmpty ())
        return;


    // sync: restoreOverrideCursor() in all exit paths
    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    Q3ValueVector <QString> textLines (1, QString::null);

    for (int i = 0; i < (int) text.length (); i++)
    {
        if (text [i] == '\n')
            textLines.push_back (QString::null);
        else
            textLines [textLines.size () - 1].append (text [i]);
    }


    if (!forceNewTextSelection &&
        m_document && m_document->selection () &&
        m_document->selection ()->isText () &&
        m_commandHistory && m_viewManager)
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\treusing existing Text Selection" << endl;
    #endif

        kpMacroCommand *macroCmd = new kpMacroCommand (i18n ("Text: Paste"),
            this);

        for (int i = 0; i < (int) textLines.size (); i++)
        {
            if (i > 0)
            {
                macroCmd->addCommand (
                    new kpToolTextEnterCommand (
                        QString::null/*uninteresting child of macroCmd*/,
                        m_viewManager->textCursorRow (),
                        m_viewManager->textCursorCol (),
                        this));
            }

            macroCmd->addCommand (
                new kpToolTextInsertCommand (
                    QString::null/*uninteresting child of macroCmd*/,
                    m_viewManager->textCursorRow (),
                    m_viewManager->textCursorCol (),
                    textLines [i],
                    this));
        }

        m_commandHistory->addCommand (macroCmd, false/*no exec*/);
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\tcreating Text Selection" << endl;
    #endif

        const kpTextStyle ts = textStyle ();
        const QFontMetrics fontMetrics = ts.fontMetrics ();

        int height = textLines.size () * fontMetrics.height ();
        if (textLines.size () >= 1)
            height += (textLines.size () - 1) * fontMetrics.leading ();

        int width = 0;
        for (Q3ValueVector <QString>::const_iterator it = textLines.begin ();
             it != textLines.end ();
             it++)
        {
            const int w = fontMetrics.width (*it);
            if (w > width)
                width = w;
        }


        const int selWidth = QMAX (kpSelection::minimumWidthForTextStyle (ts),
                                   width + kpSelection::textBorderSize () * 2);
        const int selHeight = QMAX (kpSelection::minimumHeightForTextStyle (ts),
                                    height + kpSelection::textBorderSize () * 2);
        kpSelection sel (QRect (0, 0, selWidth, selHeight),
                         textLines,
                         ts);

        if (newTextSelectionTopLeft != KP_INVALID_POINT)
        {
            sel.moveTo (newTextSelectionTopLeft);
            paste (sel, true/*force topLeft*/);
        }
        else
        {
            paste (sel);
        }
    }


    QApplication::restoreOverrideCursor ();
}

// public
void kpMainWindow::pasteTextAt (const QString &text, const QPoint &point,
                                bool allowNewTextSelectionPointShift)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::pasteTextAt(" << text
               << ",point=" << point
               << ",allowNewTextSelectionPointShift="
               << allowNewTextSelectionPointShift
               << ")" << endl;
#endif

    QApplication::setOverrideCursor (Qt::WaitCursor);

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
        QPoint pointToUse = point;

        if (allowNewTextSelectionPointShift)
        {
            // TODO: In terms of doc pixels, would be inconsistent behaviour
            //       based on zoomLevel of view.
            // pointToUse -= QPoint (-view->selectionResizeHandleAtomicSize (),
            //                       -view->selectionResizeHandleAtomicSize ());
        }

        pasteText (text, true/*force new text selection*/, pointToUse);
    }

    QApplication::restoreOverrideCursor ();
}

// public slot
void kpMainWindow::slotPaste ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotPaste() CALLED" << endl;
#endif

    // sync: restoreOverrideCursor() in all exit paths
    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    //
    // Acquire the pixmap
    //

    QMimeSource *ms = QApplication::clipboard ()->data (QClipboard::Clipboard);
    if (!ms)
    {
        kError () << "kpMainWindow::slotPaste() without mimeSource" << endl;
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
    else if (Q3TextDrag::decode (ms, text/*ref*/))
    {
        pasteText (text);
    }
    else
    {
        QApplication::restoreOverrideCursor ();

        kDebug () << "kpMainWindow::slotPaste() could not decode selection" << endl;
        kDebug () << "\tFormats supported:" << endl;
        for (int i = 0; ms->format (i); i++)
        {
            kDebug () << "\t\t" << i << ":" << ms->format (i) << endl;
        }

        // TODO: fix Klipper
        KMessageBox::sorry (this,
            i18n ("<qt><p>KolourPaint cannot paste the contents of"
                  " the clipboard as the data unexpectedly disappeared.</p>"

                  "<p>This usually occurs if the application which was"
                  " responsible"
                  " for the clipboard contents has been closed.</p></qt>"),
            i18n ("Cannot Paste"));

        // TODO: PROPAGATE: interprocess
        if (KMainWindow::memberList())
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\thave memberList" << endl;
        #endif

            for (Q3PtrList <KMainWindow>::const_iterator it = KMainWindow::memberList()->begin ();
                 it != KMainWindow::memberList()->end ();
                 it++)
            {
                kpMainWindow *mw = dynamic_cast <kpMainWindow *> (*it);

                if (!mw)
                {
                    kError () << "kpMainWindow::slotPaste() given fake kpMainWindow: " << (*it) << endl;
                    continue;
                }
            #if DEBUG_KP_MAIN_WINDOW
                kDebug () << "\t\tmw=" << mw << endl;
            #endif

                mw->slotEnablePaste ();
            }
        }

        return;
    }

    QApplication::restoreOverrideCursor ();
}

// private slot
void kpMainWindow::slotPasteInNewWindow ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotPasteInNewWindow() CALLED" << endl;
#endif

    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    kpMainWindow *win = new kpMainWindow (0/*no document*/);
    win->show ();

    win->slotPaste ();
    win->slotCrop ();


    QApplication::restoreOverrideCursor ();
}

// public slot
void kpMainWindow::slotDelete ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotDelete() CALLED" << endl;
#endif
    if (!m_actionDelete->isEnabled ())
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\taction not enabled - was probably called from kpTool::keyPressEvent()" << endl;
    #endif
        return;
    }

    if (!m_document || !m_document->selection ())
    {
        kError () << "kpMainWindow::slotDelete () doc=" << m_document
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
    kDebug () << "kpMainWindow::slotSelectAll() CALLED" << endl;
#endif
    if (!m_document)
    {
        kError () << "kpMainWindow::slotSelectAll() without doc" << endl;
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
void kpMainWindow::addDeselectFirstCommand (kpCommand *cmd)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::addDeselectFirstCommand("
               << cmd
               << ")"
               << endl;
#endif


    kpSelection *sel = m_document->selection ();

#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "\tsel=" << sel << endl;
#endif

    if (sel)
    {
        // if you just dragged out something with no action then
        // forget the drag
        if (!sel->pixmap ())
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kDebug () << "\tjust a fresh border - was nop - delete" << endl;
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
            kDebug () << "\treal selection with pixmap - push onto doc cmd" << endl;
        #endif
            kpCommand *deselectCommand = new kpToolSelectionDestroyCommand (
                sel->isText () ?
                    i18n ("Text: Finish") :
                    i18n ("Selection: Deselect"),
                true/*push onto document*/,
                this);

            if (cmd)
            {
                kpMacroCommand *macroCmd = new kpMacroCommand (cmd->name (), this);
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
    kDebug () << "kpMainWindow::slotDeselect() CALLED" << endl;
#endif
    if (!m_document || !m_document->selection ())
    {
        kError () << "kpMainWindow::slotDeselect() doc=" << m_document
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
    kDebug () << "kpMainWindow::slotCopyToFile()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    if (!m_document->selection ())
        return;

    kpSelection sel = *m_document->selection ();

    QPixmap pixmapToSave;

    if (!sel.pixmap ())
    {
        // Not a floating selection - user has just selected a region;
        // haven't pulled it off yet so probably don't expect and can't
        // visualise selection transparency so give opaque, not transparent
        // pixmap.
        pixmapToSave = m_document->getSelectedPixmap ();
    }
    else
        pixmapToSave = sel.transparentPixmap ();


    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KUrl chosenURL = askForSaveURL (i18n ("Copy to File"),
                                    m_lastCopyToURL.url (),
                                    pixmapToSave,
                                    m_lastCopyToSaveOptions,
                                    kpDocumentMetaInfo (),
                                    kpSettingsGroupEditCopyTo,
                                    false/*allow remote files*/,
                                    &chosenSaveOptions,
                                    m_copyToFirstTime,
                                    &allowOverwritePrompt,
                                    &allowLossyPrompt);

    if (chosenURL.isEmpty ())
        return;


    if (!kpDocument::savePixmapToFile (pixmapToSave,
                                       chosenURL,
                                       chosenSaveOptions, kpDocumentMetaInfo (),
                                       allowOverwritePrompt,
                                       allowLossyPrompt,
                                       this))
    {
        return;
    }


    addRecentURL (chosenURL);


    m_lastCopyToURL = chosenURL;
    m_lastCopyToSaveOptions = chosenSaveOptions;

    m_copyToFirstTime = false;
}

// private slot
void kpMainWindow::slotPasteFromFile ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotPasteFromFile()" << endl;
#endif

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    KUrl::List urls = askForOpenURLs (i18n ("Paste From File"),
                                      m_lastPasteFromURL.url (),
                                      false/*only 1 URL*/);

    if (urls.count () != 1)
        return;

    KUrl url = urls.first ();
    m_lastPasteFromURL = url;


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

