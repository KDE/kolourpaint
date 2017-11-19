
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


#ifndef kpAbstractImageSelectionTool_H
#define kpAbstractImageSelectionTool_H


#include "tools/selection/kpAbstractSelectionTool.h"


class kpImageSelectionTransparency;


// The only difference between the various subclasses of us is the kind of
// selection that they create e.g. elliptical vs rectangular.
//
// For every other operation, they act identically so, for instance, it is
// possible to move an elliptical selection while using the rectangular
// selection tool (this situation can arise when you paste an elliptical
// selection while using the rectangular selection tool; a tool change
// does not occur out of convenience to the user - see
// kpDocumentEnvironment::switchToCompatibleTool()).
class kpAbstractImageSelectionTool : public kpAbstractSelectionTool
{
Q_OBJECT

public:
    kpAbstractImageSelectionTool (const QString &text, const QString &description,
        int key,
        kpToolSelectionEnvironment *environ, QObject *parent,
        const QString &name);


//
// Drawing
//

protected:
    kpAbstractSelectionContentCommand *newGiveContentCommand () const override;

    QString nameOfCreateCommand () const override;


//
// Create, Move, Resize/Scale
//

protected:
    QString haventBegunDrawUserMessageCreate () const override;
    QString haventBegunDrawUserMessageMove () const override;
    QString haventBegunDrawUserMessageResizeScale () const override;


//
// User Changing Selection Transparency
//

protected:
    bool shouldChangeImageSelectionTransparency () const;
    // You must derive <oldTrans>, the old selection transparency, from the
    // one obtained from the user's current settings, as given by the
    // kpToolSelectionEnvironment.
    //
    // You must _not_ simply get the old selection transparency just by
    // querying the selection i.e. do _not_ pass in
    // "document()->imageSelection().transparency()".  The reason is that
    // transparency().transparentColor() might not be defined in Opaque
    // Mode.
    void changeImageSelectionTransparency (
        const QString &name,
        const kpImageSelectionTransparency &newTrans,
        const kpImageSelectionTransparency &oldTrans);

protected slots:
    void slotIsOpaqueChanged (bool isOpaque) override;
    void slotBackgroundColorChanged (const kpColor &color) override;
    void slotColorSimilarityChanged (double similarity, int) override;
};


#endif  // kpAbstractImageSelectionTool_H
