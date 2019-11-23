
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


#ifndef KP_MAIN_WINDOW_H
#define KP_MAIN_WINDOW_H


#include <QUrl>

#include <kxmlguiwindow.h>

#include "kpDefs.h"
#include "pixmapfx/kpPixmapFX.h"
#include "imagelib/kpImage.h"


class QAction;
class QActionGroup;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QMoveEvent;
class QPoint;
class QRect;
class QSize;

class KConfigGroup;
class KToolBar;
class QPrinter;

class kpColor;
class kpColorCells;
class kpColorToolBar;
class kpCommand;
class kpCommandEnvironment;
class kpCommandHistory;
class kpDocument;
class kpDocumentEnvironment;
class kpDocumentMetaInfo;
class kpDocumentSaveOptions;
class kpViewManager;
class kpImageSelectionTransparency;
class kpTextStyle;
class kpThumbnail;
class kpTool;
class kpToolEnvironment;
class kpToolSelectionEnvironment;
class kpToolToolBar;
class kpTransformDialogEnvironment;
class kpAbstractSelection;

class kpMainWindow : public KXmlGuiWindow
{
Q_OBJECT

public:
    // Opens a new window with a blank document.
    kpMainWindow ();

    // Opens a new window with the document specified by <url>
    // or creates a blank document if <url> could not be opened.
    kpMainWindow (const QUrl &url);

    // Opens a new window with the document <newDoc>
    // (<newDoc> can be 0 although this would result in a new
    //  window without a document at all).
    kpMainWindow (kpDocument *newDoc);

    void finalizeGUI(KXMLGUIClient *client) override;

private:
    void readGeneralSettings ();
    void readThumbnailSettings ();

    void init ();

    // (only called for restoring a previous session e.g. starting KDE with
    //  a previously saved session; it's not called on normal KolourPaint
    //  startup)
    void readProperties (const KConfigGroup &configGroup) override;
    // (only called for saving the current session e.g. logging out of KDE
    //  with the KolourPaint window open; it's not called on normal KolourPaint
    //  exit)
    void saveProperties (KConfigGroup &configGroup) override;

public:
    ~kpMainWindow () override;

public:
    kpDocument *document () const;
    kpDocumentEnvironment *documentEnvironment ();
    kpViewManager *viewManager () const;
    kpColorToolBar *colorToolBar () const;
    kpColorCells *colorCells () const;
    kpToolToolBar *toolToolBar () const;
    kpCommandHistory *commandHistory () const;
    kpCommandEnvironment *commandEnvironment ();

private:
    void setupActions ();
    void enableDocumentActions (bool enable = true);

    void setDocument (kpDocument *newDoc);

    void dragEnterEvent (QDragEnterEvent *e) override;
    void dropEvent (QDropEvent *e) override;
    void moveEvent (QMoveEvent *e) override;

private slots:
    void slotScrollViewAfterScroll ();
    void slotUpdateCaption ();
    void slotDocumentRestored ();


//
// Tools
//

private:
    kpToolSelectionEnvironment *toolSelectionEnvironment ();
    kpToolEnvironment *toolEnvironment ();

    void setupToolActions ();
    void createToolBox ();
    void enableToolsDocumentActions (bool enable = true);

private slots:
    void updateToolOptionPrevNextActionsEnabled ();
    void updateActionDrawOpaqueChecked ();
private:
    void updateActionDrawOpaqueEnabled ();

public:
    QActionGroup *toolsActionGroup ();

    kpTool *tool () const;

    bool toolHasBegunShape () const;
    bool toolIsASelectionTool (bool includingTextTool = true) const;
    bool toolIsTextTool () const;

private:
    // Ends the current shape.  If there is no shape currently being drawn,
    // it does nothing.
    //
    // In general, call this at the start of every kpMainWindow slot,
    // directly invoked by the _user_ (by activating an action or via another
    // way), so that:
    //
    // 1. The document contains the pixels of that shape:
    //
    //    Most tools have the shape, currently being drawn, layered above the
    //    document as a kpTempImage.  In other words, the document does not
    //    yet contain the pixels of that shape.  By ending the shape, the layer
    //    is pushed down onto the document so that it now contains those
    //    pixels.  Your slot can now safely read the document as it's now
    //    consistent with what's on the screen.
    //
    //    For example, consider the case where a line is being dragged out and
    //    CTRL+I is pressed to invert the image, while the mouse is still held
    //    down.  The CTRL+I invert code (kpMainWindow::slotInvertColors()) must
    //    push the line kpTempImage onto the document before the invert can
    //    meaningfully proceed (else the invert will see the state of the document
    //    before the line was dragged out).
    //
    //    Note that selection layers are not pushed down by this method.
    //    This is a feature, not a bug.  The user would be annoyed if e.g.
    //    slotSave() happened to push down the selection.  Use
    //    kpDocument::imageWithSelection() to get around this problem.  You
    //    should still call toolEndShape() even if a selection is active
    //    -- this ends selection "shapes", which are actually things like
    //    selection moves or smearing operations, rather than the selections
    //    themselves.
    //
    // AND/OR:
    //
    // 2. The current tool is no longer in a drawing state:
    //
    //    If your slot is going to bring up a new main window or modal dialog
    //    or at least some widget that acquires mouse or keyboard focus, this
    //    could confuse the tool if the tool is in the middle of a drawing
    //    operation.
    //
    // Do not call this in slots not invoked by the user.  For instance,
    // calling this method in response to an internal timer tick would be
    // wrong.  The user's drawing operation would unexpectedly finish and
    // this would bewilder and irritate the user.
    //
    // TODO: Help / KolourPaint Handbook does not call this.  I'm sure there
    //       are a few other actions that don't call this but should.
    void toolEndShape ();

public:
    kpImageSelectionTransparency imageSelectionTransparency () const;
    // The drawing background color is set to <transparency>.transparentColor()
    // if the <transparency> is in Transparent mode or if <forceColorChange>
    // is true (not the default).  [x]
    //
    // If <transparency> is in Opaque mode and <forceColorChange> is false,
    // the background color is not changed because:
    //
    //   1. It is ignored by the selection in Opaque mode anyway.
    //   2. This avoids irritating the user with an unnecessary background
    //      color change.
    //
    // The only case where you should set <forceColorChange> to true is in
    // kpToolImageSelectionTransparencyCommand to ensure that the state
    // is identical to when the command was constructed.
    // Later: I don't think setting it to true is ever necessary since:
    //
    //          1. The background color only counts in Transparent mode.
    //
    //          2. Any kpToolImageSelectionTransparencyCommand that switches to
    //             Transparent mode will automatically set the background
    //             color due to the first part of [x] anyway.
    //
    // The other fields of <transparency> are copied into the main window
    // as expected.
    void setImageSelectionTransparency (const kpImageSelectionTransparency &transparency,
                                   bool forceColorChange = false);
    int settingImageSelectionTransparency () const;

private slots:
    void slotToolSelected (kpTool *tool);

private:
    void readLastTool ();
    int toolNumber () const;
    void saveLastTool ();

private:
    bool maybeDragScrollingMainView () const;
private slots:
    bool slotDragScroll (const QPoint &docPoint,
                         const QPoint &docLastPoint,
                         int zoomLevel,
                         bool *didSomething);
    bool slotEndDragScroll ();

private slots:
    void slotBeganDocResize ();
    void slotContinuedDocResize (const QSize &size);
    void slotCancelledDocResize ();
    void slotEndedDocResize (const QSize &size);

    void slotDocResizeMessageChanged (const QString &string);

private slots:
    void slotActionPrevToolOptionGroup1 ();
    void slotActionNextToolOptionGroup1 ();
    void slotActionPrevToolOptionGroup2 ();
    void slotActionNextToolOptionGroup2 ();

    void slotActionDrawOpaqueToggled ();
    void slotActionDrawColorSimilarity ();

public slots:
    void slotToolRectSelection();
    void slotToolEllipticalSelection();
    void slotToolFreeFormSelection();
    void slotToolText();

//
// File Menu
//

private:
    void setupFileMenuActions ();
    void enableFileMenuDocumentActions (bool enable = true);

    void addRecentURL (const QUrl &url);

private slots:
    void slotNew ();

private:
    QSize defaultDocSize () const;
    void saveDefaultDocSize (const QSize &size);

private:
    bool shouldOpen ();
    void setDocumentChoosingWindow (kpDocument *doc);

private:
    kpDocument *openInternal (const QUrl &url,
        const QSize &fallbackDocSize,
        bool newDocSameNameIfNotExist);
    // Same as above except that it:
    //
    // 1. Assumes a default fallback document size.
    // 2. If the URL is successfully opened (with the special exception of
    //    the "kolourpaint doesnotexist.png" case), it is bubbled up to the
    //    top in the Recent Files Action.
    //
    // As a result of this behavior, this should only be called in response
    // to a user open request e.g. File / Open or "kolourpaint doesexist.png".
    // It should not be used for session restore - in that case, it does not
    // make sense to bubble the Recent Files list.
    bool open (const QUrl &url, bool newDocSameNameIfNotExist = false);

    QList<QUrl> askForOpenURLs(const QString &caption,
                              bool allowMultipleURLs = true);

private slots:
    void slotOpen ();
    void slotOpenRecent (const QUrl &url);
    void slotRecentListCleared();

#if HAVE_KSANE
    void slotScan ();
    void slotScanned (const QImage &image, int);
#endif // HAVE_KSANE

    void slotScreenshot();
    void slotMakeScreenshot();

    void slotProperties ();

    bool save (bool localOnly = false);
    bool slotSave ();

private:
    QUrl askForSaveURL (const QString &caption,
                        const QString &startURL,
                        const kpImage &imageToBeSaved,
                        const kpDocumentSaveOptions &startSaveOptions,
                        const kpDocumentMetaInfo &docMetaInfo,
                        const QString &forcedSaveOptionsGroup,
                        bool localOnly,
                        kpDocumentSaveOptions *chosenSaveOptions,
                        bool isSavingForFirstTime,
                        bool *allowLossyPrompt);

private slots:
    bool saveAs (bool localOnly = false);
    bool slotSaveAs ();

    bool slotExport ();

    void slotEnableReload ();
    bool slotReload ();
    void sendPreviewToPrinter(QPrinter *printer);

private:
    void sendDocumentNameToPrinter (QPrinter *printer);
    void sendImageToPrinter(QPrinter *printer, bool showPrinterSetupDialog);

private slots:
    void slotPrint ();
    void slotPrintPreview ();

    void slotMail ();

    bool queryCloseDocument ();
    bool queryClose () override;

    void slotClose ();
    void slotQuit ();


//
// Edit Menu
//

private:
    void setupEditMenuActions ();
    void enableEditMenuDocumentActions (bool enable = true);

public:
    QMenu *selectionToolRMBMenu ();

private slots:
    void slotCut ();
    void slotCopy ();
    void slotEnablePaste ();
private:
    QRect calcUsefulPasteRect (int imageWidth, int imageHeight);
    // (it is possible to paste a selection border i.e. a selection with no content)
    void paste (const kpAbstractSelection &sel,
                bool forceTopLeft = false);
public:
    // (<forceNewTextSelection> is ignored if <text> is empty)
    void pasteText (const QString &text,
                    bool forceNewTextSelection = false,
                    const QPoint &newTextSelectionTopLeft = KP_INVALID_POINT);
    void pasteTextAt (const QString &text, const QPoint &point,
                      // Allow tiny adjustment of <point> so that mouse
                      // pointer is not exactly on top of the topLeft of
                      // any new text selection (so that it doesn't look
                      // weird by being on top of a resize handle just after
                      // a paste).
                      bool allowNewTextSelectionPointShift = false);
public slots:
    void slotPaste ();
private slots:
    void slotPasteInNewWindow ();
public slots:
    void slotDelete ();

    void slotSelectAll ();
private:
    void addDeselectFirstCommand (kpCommand *cmd);
public slots:
    void slotDeselect ();
private slots:
    void slotCopyToFile ();
    void slotPasteFromFile ();


//
// View Menu
//

private:
    void setupViewMenuActions ();

    bool viewMenuDocumentActionsEnabled () const;
    void enableViewMenuDocumentActions (bool enable = true);
    void actionShowGridUpdate ();
    void updateMainViewGrid ();
    QRect mapToGlobal (const QRect &rect) const;
    QRect mapFromGlobal (const QRect &rect) const;

private slots:
    void slotShowGridToggled ();


//
// View Menu - Zoom
//

private:
    void setupViewMenuZoomActions ();
    void enableViewMenuZoomDocumentActions (bool enable);

    void sendZoomListToActionZoom ();

    void zoomToPre (int zoomLevel);
    void zoomToPost ();

public:
    void zoomTo (int zoomLevel, bool centerUnderCursor = false);
    void zoomToRect (const QRect &normalizedDocRect,
        bool accountForGrips,
        bool careAboutWidth, bool careAboutHeight);

public slots:
    void slotActualSize ();
    void slotFitToPage ();
    void slotFitToWidth ();
    void slotFitToHeight ();

public:
    void zoomIn (bool centerUnderCursor = false);
    void zoomOut (bool centerUnderCursor = false);

public slots:
    void slotZoomIn ();
    void slotZoomOut ();

private:
    void zoomAccordingToZoomAction (bool centerUnderCursor = false);

private slots:
    void slotZoom ();


//
// View Menu - Thumbnail
//

private:
    void setupViewMenuThumbnailActions ();
    void enableViewMenuThumbnailDocumentActions (bool enable);

private slots:
    void slotDestroyThumbnail ();
    void slotDestroyThumbnailInitatedByUser ();
    void slotCreateThumbnail ();

public:
    void notifyThumbnailGeometryChanged ();

private slots:
    void slotSaveThumbnailGeometry ();
    void slotShowThumbnailToggled ();
    void updateThumbnailZoomed ();
    void slotZoomedThumbnailToggled ();
    void slotThumbnailShowRectangleToggled ();

private:
    void enableViewZoomedThumbnail (bool enable = true);
    void enableViewShowThumbnailRectangle (bool enable = true);
    void enableThumbnailOptionActions (bool enable = true);
    void createThumbnailView ();
    void destroyThumbnailView ();
    void updateThumbnail ();


//
// Image Menu
//

private:
    kpTransformDialogEnvironment *transformDialogEnvironment ();

    bool isSelectionActive () const;
    bool isTextSelection () const;

    QString autoCropText () const;

    void setupImageMenuActions ();
    void enableImageMenuDocumentActions (bool enable = true);

private slots:
    void slotImageMenuUpdateDueToSelection ();

public:
    kpColor backgroundColor (bool ofSelection = false) const;
    void addImageOrSelectionCommand (kpCommand *cmd,
                                     bool addSelCreateCmdIfSelAvail = true,
                                     bool addSelContentCmdIfSelAvail = true);

public slots:
    void slotCrop ();

private slots:
    void slotResizeScale ();
    void slotAutoCrop ();
    void slotFlip ();
    void slotMirror ();

    void slotRotate ();
    void slotRotate270 ();
    void slotRotate90 ();

    void slotSkew ();
    void slotConvertToBlackAndWhite ();
    void slotConvertToGrayscale ();
    void slotInvertColors ();
    void slotClear ();
    void slotMakeConfidential();
    void slotMoreEffects ();


//
// Colors Menu
//

private:
    void setupColorsMenuActions ();
    void createColorBox ();
    void enableColorsMenuDocumentActions (bool enable);
private slots:
    void slotUpdateColorsDeleteRowActionEnabled ();

private:
    void deselectActionColorsKDE ();

    bool queryCloseColors ();

private:
    void openDefaultColors ();
private slots:
    void slotColorsDefault ();

private:
    bool openKDEColors (const QString &name);
private slots:
    void slotColorsKDE ();

private:
    bool openColors (const QUrl &url);
private slots:
    void slotColorsOpen ();

    void slotColorsReload ();

    bool slotColorsSave ();
    bool slotColorsSaveAs ();

    void slotColorsAppendRow ();
    void slotColorsDeleteRow ();


//
// Settings Menu
//

private:
    void setupSettingsMenuActions ();
    void enableSettingsMenuDocumentActions (bool enable = true);

private slots:
    void slotFullScreen ();

    void slotEnableSettingsShowPath ();
    void slotShowPathToggled ();
    void slotDrawAntiAliasedToggled(bool on);

    void slotKeyBindings ();

//
// Status Bar
//

private:
    enum
    {
        StatusBarItemShapePoints,
        StatusBarItemShapeSize,
        StatusBarItemDocSize,
        StatusBarItemDocDepth,
        StatusBarItemZoom
    };

    void addPermanentStatusBarItem (int id, int maxTextLen);
    void createStatusBar ();

    void setStatusBarDocDepth (int depth = 0);

private slots:
    void setStatusBarMessage (const QString &message = QString());
    void setStatusBarShapePoints (const QPoint &startPoint = KP_INVALID_POINT,
                                  const QPoint &endPoint = KP_INVALID_POINT);
    void setStatusBarShapeSize (const QSize &size = KP_INVALID_SIZE);
    void setStatusBarDocSize (const QSize &size = KP_INVALID_SIZE);
    void setStatusBarZoom (int zoom = 0);

    void recalculateStatusBarMessage ();
    void recalculateStatusBarShape ();

    void recalculateStatusBar ();


//
// Text ToolBar
//

private:
    void setupTextToolBarActions ();
    void readAndApplyTextSettings ();

public:
    void enableTextToolBarActions (bool enable = true);

private slots:
    void slotTextFontFamilyChanged ();
    void slotTextFontSizeChanged ();
    void slotTextBoldChanged ();
    void slotTextItalicChanged ();
    void slotTextUnderlineChanged ();
    void slotTextStrikeThruChanged ();

public:
    KToolBar *textToolBar ();
    bool isTextStyleBackgroundOpaque () const;
    kpTextStyle textStyle () const;
    void setTextStyle (const kpTextStyle &textStyle_);
    int settingTextStyle () const;

private:
    struct kpMainWindowPrivate *d;
};

#endif  // KP_MAIN_WINDOW_H
