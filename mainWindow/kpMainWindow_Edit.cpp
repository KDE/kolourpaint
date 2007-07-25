
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

#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qlist.h>
#include <qmenu.h>
#include <qpixmap.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>

#include <kpAbstractImageSelection.h>
#include <kpColorToolBar.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpDocumentMetaInfo.h>
#include <kpDocumentSaveOptions.h>
#include <kpMacroCommand.h>
#include <kpPixmapFX.h>
#include <kpRectangularImageSelection.h>
#include <kpSelectionDrag.h>
#include <kpTextSelection.h>
#include <kpTool.h>
#include <kpTransformCrop.h>
#include <kpTransformResizeScaleCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionDestroyCommand.h>
#include <kpToolTextEnterCommand.h>
#include <kpToolTextInsertCommand.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>
#include <kpZoomedView.h>


// private
kpPixmapFX::WarnAboutLossInfo kpMainWindow::pasteWarnAboutLossInfo ()
{
    return kpPixmapFX::WarnAboutLossInfo (
               ki18n ("The image to be pasted"
                     " may have more colors than the current screen mode."
                     " In order to display it, some colors may be changed."
                     " Try increasing your screen depth to at least %1bpp."

                     "\nIt also"

                     " contains translucency which is not fully"
                     " supported. The translucency data will be"
                     " approximated with a 1-bit transparency mask."),
               ki18n ("The image to be pasted"
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


    m_actionCut = KStandardAction::cut (this, SLOT (slotCut ()), ac);
    m_actionCopy = KStandardAction::copy (this, SLOT (slotCopy ()), ac);
    m_actionPaste = KStandardAction::paste (this, SLOT (slotPaste ()), ac);
    m_actionPasteInNewWindow = ac->addAction ("edit_paste_in_new_window");
    m_actionPasteInNewWindow->setText (i18n ("Paste in &New Window"));
    connect (m_actionPasteInNewWindow, SIGNAL (triggered (bool)),
        SLOT (slotPasteInNewWindow ()));
    m_actionPasteInNewWindow->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);

    //m_actionDelete = KStandardAction::clear (this, SLOT (slotDelete ()), ac);
    m_actionDelete = ac->addAction ("edit_clear");
    m_actionDelete->setText (i18n ("&Delete Selection"));
    connect (m_actionDelete, SIGNAL (triggered (bool)), SLOT (slotDelete ()));

    m_actionSelectAll = KStandardAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    m_actionDeselect = KStandardAction::deselect (this, SLOT (slotDeselect ()), ac);


    m_actionCopyToFile = ac->addAction ("edit_copy_to_file");
    m_actionCopyToFile->setText (i18n ("C&opy to File..."));
    connect (m_actionCopyToFile, SIGNAL (triggered (bool)),
        SLOT (slotCopyToFile ()));
    m_actionPasteFromFile = ac->addAction ("edit_paste_from_file");
    m_actionPasteFromFile->setText (i18n ("Paste &From File..."));
    connect (m_actionPasteFromFile, SIGNAL (triggered (bool)),
        SLOT (slotPasteFromFile ()));


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
QMenu *kpMainWindow::selectionToolRMBMenu ()
{
    return qobject_cast <QMenu *> (guiFactory ()->container ("selectionToolRMBMenu", this));
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

    kpAbstractSelection *sel = m_document->selection ()->clone ();

    if (dynamic_cast <kpTextSelection *> (sel))
    {
        kpTextSelection *textSel = static_cast <kpTextSelection *> (sel);
        if (!textSel->text ().isEmpty ())
        {
            QApplication::clipboard ()->setData (new Q3TextDrag (textSel->text ()),
                                                 QClipboard::Clipboard);

            // SYNC: Normally, users highlight text and press CTRL+C.
            //       Highlighting text copies it to the X11 "middle
            //       mouse button" clipboard.  CTRL+C copies it to the
            //       separate, Windows-like "CTRL+V" clipboard.
            //
            //       However, KolourPaint doesn't support highlighting.
            //       So when they press CTRL+C to copy all text, simulate
            //       the highlighting by copying the text to the "middle
            //       mouse button" clipboard.  We don't do this for images
            //       as no one ever middle-mouse-pastes images.
            //
            //       Note that we don't share the QTextDrag pointer with
            //       the above in case Qt doesn't expect it.
            //
            //       Once we change KolourPaint to support highlighted text
            //       and CTRL+C to copy only the highlighted text, delete
            //       this code.
            QApplication::clipboard ()->setData (new Q3TextDrag (textSel->text ()),
                                                 QClipboard::Selection);
        }
    }
    else if (dynamic_cast <kpAbstractImageSelection *> (sel))
    {
        kpAbstractImageSelection *imageSel =
            static_cast <kpAbstractImageSelection *> (sel);

        // Transparency doesn't get sent across the aether so nuke it now
        // so that transparency mask doesn't get needlessly recalculated
        // if we ever call sel.setBaseImage().
        imageSel->setTransparency (kpImageSelectionTransparency ());

        if (!imageSel->hasContent ())
            imageSel->setBaseImage (m_document->getSelectedBaseImage ());
        QApplication::clipboard ()->setData (new kpSelectionDrag (*imageSel),
                                             QClipboard::Clipboard);
    }
    else
        Q_ASSERT (!"Unknown selection type");

    delete sel;

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
void kpMainWindow::paste (const kpAbstractSelection &sel, bool forceTopLeft)
{
    // COMPAT: update
#if 0
    if (!sel.pixmap ())
    {
        kError () << "kpMainWindow::paste() with sel without pixmap" << endl;
        return;
    }
#endif

    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (toolHasBegunShape ())
        tool ()->endShapeInternal ();


    //
    // Make sure we've got a document (esp. with File/Close)
    //

    if (!m_document)
    {
        kpDocument *newDoc = new kpDocument (
            sel.width (), sel.height (), documentEnvironment ());

        // will also create viewManager
        setDocument (newDoc);
    }


    //
    // Paste as new selection
    //

    const kpAbstractImageSelection *imageSel =
        dynamic_cast <const kpAbstractImageSelection *> (&sel);
    if (imageSel && imageSel->hasContent () && imageSel->transparency ().isTransparent ())
    {
        m_colorToolBar->flashColorSimilarityToolBarItem ();
    }

    kpAbstractSelection *selInUsefulPos = sel.clone ();
    if (!forceTopLeft)
        selInUsefulPos->moveTo (calcUsefulPasteRect (sel.width (), sel.height ()).topLeft ());
    addDeselectFirstCommand (new kpToolSelectionCreateCommand (
        dynamic_cast <kpTextSelection *> (selInUsefulPos) ?
            i18n ("Text: Create Box") :
            i18n ("Selection: Create"),
        *selInUsefulPos,
        commandEnvironment ()));
    delete selInUsefulPos;


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
            new kpTransformResizeScaleCommand (
                false/*act on doc, not sel*/,
                qMax (sel.width (), m_document->width ()),
                qMax (sel.height (), m_document->height ()),
                kpTransformResizeScaleCommand::Resize,
                commandEnvironment ()));
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


    QList <QString> textLines;
    textLines.append (QString ());

    for (int i = 0; i < (int) text.length (); i++)
    {
        if (text [i] == '\n')
            textLines.push_back (QString::null);
        else
            textLines [textLines.size () - 1].append (text [i]);
    }


    if (!forceNewTextSelection &&
        m_document && m_document->textSelection () &&
        m_commandHistory && m_viewManager)
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\treusing existing Text Selection" << endl;
    #endif

        kpMacroCommand *macroCmd = new kpMacroCommand (i18n ("Text: Paste"),
            commandEnvironment ());

        for (int i = 0; i < (int) textLines.size (); i++)
        {
            if (i > 0)
            {
                macroCmd->addCommand (
                    new kpToolTextEnterCommand (
                        QString::null/*uninteresting child of macroCmd*/,
                        m_viewManager->textCursorRow (),
                        m_viewManager->textCursorCol (),
                        kpToolTextEnterCommand::AddEnterNow,
                        commandEnvironment ()));
            }

            macroCmd->addCommand (
                new kpToolTextInsertCommand (
                    QString::null/*uninteresting child of macroCmd*/,
                    m_viewManager->textCursorRow (),
                    m_viewManager->textCursorCol (),
                    textLines [i],
                    commandEnvironment ()));
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
        for (QList <QString>::const_iterator it = textLines.begin ();
             it != textLines.end ();
             it++)
        {
            const int w = fontMetrics.width (*it);
            if (w > width)
                width = w;
        }


        const int selWidth = qMax (kpTextSelection::MinimumWidthForTextStyle (ts),
                                   width + kpTextSelection::TextBorderSize () * 2);
        const int selHeight = qMax (kpTextSelection::MinimumHeightForTextStyle (ts),
                                    height + kpTextSelection::TextBorderSize () * 2);
        kpAbstractSelection *sel = new kpTextSelection (QRect (0, 0, selWidth, selHeight),
            textLines,
            ts);

        if (newTextSelectionTopLeft != KP_INVALID_POINT)
        {
            sel->moveTo (newTextSelectionTopLeft);
            paste (*sel, true/*force topLeft*/);
        }
        else
        {
            paste (*sel);
        }

        delete sel;
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
        m_document->textSelection () &&
        m_document->textSelection ()->pointIsInTextArea (point))
    {
        kpTextSelection *textSel = m_document->textSelection ();

        const int row = textSel->closestTextRowForPoint (point);
        const int col = textSel->closestTextColForPoint (point);

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

    kpAbstractImageSelection *sel = kpSelectionDrag::decode (ms,
        pasteWarnAboutLossInfo ());
    QString text;
    if (sel)
    {
        sel->setTransparency (imageSelectionTransparency ());
        paste (*sel);
        delete sel;
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
        if (!KMainWindow::memberList ().isEmpty ())
        {
        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\thave memberList" << endl;
        #endif

            for (QList <KMainWindow *>::const_iterator it = KMainWindow::memberList ().begin ();
                 it != KMainWindow::memberList ().end ();
                 it++)
            {
                kpMainWindow *mw = dynamic_cast <kpMainWindow *> (*it);
                Q_ASSERT (mw);

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
// HITODO: Pasting season.jpg with sel transparency settings of transparent,
//         white background color and 27% color similarity has strange effects
//         (undo/redo just after pasting to see what I mean).
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
        m_document->textSelection () ?
            i18n ("Text: Delete Box") :  // not to be confused with i18n ("Text: Delete")
            i18n ("Selection: Delete"),
        false/*no push onto doc*/,
        commandEnvironment ()));
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
    m_document->setSelection (
        kpRectangularImageSelection (m_document->rect (),
            imageSelectionTransparency ()));

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


    kpAbstractSelection *sel = m_document->selection ();

#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "\tsel=" << sel << endl;
#endif

    if (sel)
    {
        // if you just dragged out something with no action then
        // forget the drag
        if (!sel->hasContent ())
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
                dynamic_cast <kpTextSelection *> (sel) ?
                    i18n ("Text: Finish") :
                    i18n ("Selection: Deselect"),
                true/*push onto document*/,
                commandEnvironment ());

            if (cmd)
            {
                kpMacroCommand *macroCmd = new kpMacroCommand (cmd->name (),
                    commandEnvironment ());
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

    kpImage imageToSave;

    if (m_document->imageSelection ())
    {
        kpAbstractImageSelection *imageSel = m_document->imageSelection ();
        if (!imageSel->hasContent ())
        {
            // Not a floating selection - user has just selected a region;
            // haven't pulled it off yet so probably don't expect and can't
            // visualize selection transparency so give opaque, not transparent
            // pixmap.
            imageToSave = m_document->getSelectedBaseImage ();
        }
        else
            imageToSave = imageSel->transparentImage ();
    }
    else if (m_document->textSelection ())
    {
        imageToSave = m_document->textSelection ()->approximateImage ();
    }
    else
        Q_ASSERT (!"Unknown selection type");


    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KUrl chosenURL = askForSaveURL (i18n ("Copy to File"),
                                    m_lastCopyToURL.url (),
                                    imageToSave,
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


    if (!kpDocument::savePixmapToFile (imageToSave,
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

    // HITODO: We're not respecting the currently selected selection transparency.
    //         Check everywhere that we are.  Bug also in KDE 3.
    paste (kpRectangularImageSelection (
        QRect (0, 0, pixmap.width (), pixmap.height ()),
        pixmap));
}

