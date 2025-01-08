
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
    kpAbstractImageSelectionTool(const QString &text,
                                 const QString &description,
                                 int key,
                                 kpToolSelectionEnvironment *environ,
                                 QObject *parent,
                                 const QString &name);

    //
    // Drawing
    //

protected:
    kpAbstractSelectionContentCommand *newGiveContentCommand() const override;

    QString nameOfCreateCommand() const override;

    //
    // Create, Move, Resize/Scale
    //

protected:
    QString haventBegunDrawUserMessageCreate() const override;
    QString haventBegunDrawUserMessageMove() const override;
    QString haventBegunDrawUserMessageResizeScale() const override;

    //
    // User Changing Selection Transparency
    //

protected:
    bool shouldChangeImageSelectionTransparency() const;
    // You must derive <oldTrans>, the old selection transparency, from the
    // one obtained from the user's current settings, as given by the
    // kpToolSelectionEnvironment.
    //
    // You must _not_ simply get the old selection transparency just by
    // querying the selection i.e. do _not_ pass in
    // "document()->imageSelection().transparency()".  The reason is that
    // transparency().transparentColor() might not be defined in Opaque
    // Mode.
    void changeImageSelectionTransparency(const QString &name, const kpImageSelectionTransparency &newTrans, const kpImageSelectionTransparency &oldTrans);

protected Q_SLOTS:
    void slotIsOpaqueChanged(bool isOpaque) override;
    void slotBackgroundColorChanged(const kpColor &color) override;
    void slotColorSimilarityChanged(double similarity, int) override;
};

#endif // kpAbstractImageSelectionTool_H
