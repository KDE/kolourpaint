
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
#include <kpSetOverrideCursorSaver.h>
#include <kpTextSelection.h>
#include <kpTool.h>
#include <kpToolTextGiveContentCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionDestroyCommand.h>
#include <kpToolTextEnterCommand.h>
#include <kpToolTextInsertCommand.h>
#include <kpTransformCrop.h>
#include <kpTransformResizeScaleCommand.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>
#include <kpZoomedView.h>


// private
kpPixmapFX::WarnAboutLossInfo kpMainWindow::pasteWarnAboutLossInfo ()
{
    return kpPixmapFX::WarnAboutLossInfo (
        ki18n ("<qt><p>The image to be pasted"
            " may have more colors than the current screen mode can support."
            " In order to display it, some color information may be removed.</p>"

            "<p><b>If you save this image, any color loss will become"
            " permanent.</b></p>"

            "<p>To avoid this issue, increase your screen depth to at"
            " least %1bpp and then restart KolourPaint.</p>"

            "<hr/>"

            "<p>It also"

            " contains translucency which is not fully"
            " supported. The translucency data will be"
            " approximated with a 1-bit transparency mask.</p>"

            "<p><b>If you save this image, this loss of translucency will"
            " become permanent.</b></p></qt>"),
        ki18n ("<qt><p>The image to be pasted"
            " may have more colors than the current screen mode can support."
            " In order to display it, some color information may be removed.</p>"

            "<p><b>If you save this image, any color loss will become"
            " permanent.</b></p>"

            "<p>To avoid this issue, increase your screen depth to at"
            " least %1bpp and then restart KolourPaint.</p></qt>"),
        i18n ("<qt><p>The image to be pasted"
            " contains translucency which is not fully"
            " supported. The translucency data will be"
            " approximated with a 1-bit transparency mask.</p>"

            "<p><b>If you save this image, this loss of translucency will"
            " become permanent.</b></p></qt>"),
        "paste",
        this);
}


// private
void kpMainWindow::setupEditMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    // Undo/Redo
    // CONFIG: Need GUI for config history size.
    d->commandHistory = new kpCommandHistory (true/*read config*/, this);

    if (d->configFirstTime)
    {
        // (so that cfg-file-editing user can modify in the meantime)
        d->commandHistory->writeConfig ();
    }


    d->actionCut = KStandardAction::cut (this, SLOT (slotCut ()), ac);
    d->actionCopy = KStandardAction::copy (this, SLOT (slotCopy ()), ac);
    d->actionPaste = KStandardAction::paste (this, SLOT (slotPaste ()), ac);
    d->actionPasteInNewWindow = ac->addAction ("edit_paste_in_new_window");
    d->actionPasteInNewWindow->setText (i18n ("Paste in &New Window"));
    connect (d->actionPasteInNewWindow, SIGNAL (triggered (bool)),
        SLOT (slotPasteInNewWindow ()));
    d->actionPasteInNewWindow->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);

    //d->actionDelete = KStandardAction::clear (this, SLOT (slotDelete ()), ac);
    d->actionDelete = ac->addAction ("edit_clear");
    d->actionDelete->setText (i18n ("&Delete Selection"));
    connect (d->actionDelete, SIGNAL (triggered (bool)), SLOT (slotDelete ()));

    d->actionSelectAll = KStandardAction::selectAll (this, SLOT (slotSelectAll ()), ac);
    d->actionDeselect = KStandardAction::deselect (this, SLOT (slotDeselect ()), ac);


    d->actionCopyToFile = ac->addAction ("edit_copy_to_file");
    d->actionCopyToFile->setText (i18n ("C&opy to File..."));
    connect (d->actionCopyToFile, SIGNAL (triggered (bool)),
        SLOT (slotCopyToFile ()));
    d->actionPasteFromFile = ac->addAction ("edit_paste_from_file");
    d->actionPasteFromFile->setText (i18n ("Paste &From File..."));
    connect (d->actionPasteFromFile, SIGNAL (triggered (bool)),
        SLOT (slotPasteFromFile ()));


    d->editMenuDocumentActionsEnabled = false;
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
    // d->actionCut
    // d->actionCopy
    // d->actionPaste
    // d->actionPasteInNewWindow

    // d->actionDelete

    d->actionSelectAll->setEnabled (enable);
    // d->actionDeselect

    d->editMenuDocumentActionsEnabled = enable;

    // d->actionCopyToFile
    d->actionPasteFromFile->setEnabled (enable);
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
    kDebug () << "kpMainWindow::slotCut() CALLED";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    Q_ASSERT (d->document && d->document->selection ());

    toolEndShape ();

    slotCopy ();
    slotDelete ();
}

// private slot
void kpMainWindow::slotCopy ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotCopy() CALLED";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    Q_ASSERT (d->document && d->document->selection ());

    toolEndShape ();

    kpAbstractSelection *sel = d->document->selection ()->clone ();

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

        kpImage rawImage;

        if (imageSel->hasContent ())
            rawImage = imageSel->baseImage ();
        else
            rawImage = d->document->getSelectedBaseImage ();

        // Some apps, such as OpenOffice.org 2.0.4, ignore the image mask
        // when pasting.  For transparent pixels, the uninitialized RGB
        // values are used.  Fix this by initializing those values to a
        // neutral color -- white.
        //
        // Strangely enough, OpenOffice.org respects the mask when inserting
        // an image from a file, as opposed to pasting one from the clipboard.
        //
        // REFACTOR: This logic should be moved into kpSelectionDrag as that
        //           would centralize this responsibility.
        imageSel->setBaseImage (
            kpPixmapFX::pixmapWithDefinedTransparentPixels (
                rawImage,
                Qt::white));  // CONFIG

        QApplication::clipboard ()->setData (new kpSelectionDrag (*imageSel),
                                             QClipboard::Clipboard);
    }
    else
        Q_ASSERT (!"Unknown selection type");

    delete sel;
}

// private slot
void kpMainWindow::slotEnablePaste ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow(" << name () << ")::slotEnablePaste()";
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
        kDebug () << "\t" << name () << "***canDecode=" << timer.restart ();
        for (int i = 0; ; i++)
        {
            const char *fmt = ms->format (i);
            if (!fmt)
                break;

            kDebug () << "\t'" << fmt << "'";
        }
    #endif
    }

    d->actionPasteInNewWindow->setEnabled (shouldEnable);
    d->actionPaste->setEnabled (shouldEnable);
}


// private
QRect kpMainWindow::calcUsefulPasteRect (int imageWidth, int imageHeight)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::calcUsefulPasteRect("
               << imageWidth << "," << imageHeight
               << ")"
               << endl;
#endif
    Q_ASSERT (d->document);

    // TODO: 1st choice is to paste sel near but not overlapping last deselect point

    if (d->mainView && d->scrollView)
    {
        const QPoint viewTopLeft (d->scrollView->contentsX (),
                                  d->scrollView->contentsY ());

        const QPoint docTopLeft = d->mainView->transformViewToDoc (viewTopLeft);

        if ((docTopLeft.x () + imageWidth <= d->document->width () &&
             docTopLeft.y () + imageHeight <= d->document->height ()) ||
            imageWidth <= docTopLeft.x () ||
            imageHeight <= docTopLeft.y ())
        {
            return QRect (docTopLeft.x (), docTopLeft.y (),
                          imageWidth, imageHeight);
        }
    }

    return QRect (0, 0, imageWidth, imageHeight);
}

// private
void kpMainWindow::paste (const kpAbstractSelection &sel, bool forceTopLeft)
{
    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    toolEndShape ();


    //
    // Make sure we've got a document (esp. with File/Close)
    //

    if (!d->document)
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
        d->colorToolBar->flashColorSimilarityToolBarItem ();
    }

    kpAbstractSelection *selInUsefulPos = sel.clone ();
    if (!forceTopLeft)
        selInUsefulPos->moveTo (calcUsefulPasteRect (sel.width (), sel.height ()).topLeft ());
    // TODO: Should use kpCommandHistory::addCreateSelectionCommand()
    //       to really support pasting selection borders.
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
    if (sel.width () > d->document->width () ||
        sel.height () > d->document->height ())
    {
        d->commandHistory->addCommand (
            new kpTransformResizeScaleCommand (
                false/*act on doc, not sel*/,
                qMax (sel.width (), d->document->width ()),
                qMax (sel.height (), d->document->height ()),
                kpTransformResizeScaleCommand::Resize,
                commandEnvironment ()));
    }
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


    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    toolEndShape ();


    QList <QString> textLines;
    textLines.append (QString ());

    for (int i = 0; i < (int) text.length (); i++)
    {
        if (text [i] == '\n')
            textLines.push_back (QString::null);	//krazy:exclude=nullstrassign for old broken gcc
        else
            textLines [textLines.size () - 1].append (text [i]);
    }


    if (!forceNewTextSelection &&
        d->document && d->document->textSelection () &&
        d->commandHistory && d->viewManager)
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\treusing existing Text Selection";
    #endif

        kpTextSelection *textSel = d->document->textSelection ();
        if (!textSel->hasContent ())
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kDebug () << "\t\tneeds content" << endl;
        #endif
            commandHistory ()->addCreateSelectionCommand (
                new kpToolSelectionCreateCommand (
                    i18n ("Text: Create Box"),
                    *textSel,
                    commandEnvironment ()),
                false/*no exec*/);
        }

        kpMacroCommand *macroCmd = new kpMacroCommand (i18n ("Text: Paste"),
            commandEnvironment ());
        // (yes, this is the same check as the previous "if")
        if (!textSel->hasContent ())
        {
            kpCommand *giveContentCmd = new kpToolTextGiveContentCommand (
                *textSel,
                QString ()/*uninteresting child of macro cmd*/,
                commandEnvironment ());
            giveContentCmd->execute ();

            macroCmd->addCommand (giveContentCmd);
        }

        for (int i = 0; i < (int) textLines.size (); i++)
        {
            if (i > 0)
            {
                macroCmd->addCommand (
                    new kpToolTextEnterCommand (
                        QString::null/*uninteresting child of macroCmd*/,	//krazy:exclude=nullstrassign for old broken gcc
                        d->viewManager->textCursorRow (),
                        d->viewManager->textCursorCol (),
                        kpToolTextEnterCommand::AddEnterNow,
                        commandEnvironment ()));
            }

            macroCmd->addCommand (
                new kpToolTextInsertCommand (
                    QString::null/*uninteresting child of macroCmd*/,	//krazy:exclude=nullstrassign for old broken gcc
                    d->viewManager->textCursorRow (),
                    d->viewManager->textCursorCol (),
                    textLines [i],
                    commandEnvironment ()));
        }

        d->commandHistory->addCommand (macroCmd, false/*no exec*/);
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\tcreating Text Selection";
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
        kpTextSelection newTextSel (QRect (0, 0, selWidth, selHeight),
            textLines,
            ts);

        if (newTextSelectionTopLeft != KP_INVALID_POINT)
        {
            newTextSel.moveTo (newTextSelectionTopLeft);
            paste (newTextSel, true/*force topLeft*/);
        }
        else
        {
            paste (newTextSel);
        }
    }
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

    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    toolEndShape ();


    if (d->document &&
        d->document->textSelection () &&
        d->document->textSelection ()->pointIsInTextArea (point))
    {
        kpTextSelection *textSel = d->document->textSelection ();

        int row, col;

        if (textSel->hasContent ())
        {
            row = textSel->closestTextRowForPoint (point);
            col = textSel->closestTextColForPoint (point);
        }
        else
        {
            row = col = 0;
        }

        d->viewManager->setTextCursorPosition (row, col);

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
}

// public slot
void kpMainWindow::slotPaste ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotPaste() CALLED";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    toolEndShape ();


    //
    // Acquire the image.
    //

    QMimeSource *ms = QApplication::clipboard ()->data (QClipboard::Clipboard);
    if (!ms)
    {
        kError () << "kpMainWindow::slotPaste() without mimeSource" << endl;
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
        kpSetOverrideCursorSaver cursorSaver (Qt::arrowCursor);

        kDebug () << "kpMainWindow::slotPaste() could not decode selection";
        kDebug () << "\tFormats supported:";
        for (int i = 0; ms->format (i); i++)
        {
            kDebug () << "\t\t" << i << ":" << ms->format (i);
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
        foreach (KMainWindow *kmw, KMainWindow::memberList ())
        {
            Q_ASSERT (dynamic_cast <kpMainWindow *> (kmw));
            kpMainWindow *mw = static_cast <kpMainWindow *> (kmw);

        #if DEBUG_KP_MAIN_WINDOW
            kDebug () << "\t\tmw=" << mw;
        #endif

            mw->slotEnablePaste ();
        }

        return;
    }
}

// private slot
// HITODO: Pasting season.jpg with sel transparency settings of transparent,
//         white background color and 27% color similarity has strange effects
//         (undo/redo just after pasting to see what I mean).
void kpMainWindow::slotPasteInNewWindow ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotPasteInNewWindow() CALLED";
#endif

    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    toolEndShape ();


    kpMainWindow *win = new kpMainWindow (0/*no document*/);
    win->show ();

    win->slotPaste ();
    win->slotCrop ();
}

// public slot
void kpMainWindow::slotDelete ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotDelete() CALLED";
#endif
    if (!d->actionDelete->isEnabled ())
    {
    #if DEBUG_KP_MAIN_WINDOW && 1
        kDebug () << "\taction not enabled - was probably called from kpTool::keyPressEvent()";
    #endif
        return;
    }

    Q_ASSERT (d->document && d->document->selection ());

    toolEndShape ();

    addImageOrSelectionCommand (new kpToolSelectionDestroyCommand (
        d->document->textSelection () ?
            i18n ("Text: Delete Box") :  // not to be confused with i18n ("Text: Delete")
            i18n ("Selection: Delete"),
        false/*no push onto doc*/,
        commandEnvironment ()));
}


// private slot
void kpMainWindow::slotSelectAll ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotSelectAll() CALLED";
#endif
    Q_ASSERT (d->document);

    toolEndShape ();

    if (d->document->selection ())
        slotDeselect ();

    // just the border - don't actually pull image from doc yet
    d->document->setSelection (
        kpRectangularImageSelection (d->document->rect (),
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


    kpAbstractSelection *sel = d->document->selection ();

#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "\tsel=" << sel;
#endif

    if (sel)
    {
        // if you just dragged out something with no action then
        // forget the drag
        if (!sel->hasContent ())
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kDebug () << "\tjust a fresh border - was nop - delete";
        #endif
            d->document->selectionDelete ();
            if (tool ())
                tool ()->somethingBelowTheCursorChanged ();

            if (cmd)
                d->commandHistory->addCommand (cmd);
        }
        else
        {
        #if DEBUG_KP_MAIN_WINDOW && 1
            kDebug () << "\treal selection with image - push onto doc cmd";
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
                d->commandHistory->addCommand (macroCmd);
            }
            else
                d->commandHistory->addCommand (deselectCommand);
        }
    }
    else
    {
        if (cmd)
            d->commandHistory->addCommand (cmd);
    }
}


// public slot
void kpMainWindow::slotDeselect ()
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kDebug () << "kpMainWindow::slotDeselect() CALLED";
#endif
    Q_ASSERT (d->document && d->document->selection ());

    toolEndShape ();

    addDeselectFirstCommand (0);
}


// private slot
void kpMainWindow::slotCopyToFile ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotCopyToFile()";
#endif

    toolEndShape ();


    if (!d->document->selection ())
        return;

    kpImage imageToSave;

    if (d->document->imageSelection ())
    {
        kpAbstractImageSelection *imageSel = d->document->imageSelection ();
        if (!imageSel->hasContent ())
        {
            // Not a floating selection - user has just selected a region;
            // haven't pulled it off yet so probably don't expect and can't
            // visualize selection transparency so give opaque, not transparent
            // image.
            imageToSave = d->document->getSelectedBaseImage ();
        }
        else
            imageToSave = imageSel->transparentImage ();
    }
    else if (d->document->textSelection ())
    {
        imageToSave = d->document->textSelection ()->approximateImage ();
    }
    else
        Q_ASSERT (!"Unknown selection type");


    kpDocumentSaveOptions chosenSaveOptions;
    bool allowOverwritePrompt, allowLossyPrompt;
    KUrl chosenURL = askForSaveURL (i18n ("Copy to File"),
                                    d->lastCopyToURL.url (),
                                    imageToSave,
                                    d->lastCopyToSaveOptions,
                                    kpDocumentMetaInfo (),
                                    kpSettingsGroupEditCopyTo,
                                    false/*allow remote files*/,
                                    &chosenSaveOptions,
                                    d->copyToFirstTime,
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


    d->lastCopyToURL = chosenURL;
    d->lastCopyToSaveOptions = chosenSaveOptions;

    d->copyToFirstTime = false;
}

// private slot
void kpMainWindow::slotPasteFromFile ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotPasteFromFile()";
#endif

    toolEndShape ();


    KUrl::List urls = askForOpenURLs (i18n ("Paste From File"),
                                      d->lastPasteFromURL.url (),
                                      false/*only 1 URL*/);

    if (urls.count () != 1)
        return;

    KUrl url = urls.first ();
    d->lastPasteFromURL = url;


    kpImage image = kpDocument::getPixmapFromFile (url,
        false/*show error message if doesn't exist*/,
        this);


    if (image.isNull ())
        return;


    addRecentURL (url);

    // HITODO: We're not respecting the currently selected selection transparency.
    //         Check everywhere that we are.  KDE3: Bug also in KDE 3.
    paste (kpRectangularImageSelection (
        QRect (0, 0, image.width (), image.height ()),
        image));
}

