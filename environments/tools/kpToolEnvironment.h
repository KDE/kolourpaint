
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


#ifndef kpToolEnvironment_H
#define kpToolEnvironment_H


#include "environments/kpEnvironmentBase.h"


class QActionGroup;
class QPoint;
class QRect;
class QString;

class KActionCollection;

class kpColor;
class kpCommandHistory;
class kpToolToolBar;


// Facade for kpTool clients.
class kpToolEnvironment : public kpEnvironmentBase
{
Q_OBJECT

public:
    // Note: Our interface must never publicly leak <mainWindow> or any other
    //       classes we are trying to hide as that would defeat the point of
    //       the facade.
    kpToolEnvironment (kpMainWindow *mainWindow);
    ~kpToolEnvironment () override;


    KActionCollection *actionCollection () const;

    kpCommandHistory *commandHistory () const;

    QActionGroup *toolsActionGroup () const;
    kpToolToolBar *toolToolBar () const;
    void hideAllToolWidgets () const;
    bool selectPreviousTool () const;

    kpColor color (int which) const;
    double colorSimilarity () const;
    int processedColorSimilarity () const;

    // (only valid in kpTool::slotForegroundColorChanged())
    kpColor oldForegroundColor () const;
    // (only valid in kpTool::slotBackgroundColorChanged())
    kpColor oldBackgroundColor () const;

    // (only valid in kpTool::slotColorSimilarityChanged())
    double oldColorSimilarity () const;

    // Flashes the Color Similarity Tool Bar Item to highlight to the user,
    // the existence of the Color Similarity feature.
    //
    // This should be used in only 3 circumstances:
    //
    // 1. Tools not acting on the selection but using Color Similarity
    //    e.g. Color Eraser, Flood Fill, Auto Crop.  Note that Auto Crop
    //    becomes Remove Internal Border when a selection is active but
    //    the flashing still occurs because the extent of the effect is
    //    directly dependent on Color Similarity.
    //
    // 2. A change to selection transparency (background color, color similarity
    //    percentage, change from opaque to transparent).
    //
    //    It is not used when the transparency is opaque or when changing from
    //    transparent to opaque, as you're not using Color Similarity in these
    //    cases.
    //
    //    Similarly, it is not used when there is no image selection or the
    //    image selection is just a border, as changing the selection
    //    transparency also does nothing in these cases.
    //
    // 3. Pulling a selection from the document, when selection transparency
    //    is transparent.
    //
    //    Except for any pulling phase, it is not used for effects that use
    //    the "transparent image" of a selection (e.g. smearing a transparent
    //    selection).  This is to minimize distractions and because the flashing
    //    has already been done by pulling.
    //
    // This should always be called _before_ an operation related to Color
    // Similarity, to indicate that the Color Similarity was applied throughout
    // the operation -- not at the end.  This aspect is most noticeable for
    // the time-consuming Autocrop feature, which may fail with a dialog --
    // it would look wrong to only flash the Color Similarity Tool Bar Item when
    // the dialog comes up after the failed operation, as it implies that Color
    // Similarity was not used during the operation.  Having said all this,
    // currently the flashing still happens afterwards because the flashing is
    // not done in a separate thread.  However, assume that the implementation
    // will be fixed later.
    //
    // It is tempting to simply disable the Color Similarity Tool Bar Item
    // if the current tool does not use Color Similarity - this change in
    // enabled state would be enough to highlight the existence of Color
    // Similarity, without the need for this flashing.  However, Auto Crop
    // is always available - independent of the current tool - and it uses
    // Color Similarity, so we can never disable the Tool Bar Item.
    //
    // We flash in tools but not commands as else it would be very distracting
    // Undo/Redo - this flashing in the tools is distracting enough already :)
    void flashColorSimilarityToolBarItem () const;

    void setColor (int which, const kpColor &color) const;

    void deleteSelection () const;
    void pasteTextAt (const QString &text, const QPoint &point,
                      // Allow tiny adjustment of <point> so that mouse
                      // pointer is not exactly on top of the topLeft of
                      // any new text selection (so that it doesn't look
                      // weird by being on top of a resize handle just after
                      // a paste).
                      bool allowNewTextSelectionPointShift = false) const;

    void zoomIn (bool centerUnderCursor = false) const;
    void zoomOut (bool centerUnderCursor = false) const;

    void zoomToRect (const QRect &normalizedDocRect,
        bool accountForGrips,
        bool careAboutWidth, bool careAboutHeight) const;

    void fitToPage () const;

    static bool drawAntiAliased;


private:
    struct kpToolEnvironmentPrivate * const d;
};


#endif  // kpToolEnvironment_H

